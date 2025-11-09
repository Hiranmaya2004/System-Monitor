// Day 4: sysmon.cpp
// Build: g++ -std=c++17 sysmon.cpp -o sysmon

#include <bits/stdc++.h>
#include <signal.h>
#include <errno.h>
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
        p.name = content.substr(content.find(' ')+1, rp - content.find(' ') - 1);
        string after = content.substr(rp+2);
        vector<string> toks; string t;
        stringstream s(after);
        while (s >> t) toks.push_back(t);
        if (toks.size() >= 13) { p.utime = stoul(toks[11]); p.stime = stoul(toks[12]); }
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

void print_header(const string &sortmode) {
    cout << "=== System Monitor (Day 4) - sorted by " << sortmode << " ===\n\n";
    cout << left << setw(8) << "PID" << setw(25) << "NAME" << setw(12) << "CPU_jiff" << setw(12) << "RSS(kB)" << "\n";
    cout << string(70,'-') << "\n";
}

int main(int argc, char** argv) {
    string sortmode = "cpu";
    for (int i=1;i<argc;i++) {
        string a = argv[i];
        if (a.rfind("--sort=",0) == 0) sortmode = a.substr(7);
    }

    vector<ProcInfo> procs;
    for (int pid : list_pids()) procs.push_back(read_proc(pid));

    if (sortmode == "mem")
        sort(procs.begin(), procs.end(), [](auto &a, auto &b){ return a.rss_kb > b.rss_kb; });
    else
        sort(procs.begin(), procs.end(), [](auto &a, auto &b){ return a.total_time() > b.total_time(); });

    print_header(sortmode);
    int shown=0;
    for (auto &p : procs) {
        if (p.pid==0) continue;
        cout << setw(8) << p.pid << setw(25) << p.name << setw(12) << p.total_time() << setw(12) << p.rss_kb << "\n";
        if (++shown>=40) break;
    }

    cout << "\nCommands:\n  k <PID>  -> send SIGTERM to PID\n  K <PID>  -> send SIGKILL to PID\n  q        -> quit\n";
    cout << "> ";
    string cmd;
    while (getline(cin, cmd)) {
        stringstream ss(cmd);
        string op; int pid;
        ss >> op;
        if (op == "q") break;
        if ((op == "k" || op == "K") && (ss >> pid)) {
            int sig = (op == "K") ? SIGKILL : SIGTERM;
            if (kill(pid, sig) == 0) {
                cout << "Signal " << sig << " sent to PID " << pid << "\n";
            } else {
                cout << "Failed to send signal to " << pid << " : " << strerror(errno) << "\n";
            }
        } else {
            cout << "Unknown command\n";
        }
        cout << "> ";
    }

    return 0;
}
