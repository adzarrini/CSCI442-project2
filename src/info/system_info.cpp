#include "system_info.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

using namespace std;


double get_uptime() {
  // Implemented
	ifstream uptime(PROC_ROOT "/uptime");
	if(!uptime) {
		cerr << "Error opening /proc/uptime" << endl;
		exit(EXIT_FAILURE);
	}
	double upTime, idle;
	uptime >> upTime >> idle;

  return upTime;
}


SystemInfo get_system_info() {
  // Implemented
	SystemInfo systemInfo;

	systemInfo.uptime = get_uptime();
	systemInfo.load_average = get_load_average();
	systemInfo.memory_info = get_memory_info();
	systemInfo.cpus = get_cpu_info();
	systemInfo.processes = get_all_processes(PROC_ROOT);
	systemInfo.num_processes = systemInfo.processes.size();

	systemInfo.num_threads = 0;
	systemInfo.num_user_threads = 0;
	systemInfo.num_kernel_threads = 0;
	systemInfo.num_running = 0;


	for(ProcessInfo p : systemInfo.processes) {
		if(p.is_kernel_thread()) systemInfo.num_kernel_threads++;
		if(p.state == 'R') systemInfo.num_running++;
		for(ProcessInfo t : p.threads) {
			if(t.is_user_thread()) systemInfo.num_user_threads++;
		}
	}
	systemInfo.num_threads = systemInfo.num_kernel_threads + systemInfo.num_user_threads;

  return systemInfo;
}
