#include <iostream>
#include <cstdint>
#include <iomanip>

#if defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <sys/resource.h>
    #include <limits>
#endif

std::size_t get_usable_stack_bytes() {
#if defined(_WIN32)
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&mbi, &mbi, sizeof(mbi));
    uintptr_t current = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
    uintptr_t base = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
    return current - base;
#elif defined(__unix__) || defined(__APPLE__)
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        if (rl.rlim_cur == RLIM_INFINITY) return 64 * 1024 * 1024; // 64MB fallback
        return rl.rlim_cur;
    }
    return 0;
#else
    return 0;
#endif
}

void try_stack_allocation(std::size_t total_stack) {
    const std::size_t safety_margin = 32 * 1024; // 32 KB safety buffer
    if (total_stack <= safety_margin) {
        std::cerr << "Not enough stack space to safely allocate.\n";
        return;
    }

    std::size_t alloc_bytes = total_stack - safety_margin;
    std::size_t num_ints = alloc_bytes / sizeof(int);

    std::cout << "Preparing to allocate a stack array:\n"
          << "  - Elements   : " << num_ints << " integers\n"
          << "  - Total size : " << (num_ints * sizeof(int)) / 1024 << " KB ("
          << static_cast<double>(num_ints * sizeof(int)) / (1024 * 1024) << " MB)\n";


#if defined(_WIN32)
    std::cout << "Skipping stack allocation on Windows for safety.\n";
#else
    try {
        volatile int stack_array[num_ints];
        stack_array[0] = 123;
        stack_array[num_ints - 1] = 321;
        std::cout << "Stack allocation successful.\n";
    } catch (...) {
        std::cerr << "Stack allocation failed or crashed.\n";
    }
#endif
}

int main() {
    std::size_t stack_bytes = get_usable_stack_bytes();
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "=== Stack Size Information ===\n";

#if defined(_WIN32)
    std::cout << "Platform          : Windows\n";
    std::cout << "Usable Stack Size : " << stack_bytes / 1024 << " KB\n";

#elif defined(__unix__) || defined(__APPLE__)
    struct rlimit rl;
    getrlimit(RLIMIT_STACK, &rl);

    std::cout << "Platform          : "
#if defined(__APPLE__)
              << "macOS\n";
#else
              << "Linux/Unix\n";
#endif

    std::cout << "Soft Stack Limit  : " << rl.rlim_cur / 1024 << " KB ("
              << static_cast<double>(rl.rlim_cur) / (1024 * 1024) << " MB)\n";

    if (rl.rlim_max == RLIM_INFINITY) {
        std::cout << "Hard Stack Limit  : unlimited\n";
    } else {
        std::cout << "Hard Stack Limit  : "
                  << static_cast<double>(rl.rlim_max) / (1024 * 1024) << " MB ("
                  << static_cast<double>(rl.rlim_max) / (1024 * 1024 * 1024) << " GB)\n";
    }

    std::cout << "Usable Stack Size : " << stack_bytes / 1024 << " KB ("
              << static_cast<double>(stack_bytes) / (1024 * 1024) << " MB)\n";
#endif

    std::cout << "\n=== Stack Allocation Test ===\n";
    try_stack_allocation(stack_bytes);

    return 0;
}
