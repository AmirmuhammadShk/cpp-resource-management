#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <new>
#include <type_traits>
#include <utility>
#include <cstdlib>
// A tiny fixed-size arena that:
// - uses an internal stack buffer (no malloc)
// - supports placement-new allocation
// - tracks destructors and runs them in reverse order at reset/destruction
template <std::size_t N, std::size_t MaxDtors = 128>
class FixedArena {
public:
    FixedArena() = default;

    FixedArena(const FixedArena&) = delete;
    FixedArena& operator=(const FixedArena&) = delete;

    ~FixedArena() {
        reset(); // RAII cleanup
    }

    // Allocate and construct an object of type T in the arena.
    template <class T, class... Args>
    T* make(Args&&... args) {
        static_assert(!std::is_array_v<T>, "Arrays not supported in this simple arena.");
        static_assert(alignof(T) <= alignof(std::max_align_t), "Unusual alignment not supported.");

        void* mem = allocate_aligned(sizeof(T), alignof(T));
        if (!mem) return nullptr;

        T* obj = ::new (mem) T(std::forward<Args>(args)...);
        push_dtor<T>(obj);
        return obj;
    }

    void reset() noexcept {
        // Destroy in reverse order of construction
        for (std::size_t i = dtor_count_; i > 0; --i) {
            dtors_[i - 1].fn(dtors_[i - 1].ptr);
        }
        dtor_count_ = 0;
        offset_ = 0;
    }

    std::size_t used() const noexcept { return offset_; }
    std::size_t capacity() const noexcept { return N; }

private:
    struct DtorEntry {
        void (*fn)(void*) noexcept;
        void* ptr;
    };

    template <class T>
    static void destroy(void* p) noexcept {
        static_cast<T*>(p)->~T();
    }

    template <class T>
    void push_dtor(T* obj) {
        if (dtor_count_ >= MaxDtors) {
            // In a real design, you might handle this better.
            // For demo purposes, we fail fast.
            std::puts("Too many objects for destructor list");
            ::abort();
        }
        dtors_[dtor_count_++] = DtorEntry{&destroy<T>, obj};
    }

    void* allocate_aligned(std::size_t size, std::size_t alignment) noexcept {
        std::size_t current = offset_;
        std::size_t aligned = align_up(current, alignment);
        if (aligned + size > N) return nullptr;
        offset_ = aligned + size;
        return buffer_ + aligned;
    }

    static std::size_t align_up(std::size_t x, std::size_t alignment) noexcept {
        std::size_t mask = alignment - 1;
        return (x + mask) & ~mask;
    }

    alignas(std::max_align_t) std::uint8_t buffer_[N]{};
    std::size_t offset_{0};

    DtorEntry dtors_[MaxDtors]{};
    std::size_t dtor_count_{0};
};

// Example resource type (not heap-allocating).
struct Widget {
    explicit Widget(int id) : id(id) {
        std::printf("Widget(%d) constructed\n", id);
    }
    ~Widget() {
        std::printf("Widget(%d) destroyed\n", id);
    }
    int id;
};

int main() {
    FixedArena<1024> arena;

    auto* a = arena.make<Widget>(1);
    auto* b = arena.make<Widget>(2);
    auto* c = arena.make<Widget>(3);

    if (!a || !b || !c) {
        std::puts("Allocation failed (arena out of space)");
        return 1;
    }

    std::printf("Arena used: %zu / %zu bytes\n", arena.used(), arena.capacity());

    // Arena cleanup happens automatically at end of scope (RAII),
    // or you can do it manually:
    std::puts("Resetting arena...");
    arena.reset();

    std::printf("Arena used after reset: %zu / %zu bytes\n", arena.used(), arena.capacity());
    return 0;
}
