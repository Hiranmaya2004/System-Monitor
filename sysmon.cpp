// Day 3: sysmon.cpp
// Build: g++ -std=c++17 sysmon.cpp -o sysmon

#include <bits/stdc++.h>
using namespace std;
namespace fs = filesystem;

struct ProcInfo {
    int pid;
    string name;
    unsigned long utime=0;
    unsigned long stime=0;
    long rss_kb=0;
    unsigned long total_time() const { return utime + stime; }
};

vector<int> list_pids() {
    vector<int> pids;
    for (const auto &entry : fs::directory_iterator("/proc")) {
        string name = entry.path().filename();
        if (!name.empty() && all_of(name.begin(), name.end(), ::isdigit))
            pids.push_back(stoi(name));
    }
    sort(pids.begin(), pids.end());
    return pids;
}

ProcInfo read_proc(int pid) {
    ProcInfo p; p.pid = pid;
    string statpath = "/proc/" + to_string(pid) + "/stat";
    ifstream sf(statpath);
    if (sf) {
        string content; getline(sf, content);
        size_t rp = content.rfind(')');
        string comm = content.substr(content.find(' ')+1, rp - content.find(' ') - 1);
        p.name = comm;
        string after = content.substr(rp+2);
        vector<string> toks; string t;
        stringstream s(after);
        while (s >> t) toks.push_back(t);
        if (toks.size() >= 13) { // need at least up to stime
            p.utime = stoul(toks[11]);
            p.stime = stoul(toks[12]);
        }
    }
    string statuspath = "/proc/" + to_string(pid) + "/status";
    ifstream pf(statuspath);
    string line;
    while (pf && getline(pf, line)) {
        if (line.rfind("VmRSS:",0) == 0) {
            stringstream ss(line); string key; long val; string unit;
            ss >> key >> val >> unit;
            p.rss_kb = val;
            break;
        }
    }
    return p;
}

int main(int argc, char** argv) {
    string sortmode = "cpu";
    for (int i=1;i<argc;i++) {
        string a = argv[i];
        if (a.rfind("--sort=",0) == 0) sortmode = a.substr(7);
    }

    vector<ProcInfo> procs;
    for (int pid : list_pids()) {
        procs.push_back(read_proc(pid));
    }

    if (sortmode == "mem") {
        sort(procs.begin(), procs.end(), [](auto &a, auto &b){
            return a.rss_kb > b.rss_kb;
        });
    } else { // cpu
        sort(procs.begin(), procs.end(), [](auto &a, auto &b){
            return a.total_time() > b.total_time();
        });
    }

    cout << "=== System Monitor (Day 3) - sorted by " << sortmode << " ===\n\n";
    cout << left << setw(8) << "PID" << setw(25) << "NAME" << setw(12) << "CPU_jiff" << setw(12) << "RSS(kB)" << "\n";
    cout << string(70,'-') << "\n";
    int shown = 0;
    for (auto &p : procs) {
        if (p.pid==0) continue;
        cout << setw(8) << p.pid << setw(25) << p.name << setw(12) << p.total_time() << setw(12) << p.rss_kb << "\n";
        if (++shown >= 40) break;
    }
    return 0;
}
