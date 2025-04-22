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

void print_stack_limit() {
#if defined(_WIN32)
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&mbi, &mbi, sizeof(mbi));
    uintptr_t current = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
    uintptr_t base = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
    size_t usableStack = current - base;

    std::cout << "Estimated usable stack size (Windows): "
              << usableStack / 1024 << " KB\n";

#elif defined(__unix__) || defined(__APPLE__)
    // POSIX: use getrlimit
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "Stack size limit (soft): "
                  << rl.rlim_cur / 1024 << " KB ("
                  << static_cast<double>(rl.rlim_cur) / (1024 * 1024) << " MB)\n";

        if (rl.rlim_max == RLIM_INFINITY) {
            std::cout << "Stack size limit (hard): unlimited\n";
        } else {
            std::cout << "Stack size limit (hard): "
                      << static_cast<double>(rl.rlim_max) / (1024 * 1024) << " MB ("
                      << static_cast<double>(rl.rlim_max) / (1024 * 1024 * 1024) << " GB)\n";
        }
    } else {
        std::cerr << "Failed to get stack size limit.\n";
    }
#else
    std::cout << "Unsupported platform.\n";
#endif
}

int main() {
    print_stack_limit();
    return 0;
}
