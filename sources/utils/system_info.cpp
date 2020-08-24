#include "system_info.h"
#ifdef _WIN32
#include <windows.h>

utils::MemoryUsage utils::GetMemoryUsage()
{
	MemoryUsage r;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	r.memory_in_use_percents = statex.dwMemoryLoad;
	r.physical_memory_total = statex.ullTotalPhys;
	r.physical_memory_free = statex.ullAvailPhys;
	r.paging_file_total = statex.ullTotalPageFile;
	r.paging_file_free = statex.ullAvailPageFile;
	r.virtual_memory_total = statex.ullTotalVirtual;
	r.virtual_memory_free = statex.ullAvailVirtual;
	r.extended_memory_free = statex.ullAvailExtendedVirtual;
	return r;
}
#else
#include "sys/types.h"
#include "sys/sysinfo.h"

utils::MemoryUsage utils::GetMemoryUsage()
{
	utils::MemoryUsage r = {0};
	struct sysinfo memInfo = {0};
	sysinfo(&memInfo);
	r.virtual_memory_total = (memInfo.totalram + memInfo.totalswap) * memInfo.mem_unit;
	r.virtual_memory_free = r.virtual_memory_total
			- (memInfo.totalram - memInfo.freeram + memInfo.totalswap - memInfo.freeswap) * memInfo.mem_unit;
	r.physical_memory_total = memInfo.totalram * memInfo.mem_unit;
	r.physical_memory_free = (memInfo.freeram + memInfo.bufferram) * memInfo.mem_unit;
	r.memory_in_use_percents = 100LL * (memInfo.totalram - (memInfo.freeram + memInfo.bufferram)) / memInfo.totalram;
	return r;
}
#endif
