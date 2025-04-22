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

size_t print_stack_limit() {
    size_t size;
#if defined(_WIN32)
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&mbi, &mbi, sizeof(mbi));
    uintptr_t current = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
    uintptr_t base = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
    size_t usableStack = current - base;

    std::cout << "Estimated usable stack size (Windows): "
              << usableStack / 1024 << " KB\n";
    size = usableStack;

#elif defined(__unix__) || defined(__APPLE__)
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "Stack size limit (soft): "
                  << rl.rlim_cur / 1024 << " KB ("
                  << static_cast<double>(rl.rlim_cur) / (1024 * 1024) << " MB)\n";
        size = rl.rlim_cur;

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
    return size;
}

int main() {
    size_t limit = print_stack_limit();
    size_t safe_size = limit - (64 * 1024);

    std::cout << "Trying to allocate " << safe_size / 1024 << " KB on stack...\n";

    int array[safe_size / sizeof(int)];
    std::cout << "Success.\n";

    return 0;
}
