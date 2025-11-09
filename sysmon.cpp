// Day 2: sysmon.cpp
// Build: g++ -std=c++17 sysmon.cpp -o sysmon

#include <bits/stdc++.h>
using namespace std;

struct ProcInfo {
    int pid;
    string name;
    unsigned long utime;
    unsigned long stime;
    long rss_kb;
    unsigned long total_time() const { return utime + stime; }
};

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t");
    if (a==string::npos) return "";
    size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b-a+1);
}

vector<int> list_pids() {
    vector<int> pids;
    for (const auto &entry : filesystem::directory_iterator("/proc")) {
        string name = entry.path().filename();
        bool isnum = !name.empty() && all_of(name.begin(), name.end(), ::isdigit);
        if (isnum) pids.push_back(stoi(name));
    }
    sort(pids.begin(), pids.end());
    return pids;
}

ProcInfo read_proc(int pid) {
    ProcInfo p{};
    p.pid = pid;
    // read /proc/[pid]/stat
    string statpath = "/proc/" + to_string(pid) + "/stat";
    ifstream sf(statpath);
    if (sf) {
        string content;
        getline(sf, content);
        // parsing: pid (comm) state ... utime stime ...
        // comm may have spaces inside parentheses, so find last ')'
        size_t rp = content.rfind(')');
        string after = content.substr(rp+2); // skip ") "
        string comm = content.substr(content.find(' ')+1, rp - content.find(' ') - 1);
        p.name = comm;
        stringstream ss(after);
        // skip fields until utime (which is field 14 and 15 relative to start of stat)
        // We'll read many tokens to get to utime/stime
        // fields 1.. (we already consumed comm and state), so we read tokens until 13 fields are skipped
        // A robust method: parse entire line by tokenizing
        vector<string> toks;
        string tok;
        string temp;
        // rebuild full token list including comm as one token
        // get tokens by splitting content with spaces, but keep comm as one entry
        // We already have 'after' which starts from field 3 onward
        stringstream s2(after);
        while (s2 >> tok) toks.push_back(tok);
        if (toks.size() >= 15) {
            // utime is toks[11], stime toks[12] (0-based)
            p.utime = stoul(toks[11]);
            p.stime = stoul(toks[12]);
        }
    }
    // read /proc/[pid]/statm or status for RSS
    string statuspath = "/proc/" + to_string(pid) + "/status";
    ifstream pf(statuspath);
    string line;
    while (pf && getline(pf, line)) {
        if (line.rfind("VmRSS:",0) == 0) {
            stringstream ss(line);
            string key; long val; string unit;
            ss >> key >> val >> unit;
            p.rss_kb = val; // in kB
            break;
        }
    }
    return p;
}

int main() {
    cout << "=== Simple System Monitor (Day 2) ===\n\n";
    cout << left << setw(8) << "PID" << setw(25) << "NAME" << setw(12) << "CPU_jiff" << setw(12) << "RSS(kB)" << "\n";
    cout << string(60,'-') << "\n";

    auto pids = list_pids();
    int shown = 0;
    for (int pid : pids) {
        if (shown++ >= 30) break; // limit output for readability
        ProcInfo pr = read_proc(pid);
        cout << setw(8) << pr.pid << setw(25) << pr.name << setw(12) << pr.total_time() << setw(12) << pr.rss_kb << "\n";
    }
    cout << "\n(End of Day 2 output)\n";
    return 0;
}
