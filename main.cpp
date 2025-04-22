#include <iostream>
#include <cstdint>
#include <thread>
#include <iomanip>
#include <csignal>
#include <csetjmp>

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#endif

size_t STEP_SIZE = 1024 * 4;
volatile size_t g_stack_used = 0;

#if defined(__unix__) || defined(__APPLE__)
stack_t altstack;
sigjmp_buf env;

void sigsegv_handler(int) {
    siglongjmp(env, 1);
}
#endif

size_t print_stack_limit() {
    size_t size = 0;
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
    } else {
        std::cerr << "Failed to get stack size limit.\n";
    }
#else
    std::cout << "Unsupported platform.\n";
#endif
    return size;
}

void probe_stack_usage_recursive(size_t depth = 0) {
    char buffer[STEP_SIZE];
    buffer[0] = static_cast<char>(depth);
    g_stack_used += STEP_SIZE;
    probe_stack_usage_recursive(depth + 1);
}

#if defined(_WIN32)
DWORD WINAPI thread_func(LPVOID) {
    __try {
        g_stack_used = 0;
        probe_stack_usage_recursive();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return 0;
}
#else
void* thread_func(void*) {
    altstack.ss_sp = malloc(SIGSTKSZ);
    altstack.ss_size = SIGSTKSZ;
    altstack.ss_flags = 0;
    sigaltstack(&altstack, nullptr);

    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sigaction(SIGSEGV, &sa, nullptr);

    g_stack_used = 0;

    if (sigsetjmp(env, 1) == 0) {
        probe_stack_usage_recursive();
    }

    free(altstack.ss_sp);
    return nullptr;
}
#endif

size_t start_probing_stack(size_t stack_limit) {
#if defined(_WIN32)
    DWORD thread_id;
    HANDLE hThread = CreateThread(nullptr, stack_limit, thread_func, nullptr, 0, &thread_id);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
#else
    const size_t min_stack = 64 * 1024;
    if (stack_limit < min_stack)
        stack_limit = min_stack;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_limit);

    pthread_t thread;
    pthread_create(&thread, &attr, thread_func, nullptr);
    pthread_join(thread, nullptr);

    pthread_attr_destroy(&attr);
#endif
    return g_stack_used;
}

int main() {
    size_t limit = print_stack_limit();
    size_t used = start_probing_stack(limit);
    std::cout << "Usable stack space before crash: "
              << used / 1024 << " KB (" << used / (1024.0 * 1024.0) << " MB)\n";
    return 0;
}
