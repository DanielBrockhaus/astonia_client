#include <windows.h>

unsigned long long get_total_system_memory(void) {
    MEMORYSTATUSEX ms;
    ms.dwLength=sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    return ms.ullTotalPhys;
}
