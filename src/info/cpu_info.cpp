#include "cpu_info.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;


vector<CpuInfo> get_cpu_info() {
  // TODO: implement me
	vector<CpuInfo> cpuInfo;

	ifstream stat(PROC_ROOT "/stat");
	if(!stat) {
		cerr << "Error opening /proc/stat" << endl;
		exit(EXIT_FAILURE);
	}
	string cpuFirst;
	while (stat >> cpuFirst) {
		CpuInfo cpu;
		if(!cpuFirst.find("cpu")) {
			stat >> cpu.user_time >> cpu.nice_time >> cpu.system_time >> cpu.idle_time
			>> cpu.io_wait_time >> cpu.irq_time >> cpu.softirq_time >> cpu.steal_time
			>> cpu.guest_time >> cpu.guest_nice_time;
			cpuInfo.push_back(cpu);
		}

	}

  return cpuInfo;
}


CpuInfo operator -(const CpuInfo& lhs, const CpuInfo& rhs) {
  // TODO: implement me
  return CpuInfo();
}
