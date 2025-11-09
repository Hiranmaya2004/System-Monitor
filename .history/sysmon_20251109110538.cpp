// Day 1: sysmon.cpp
// Build: g++ -std=c++17 sysmon.cpp -o sysmon

#include <bits/stdc++.h>
using namespace std;

string read_file_first_line(const string &path) {
    ifstream f(path);
    string s;
    if (f.is_open()) getline(f, s);
    return s;
}

long parse_mem_kb(const string &line) {
    // Expect: "MemTotal:   16389880 kB"
    stringstream ss(line);
    string key;
    long val = 0;
    ss >> key >> val;
    return val; // kB
}

int main() {
    cout << "=== Simple System Monitor (Day 1) ===\n\n";

    // Uptime
    string uptime_line = read_file_first_line("/proc/uptime");
    double uptime_seconds = 0.0;
    if(!uptime_line.empty()) {
        stringstream ss(uptime_line);
        ss >> uptime_seconds;
    }
    cout << "Uptime: " << (long)uptime_seconds << " seconds\n";

    // Load average (first field in /proc/loadavg)
    string loadavg_line = read_file_first_line("/proc/loadavg");
    string load1;
    if(!loadavg_line.empty()) {
        stringstream ss(loadavg_line);
        ss >> load1;
    }
    cout << "Load average (1m): " << load1 << "\n";

    // Memory
    string mem_total_line = read_file_first_line("/proc/meminfo");
    string mem_free_line;
    ifstream memf("/proc/meminfo");
    vector<string> memlines;
    string l;e
    while(getline(memf, l) && memlines.size() < 3) memlines.push_back(l);
    if(memlines.size() >= 1) mem_total_line = memlines[0];
    if(memlines.size() >= 2) mem_free_line = memlines[1];

    long mem_total_kb = parse_mem_kb(mem_total_line);
    long mem_free_kb = parse_mem_kb(mem_free_line);
    cout << "Memory Total: " << mem_total_kb / 1024 << " MB\n";
    cout << "Memory Free:  " << mem_free_kb / 1024 << " MB\n";

    cout << "\n(End of Day 1 output)\n";
    return 0;
}
