#include <cstdlib>
#include <cstring>
#include <iostream>

// Helper to encourage the allocator to reuse freed memory.
static void churn_allocator() {
    for (int i = 0; i < 2000; ++i) {
        char* p = (char*)std::malloc(64);
        if (!p) std::abort();
        std::memset(p, 0xAA, 64);
        std::free(p);
    }
}

int main() {
    std::cout << "Allocating 32 bytes...\n";
    char* p = (char*)std::malloc(32);
    if (!p) return 1;

    std::strcpy(p, "hello, world");
    std::cout << "p points to: " << p << "\n";

    std::cout << "Freeing p...\n";
    std::free(p);

    // Make it more likely p's chunk gets reused/overwritten.
    churn_allocator();

    std::cout << "Use-after-free: writing to freed memory...\n";
    // Intentional UAF write - undefined behavior; commonly crashes under ASAN or later.
    p[0] = 'X';

    // Often crashes before this line.
    std::cout << "If you see this, allocator didn't crash this time.\n";
    return 0;
}
