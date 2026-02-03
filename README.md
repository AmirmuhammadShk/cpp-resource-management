# Some C++ Resource Management Best Practice (Linux)

This repository explores **how C++ manages resources**, from doing it *right* with RAII, to *breaking things on purpose*, to managing memory **without malloc**.

Tested on Linux with `g++`.

---

## Requirements

- Linux
- `g++` (GCC 10+ recommended)
- `gdb` (for the debugging example)

Check:
```bash
g++ --version
gdb --version
```

---

## Build & Run

Each folder is independent.  
Compile from the project root or inside each folder.

---

## RAII Logger (no leaks)

**What it shows**

- RAII for files and locks  
- No raw `new` / `delete`
- Automatic cleanup even when exceptions happen

**Build & run**
```bash
cd raii-logger
g++ -std=c++20 -O2 -pthread main.cpp -o raii_logger
./raii_logger
```

**What happens**

- Multiple threads write to a log file
- An exception is thrown on purpose
- File and mutex are still cleaned up safely

This demonstrates **why RAII prevents leaks and deadlocks**.

---

## Use-after-free (intentional bug)

**What it shows**

- A classic C/C++ memory bug
- How use-after-free happens
- How to debug it with `gdb`

This program is **intentionally broken**.

**Build**
```bash
cd uaf-gdb
g++ -std=c++20 -g -O0 main.cpp -o uaf
```

**Run normally**
```bash
./uaf
```

It may crash or behave strangely that’s the point.

**Debug with gdb**
```bash
gdb ./uaf
```

Inside gdb:
```gdb
run
bt
```

You’ll see the crash caused by writing to memory **after it was freed**.

This project is about **learning to recognize and debug undefined behavior**.

---

## Fixed Arena (no malloc)

**What it shows**

- Manual memory management without `malloc`
- Fixed-size memory arena
- RAII cleanup of objects

**Build & run**
```bash
cd fixed-arena
g++ -std=c++20 -O2 main.cpp -o fixed_arena
./fixed_arena
```

**What happens**

- Objects are constructed inside a fixed buffer
- No heap allocation
- Destructors run automatically when the arena resets or goes out of scope

This is similar to patterns used in **game engines and embedded systems**.

---

## Folder structure

```
cpp-resource-management/
├── raii-logger/
├── uaf-gdb/
├── fixed-arena/
└── README.md
```

---

## Why this repo exists

- Show **correct C++ resource management**
- Show **what goes wrong when ownership is broken**
- Show **low-level control without dynamic allocation**


