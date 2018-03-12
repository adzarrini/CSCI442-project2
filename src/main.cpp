/**
 * Contains the main() routine of what will eventually be your version of top.
 */
#include <getopt.h>
#include <unistd.h>
#include <cstdlib>
#include <ncurses.h>
#include <time.h>
#include <iostream>
#include "info/system_info.h"
#include <ctime>
#include <string>
#include <chrono>
#include <map>
#include <algorithm>
using namespace std;

void get_time() {
  time_t result = std::time(nullptr);
  printw("top - %s", asctime(localtime(&result)));
}

void get_uptime(double uptime){
  time_t seconds(uptime); // you have to convert your input_seconds into time_t
  tm *p = gmtime(&seconds); // convert to broken down time
  printw("Up %02d days, %02d:%02d:%02d, ", p->tm_yday, p->tm_hour, p->tm_min, p->tm_sec);
}

void get_average(LoadAverageInfo load) {
  printw("load average: %.2f, %.2f, %.2f", load.one_min, load.five_mins, load.fifteen_mins);
}


// Could not get results implementing with delta, would get nan since newCpu would equal oldCpu
// resulting in a 0 division. This way we got number results
void get_cpu(vector<CpuInfo> newCpu, vector<CpuInfo> oldCpu) {
  CpuInfo delta = newCpu[0];//-oldCpu[0];

  double total_time = delta.total_time();
  double user_time = double(delta.user_time) / total_time * 100;
  double system_time = double(delta.system_time) / total_time * 100;
  double idle_time = double(delta.idle_time) / total_time * 100;

  printw("\nTotal CPU Usage- user mode: %.1f%, kernel mode: %.1f%, idle: %.1f%", user_time, system_time, idle_time);

  for(unsigned int i = 1; i < newCpu.size(); i++) {
    delta = newCpu[i];// - oldCpu[i];
    double total_time = delta.total_time();
    double user_time = double(delta.user_time) / total_time * 100;
    double system_time = double(delta.system_time) / total_time * 100;
    double idle_time = double(delta.idle_time) / total_time * 100;
    printw("\nCPU %d- user: %.1f%, kernel: %.1f%, idle: %.1f%", i - 1, user_time, system_time, idle_time);
  }

}

void get_processes(SystemInfo sys) {
  printw("\nProcesses: %d total, %d threads, %d running, %d user, %d kernel", sys.num_processes, sys.num_threads, sys.num_running, sys.num_user_threads, sys.num_kernel_threads);
}

void get_memory(MemoryInfo mem) {
  printw("\nMemory: %d total, %d free, %d used, %d buffers", mem.total_memory, mem.free_memory, mem.total_memory-mem.free_memory, mem.buffers_memory);
  printw("\nSwap: %d total, %d free, %d used, %d cached", mem.total_swap, mem.free_swap, mem.total_swap-mem.free_swap, mem.cached_memory);
  printw("\n");
}

void populate_map(map<int,double>& mapProc, vector<ProcessInfo> procs) {
  double time;
  for (unsigned int i = 0; i < procs.size(); i++) {
    time = double(procs[i].utime + procs[i].stime);
    mapProc.insert(pair <int, double> (procs[i].pid, time));
  }
}

tm* convert_ticks(ProcessInfo current) {
  double s = current.utime + current.stime / sysconf(_SC_CLK_TCK);
  time_t seconds(s);
  tm *p = gmtime(&seconds);
  return p;
}

void get_table(vector<ProcessInfo> newProc, vector<ProcessInfo> oldProc) {
  map<int,double> mapProc;
  double time;
  populate_map(mapProc, oldProc);

  printw("%-5s %-7s %-7s %-7s %-17s %-10s", "Pid", "State", "RSS", "Cpu%", "Command", "Time");
  for (unsigned int i = 0; i < 30; i++) {
    time = double(newProc[i].utime + newProc[i].stime);
    double cpuPercent;
    for (const auto& proc : mapProc) {
      if(proc.first == newProc[i].pid) {
        if(double(proc.second - time) <= 0.0) {
          cpuPercent = 0.0;
          break;
        }
        cpuPercent = double(time - proc.second)/(time + proc.second);
      }
    }
    string cmdline;
    tm* ticks;
    if (mapProc.count(newProc[i].pid > 0)) {
      cmdline = newProc[i].comm;
      cmdline.erase(0,1);
      cmdline.erase(cmdline.end()-1);

      ticks = convert_ticks(newProc[i]);      
    }
    printw("\n%-5d %-7c %-7ld %-7.2f %-17s %02d:%02d:%02d", newProc[i].pid, newProc[i].state, (newProc[i].resident * 4), 
      cpuPercent, cmdline.c_str(), ticks->tm_hour, ticks->tm_min, ticks->tm_sec); 

    refresh();
  }
}


typedef bool (*SortFn)(const ProcessInfo&, const ProcessInfo&);

/**
 * For flag implementation
 */
bool pid_compare(const ProcessInfo& proc1, const ProcessInfo& proc2) {
  return proc1.pid < proc2.pid;
}

bool time_compare(const ProcessInfo& proc1, const ProcessInfo& proc2) {
  return (proc1.utime + proc1.stime) > (proc2.utime + proc2.stime);
}

bool memory_compare(const ProcessInfo& proc1, const ProcessInfo& proc2) {
  return proc1.resident > proc2.resident;
}

bool cpu_usage_compare(const ProcessInfo& proc1, const ProcessInfo& proc2) {
   return proc1.cpu_percent > proc2.cpu_percent;
}
/*************************************************/
// This function was created with the aid of other students. Does the sorting for flags and dependencies
SortFn get_sort(int argc, char** argv, int& delay, int& hflag) {

  // An array of long-form options.
  static struct option long_options[] = {

    // long-form options.
    {"delay",  required_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {"sort-key",  required_argument, NULL, 's'},
    // Terminate the long_options array with an object containing all zeroes.
    {0, 0, 0, 0}
  };

  
    SortFn sort_fn = &cpu_usage_compare;
    // getopt_long parses one argument at a time. Loop until it tells us that it's
    // all done (returns -1).
    while (true) {

      // getopt_long stores the latest option index here,you can get the flag's
      // long-form name by using something like long_options[option_index].name
      int option_index = 0;

      // Process the next command-line flag. the return value here is the
      // character or integer specified by the short / long options.
      int flag_char = getopt_long(
          argc,           // The total number of arguments passed to the binary
          argv,           // The arguments passed to the binary
          "d:s:h",     // Short-form flag options
          long_options,   // Long-form flag options
          &option_index); // The index of the latest long-form flag

      // Detect the end of the options.
      if (flag_char == -1) {
        break;
      }

      switch (flag_char) {

        case 'd':
          //cout << "option -d with value " << optarg << endl;
          delay = atoi(optarg) * 1000;
        break;

        case 's':
          {
          string str(optarg);
            if (str == "PID") {
              sort_fn = &pid_compare;
            }       
            else if (str == "CPU") {
              sort_fn = &cpu_usage_compare;
            }       
            else if (str == "MEM") {
              sort_fn = &memory_compare;
            }       
            else if (str == "TIME") {
              sort_fn = &time_compare;          
            }     
          }
          break;
        case 'h':
          hflag = 0;
          break;

        case '?':
          // This represents an error case, but getopt_long already printed an error
          // message for us.
          abort();
          break;

        default:
          // This would only happen if a flag hasn't been handled, or if you're not
          // detecting -1 (no more flags) correctly.
          exit(EXIT_FAILURE);
      }

    }
  return sort_fn;
}

/**
 * Gets a character from the user, waiting for however many milliseconds that
 * were passed to timeout() below. If the letter entered is q, this method will
 * exit the program.
 */
void exit_if_user_presses_q() {
  char c = getch();

  if (c == 'q') {
    endwin();
    exit(EXIT_SUCCESS);
  }
}

void get_help() {
  while(true) {
    initscr();

    wclear(stdscr);

    printw("Help Menu:\n");
    printw("-d --delay DELAY\n");
    printw("Delay between updates, in tenths of seconds\n");
    printw("-s --sort-key COLUMN\n");
    printw("Sort by this column; one of: PID, CPU, MEM, or TIME\n");
    printw("-h --help\n");
    printw("Display a help message about these flags and exit\n");
    
    refresh();
    
    exit_if_user_presses_q();
    }
}

/**
 * Entry point for the program.
 */
int main(int argc, char** argv) {
  SortFn sort_fn;
  int delay = 1000;
  int hflag = -1;
  if (argc > 1) {
  sort_fn = get_sort(argc, argv, delay, hflag);
  }
  else {
  sort_fn = &cpu_usage_compare;
  }
  if (hflag != -1) {
    get_help();
    return EXIT_SUCCESS;
  }
  // ncurses initialization
  initscr();
  // Don't show a cursor.
  curs_set(FALSE);
    

  // Set getch to return after 1000 milliseconds; this allows the program to
  // immediately respond to user input while not blocking indefinitely.
  timeout(delay);

  SystemInfo oldSys = get_system_info(), newSys;
  vector<CpuInfo> oldCpu = oldSys.cpus, newCpu;
  vector<ProcessInfo> oldProc = oldSys.processes, newProc;

  while (true) {
    
    newSys = get_system_info();
    newCpu = newSys.cpus;
    newProc = newSys.processes;
    wclear(stdscr);

    //display
    get_time();
    get_uptime(newSys.uptime);
    get_average(newSys.load_average);
    get_cpu(newCpu, oldCpu);
    get_processes(newSys);
    get_memory(newSys.memory_info);
    std::sort(newProc.begin(), newProc.end(), sort_fn);
    get_table(newSys.processes, oldSys.processes);

    // Redraw the screen.
    refresh();

    oldSys = newSys;
    // End the loop and exit if Q is pressed
    exit_if_user_presses_q();
  }

  // ncurses teardown
  endwin();

  return EXIT_SUCCESS;
}
