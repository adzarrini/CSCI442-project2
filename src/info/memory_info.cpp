#include "memory_info.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


MemoryInfo get_memory_info() {
  // TODO: implement me

	MemoryInfo memInfo;

	ifstream meminfo (PROC_ROOT "/meminfo");
	if(!meminfo) {
		cerr << "Error opening /proc/meminfo" << endl;
		exit(EXIT_FAILURE);
	}
	string memType;
	while(meminfo >> memType) {
		string unit;

		if(memType == "MemTotal:") meminfo >> memInfo.total_memory;
		if(memType == "MemFree:") meminfo >> memInfo.free_memory;
		if(memType == "Buffers:") meminfo >> memInfo.buffers_memory;
		if(memType == "Cached:") meminfo >> memInfo.cached_memory;
		if(memType == "SwapTotal:") meminfo >> memInfo.total_swap;
		if(memType == "SwapFree:") meminfo >> memInfo.free_swap;

		meminfo >> unit;

	}

	meminfo.close();

  return memInfo;
}
