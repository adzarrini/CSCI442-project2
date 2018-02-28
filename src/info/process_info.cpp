#include "process_info.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include<regex>

using namespace std;


ProcessInfo get_process(int pid, const char* basedir) {
  // TODO: implement me
	ProcessInfo process;

	// Get info from cmdline
	string add(basedir);
	ifstream cmdline(add+"/"+to_string(pid)+"/cmdline");
	if (!cmdline) {
		cerr << "Error opening /proc/" << pid << "/cmdline" << endl;
		exit(EXIT_FAILURE);
	} 

	// Get info from cmdline
	ifstream comm(add+"/"+to_string(pid)+"/comm");
	if (!comm) {
		cerr << "Error opening /proc/" << pid << "/cmdline" << endl;
		exit(EXIT_FAILURE);
	} 

	string cmdtemp;
	cmdline >> cmdtemp;
	if(cmdtemp.empty()) comm >> process.command_line;
	else {
		regex_replace(back_inserter(process.command_line), cmdtemp.begin(), cmdtemp.end(), regex("\\0+"), " ");
		process.command_line.pop_back();
	}

	cmdline.close();
	comm.close();
	// Get info from status
	ifstream status(add+"/"+to_string(pid)+"/status");
	if (!status) {
		cerr << "Error opening /proc/" << pid << "/status" << endl;
		exit(EXIT_FAILURE);
	} 

	string statusType;
	while(status >> statusType) {
		if(!statusType.find("Tgid")) {
			status >> process.tgid;
			break;
		}
	}

	// Get info from statm
	ifstream statm(add+"/"+to_string(pid)+"/statm");
	if (!statm) {
		cerr << "Error opening /proc/" << pid << "/statm" << endl;
		exit(EXIT_FAILURE);
	} 
	
	statm >> process.size >> process.resident >> process.share >> process.trs 
	>> process.lrs >> process.drs >> process.dt;

	statm.close();
	
	// Get info from stat
	ifstream stat(add+"/"+to_string(pid)+"/stat");
	if (!stat) {
		cerr << "Error opening /proc/" << pid << "/stat" << endl;
		exit(EXIT_FAILURE);
	} 
	
	stat >> process.pid >> process.comm >> process.state >> process.ppid 
	>> process.pgrp >> process.session >> process.tty_nr >> process.tpgid 
	>> process.flags >> process.minflt >> process.cminflt >> process.majflt
	>> process.cmajflt >> process.utime >> process.stime >> process.cutime
	>> process.cstime >> process.priority >> process.nice >> process.num_threads
	>> process.itrealvalue >> process.starttime >> process.vsize >> process.rss
	>> process.rsslim >> process.startcode >> process.endcode >> process.startstack
	>> process.kstkesp >> process.kstkeip >> process.signal >> process.blocked
	>> process.sigignore >> process.sigcatch >> process.wchan >> process.nswap
	>> process.cnswap >> process.exit_signal >> process.processor 
	>> process.rt_priority >> process.policy >> process.delayacct_blkio_ticks 
	>> process.guest_time >> process.cguest_time;

	stat.close();

	process.threads = get_all_processes((add+"/"+to_string(pid)+"/task").c_str());
	for(size_t i = 0; i < process.threads.size(); i++) {
		process.threads[i].tgid = pid;
	}

  return process;
}


vector<ProcessInfo> get_all_processes(const char* basedir) {
  // Implemented
	vector<ProcessInfo> processes;

	DIR* dir = opendir(basedir);
	dirent* entry;

	if(!dir) return processes;
	

	while((entry = readdir(dir)) != NULL) {
		int pid = atol(entry->d_name);
		if(pid <= 0) continue;

		processes.push_back(get_process(pid,basedir));
	}

	closedir(dir);

  return processes;
}
