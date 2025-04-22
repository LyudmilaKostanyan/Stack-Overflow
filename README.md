# Stack Allocation and Overflow Detection

## Overview

This project explores platform-dependent stack size limits and demonstrates what happens when attempting to allocate large arrays on the stack. It provides cross-platform insight into stack memory behavior on Linux, macOS, and Windows.

## Problem Description

The stack is a limited region of memory with platform-defined limits. Allocating large local arrays risks stack overflow, which may crash the program. This project detects usable stack size at runtime and tries to allocate an array just below the safe limit, allowing us to observe and compare stack behavior across different systems.

This is useful for:

- Understanding stack memory layout and system constraints.
- Testing platform-dependent behaviors without crashing the system.
- Learning about guard pages, signal handling reserves, and runtime stack metadata.

## Code Explanation

The program consists of three main parts:

### 1. `get_usable_stack_bytes()`

- Retrieves the currently available stack size at runtime.
- On POSIX systems, it uses `getrlimit(RLIMIT_STACK, ...)` to get the soft limit.
- On Windows, it uses `VirtualQuery()` to find the current stack pointer and the base of the reserved region.

### 2. `try_stack_allocation(std::size_t total_stack)`

- Computes how much memory can be safely allocated on the stack.
- Subtracts a constant `safety_margin` (32 KB) from the usable stack size to avoid hitting guard pages or reserved system space.
- Allocates a local array (`int stack_array[]`) of size just under the safe threshold.
- Writes to the first and last elements to verify the array is actually allocated and used.
- On Windows, allocation is skipped to avoid overflow, since the default stack is very limited unless increased via linker settings.

### 3. `main()`

- Calls `get_usable_stack_bytes()` to determine usable stack space.
- Outputs stack limit information in a readable format.
- Calls `try_stack_allocation()` to attempt allocation and verify success.

All platform differences are handled with preprocessor macros.

## Example Output

### Ubuntu

```
=== Stack Size Information ===
Platform          : Linux/Unix
Soft Stack Limit  : 16384 KB (16.00 MB)
Hard Stack Limit  : unlimited
Usable Stack Size : 16384 KB (16.00 MB)

=== Stack Allocation Test ===
Preparing to allocate a stack array:
  - Elements   : 4186112 integers
  - Total size : 16352 KB (15.97 MB)
Stack allocation successful.
```

Note: On Linux, the reported "Usable Stack Size" is often exactly equal to the soft limit. That's because most Linux distributions reserve very little space within the stack for metadata—guard pages and signal handling areas are often placed outside the visible stack allocation. Thus, the entire soft limit is usually available for use by the application.

### macOS

```
=== Stack Size Information ===
Platform          : macOS
Soft Stack Limit  : 8176 KB (7.98 MB)
Hard Stack Limit  : 63.98 MB (0.06 GB)
Usable Stack Size : 8176 KB (7.98 MB)

=== Stack Allocation Test ===
Preparing to allocate a stack array:
  - Elements   : 2084864 integers
  - Total size : 8144 KB (7.95 MB)
Stack allocation successful.
```

### Windows

```
=== Stack Size Information ===
Platform          : Windows
Usable Stack Size : 1020 KB

=== Stack Allocation Test ===
Preparing to allocate a stack array:
  - Elements   : 252928 integers
  - Total size : 988 KB (0.96 MB)
Skipping stack allocation on Windows for safety.
```

#### Why aren't stack sizes exact on macOS and Windows?

These platforms reserve a portion of the stack for:

- Guard pages to detect overflows
- Thread-local storage and return address buffers
- Signal/exception handling context

For example:

- macOS reports `8176 KB` instead of `8192 KB`
- Windows reports `1020 KB` instead of `1024 KB`

These few kilobytes are not usable by the application, so the system subtracts them from the reported value.

Linux, on the other hand, usually reports the full soft limit as usable because system-reserved metadata is often handled differently (e.g., via separate memory areas or in the kernel).

## How to Compile and Run

### 1. Clone the Repository

```bash
git clone https://github.com/LyudmilaKostanyan/Stack-Overflow.git
cd Stack-Overflow
```

### 2. Build the Project

Use CMake to build the project:

```bash
cmake -S . -B build
cmake --build build
```

Ensure you have CMake and a C++ compiler (e.g., g++, clang++, or MSVC) installed.

### 3. Run the Program

#### For Windows Users

```bash
./build/Release/main.exe
```

#### For Linux/macOS Users

```bash
./build/main
```
