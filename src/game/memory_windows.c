#include <windows.h>
#include <psapi.h>

unsigned long long get_total_system_memory(void) {
    MEMORYSTATUSEX ms;
    ms.dwLength=sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    return ms.ullTotalPhys;
}

size_t get_memory_usage(void) {
    PROCESS_MEMORY_COUNTERS mi;
    GetProcessMemoryInfo(GetCurrentProcess(),&mi,sizeof(mi));
    return mi.WorkingSetSize;
}
