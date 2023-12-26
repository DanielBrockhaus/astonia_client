#include <sys/resource.h>
#include <unistd.h>

unsigned long long get_total_system_memory(void) {
    return sysconf(_SC_PHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
}

size_t get_memory_usage(void) {
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
    return r_usage.ru_maxrss*1024;
}
