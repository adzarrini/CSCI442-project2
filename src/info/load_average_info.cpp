#include "load_average_info.h"
#include <fstream>
#include <iostream>

using namespace std;


LoadAverageInfo get_load_average() {
  // Implemented
	LoadAverageInfo aveInfo;

	ifstream loadavg (PROC_ROOT "/loadavg");
	if(!loadavg) {
		cerr << "Error opening /proc/loadavg" << endl;
		exit(EXIT_FAILURE);
	}
	
	loadavg >> aveInfo.one_min >> aveInfo.five_mins >> aveInfo.fifteen_mins;

	loadavg.close();

  return aveInfo;
}
