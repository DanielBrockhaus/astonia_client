#include <unistd.h>

unsigned long long get_total_system_memory(void) {
    return sysconf(_SC_PHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
}
