#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"


using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
        static string getCmd(string pid);
        static vector<string> getPidList();
        static std::string getVmSize(string pid);
        static std::string getCpuPercent(string pid);
        static long int getSysUpTime();
        static std::string getProcUpTime(string pid);
        static string getProcUser(string pid);
        static vector<string> getSysCpuPercent(string coreNumber = "");
	static float getSysActiveCpuTime(vector<string> values);
	static float getSysIdleCpuTime(vector<string> values);
        static float getSysRamPercent();
        static string getSysKernelVersion();
        static int getNumberOfCores();
        static int getTotalThreads();
        static int getTotalNumberOfProcesses();
        static int getNumberOfRunningProcesses();
        static string getOSName();
        static string PrintCpuStats(vector<string> values1, vector<string> values2);
        static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:
string ProcessParser::getVmSize(string pid){
    string line;
    string name = "VmData";
    float result;

    //Retrieve data from file
    ifstream fin;
    Util::getStream(Path::basePath() + pid + Path::statusPath(), fin);

    while (getline(fin, line)){
        if (line.compare(0, name.size(), name) == 0) {
            //parse line
            istringstream buf(line);
            istream_iterator<string> beg(buf), end; //splits on white space
            vector<string> values(beg, end);
            //convert kB to GB
            result = (stof(values[1])/(float(1024*1024)));
            break;
        }
    }
    return to_string(result);
}

string ProcessParser::getCpuPercent(string pid) {
    string line;
    float result;

    //Retrieve data from file
    ifstream fin; 
    Util::getStream(Path::basePath() + pid + Path::statPath(), fin);
    getline(fin, line);
    //Parse data
    istringstream buf(line);
    istream_iterator<string> beg(buf), end; //split on whitespace
    vector<string> values(beg, end);
    //Field descriptions can be found at http://man7.org/linux/man-pages/man5/proc.5.html
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    result = 100.0*((total_time/freq)/seconds);
    return to_string(result);
}

string ProcessParser::getProcUpTime(string pid) {
    string line;
    string value;
    float result;

    //Retrieve data
    ifstream fin;
    Util::getStream(Path::basePath() + pid + Path::statPath(), fin);
    getline(fin, line);
    //parse data
    istringstream buf(line);
    istream_iterator<string> beg(buf), end; //split on whitespace
    vector<string> values(beg, end);
    result = float(stof(values[13])/sysconf(_SC_CLK_TCK));
    return to_string(result);
}

long int ProcessParser::getSysUpTime() {
    long double value;
    ifstream fin;
    Util::getStream(Path::basePath() + Path::upTimePath(), fin);
    fin >> value;
    return (long int)value;
}

string ProcessParser::getProcUser(string pid) {
  string line;
  string name = "Uid:";
  string uid;
  string username;

  ifstream fin;
  Util::getStream(Path::basePath() + pid + Path::statusPath(), fin);
  while (getline(fin, line)) {
    if(line.compare(0, name.size(), name) == 0){
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      uid = values[1];
      break;
    }
  }
  fin.close();
  Util::getStream("/etc/passwd", fin);
  while (getline(fin, line)) {
    if(line.find("x:" + uid) != std::string::npos) {
      username = line.substr(0, line.find(":"));
      return username;
    }
  }
  return "";
}

vector<string> ProcessParser::getPidList() {
  DIR* dir;
  vector<string> container;
  
  if (!(dir = opendir("/proc")))
    throw std::runtime_error(std::strerror(errno));

  while (dirent* dirp = readdir(dir)) {
    if (dirp->d_type != DT_DIR)//skip
      continue;
    if (all_of(dirp->d_name,dirp->d_name + std::strlen(dirp->d_name), [](char c){ return isdigit(c); }))
      container.push_back(dirp->d_name);
  }

  closedir(dir);
  return container;
}

string ProcessParser::getCmd(string pid) {
  string line;
  ifstream fin;
  Util::getStream(Path::basePath() + pid + Path::cmdPath(), fin);
  getline(fin, line);
  return line;
}

int ProcessParser::getNumberOfCores() {
  string line;
  string name = "cpu cores";
  string dummy;
  int cores;

  ifstream fin;
  Util::getStream(Path::basePath() + "cpuinfo", fin);
  while( getline(fin, line) ) {
    if(line.compare(0, name.size(), name) == 0) {
      istringstream buf(line);
      buf >> dummy >> dummy >> cores;
      return cores;
    }
  }
  return 0;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
  string line;
  string name = "cpu" + coreNumber; //does core number (as retrieved previously) make sense here for multi processor systems? /proc/stat seems to indicate cpu# corresponds to processor and core number is within the processor

  ifstream fin;
  Util::getStream(Path::basePath() + "stat", fin);
  while( getline(fin, line) ) {
    if(line.compare(0, name.size(), name) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      return values;
    }
  }
  return vector<string>();
}

float ProcessParser::getSysActiveCpuTime(vector<string> values) {
  return (stof(values[S_USER]) +
	  stof(values[S_NICE]) +
	  stof(values[S_SYSTEM]) +
	  stof(values[S_IRQ]) +
	  stof(values[S_SOFTIRQ]) +
	  stof(values[S_STEAL]) +
	  stof(values[S_GUEST]) +
	  stof(values[S_GUEST_NICE]));
}

float ProcessParser::getSysIdleCpuTime(vector<string> values) {
  return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2) {
  float cpuTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
  float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
  float totalTime = cpuTime + idleTime;
  float result = cpuTime/totalTime * 100.0;
  return to_string(result);
}
