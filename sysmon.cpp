// Day 5: sysmon.cpp
// Build: g++ -std=c++17 sysmon.cpp -o sysmon -lncurses
// Run: ./sysmon [interval_seconds]

#include <bits/stdc++.h>
#include <ncurses.h>
#include <signal.h>
using namespace std;
namespace fs = filesystem;

struct ProcSnapshot {
    int pid;
    string name;
    unsigned long total_time = 0; // utime+stime (jiffies)
    long rss_kb = 0;
    double cpu_percent = 0.0;
    double mem_mb = 0.0;
};

unsigned long get_total_jiffies() {
    ifstream f("/proc/stat");
    string line;
    getline(f, line);
    // line: cpu  3357 0 4313 1362393 ...
    stringstream ss(line);
    string cpu; unsigned long v, sum=0;
    ss >> cpu;
    while (ss >> v) sum += v;
    return sum;
}

vector<int> list_pids() {
    vector<int> pids;
    for (auto &e : fs::directory_iterator("/proc")) {
        string name = e.path().filename();
        if (!name.empty() && all_of(name.begin(), name.end(), ::isdigit))
            pids.push_back(stoi(name));
    }
    return pids;
}

bool read_proc_times(int pid, unsigned long &totaltime, long &rss_kb, string &name) {
    string statp = "/proc/" + to_string(pid) + "/stat";
    ifstream sf(statp);
    if (!sf) return false;
    string content; getline(sf, content);
    size_t rp = content.rfind(')');
    if (rp == string::npos) return false;
    name = content.substr(content.find(' ')+1, rp - content.find(' ') - 1);
    string after = content.substr(rp+2);
    vector<string> toks; string t; stringstream s(after);
    while (s >> t) toks.push_back(t);
    if (toks.size() < 13) return false;
    unsigned long utime = stoul(toks[11]);
    unsigned long stime = stoul(toks[12]);
    totaltime = utime + stime;

    // rss from /proc/[pid]/status
    rss_kb = 0;
    ifstream pf("/proc/" + to_string(pid) + "/status");
    string line;
    while (pf && getline(pf,line)) {
        if (line.rfind("VmRSS:",0) == 0) {
            string key; long val; string unit;
            stringstream ss(line); ss >> key >> val >> unit;
            rss_kb = val;
            break;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    int interval = 2;
    if (argc >= 2) interval = stoi(argv[1]);
    bool sort_by_cpu = true;

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE); // non-blocking getch
    keypad(stdscr, TRUE);

    // We'll keep previous snapshots in maps
    unordered_map<int, unsigned long> prev_proc_time;
    unsigned long prev_total_jiffies = get_total_jiffies();

    while (true) {
        unsigned long cur_total_jiffies = get_total_jiffies();

        vector<ProcSnapshot> procs;

        for (int pid : list_pids()) {
            unsigned long ttime;
            long rss;
            string name;
            if (!read_proc_times(pid, ttime, rss, name)) continue;
            ProcSnapshot ps;
            ps.pid = pid; ps.name = name; ps.total_time = ttime; ps.rss_kb = rss; ps.mem_mb = rss/1024.0;
            auto it = prev_proc_time.find(pid);
            unsigned long proc_prev = (it!=prev_proc_time.end()) ? it->second : 0;
            unsigned long proc_delta = (ttime >= proc_prev) ? (ttime - proc_prev) : 0;
            unsigned long total_delta = (cur_total_jiffies >= prev_total_jiffies) ? (cur_total_jiffies - prev_total_jiffies) : 1;
            ps.cpu_percent = 100.0 * (double)proc_delta / (double)total_delta;
            procs.push_back(ps);
            // update temp map later
        }

        // update previous maps
        prev_proc_time.clear();
        for (auto &p : procs) prev_proc_time[p.pid] = p.total_time;
        prev_total_jiffies = cur_total_jiffies;

        if (sort_by_cpu) {
            sort(procs.begin(), procs.end(), [](auto &a, auto &b){ return a.cpu_percent > b.cpu_percent; });
        } else {
            sort(procs.begin(), procs.end(), [](auto &a, auto &b){ return a.mem_mb > b.mem_mb; });
        }

        // Render
        erase();
        mvprintw(0,0,"SysMon (Day 5) - refresh every %d s - sort by %s - press 's' to toggle sort, 'k' to kill PID, 'q' to quit", interval, sort_by_cpu ? "CPU" : "MEM");
        mvprintw(1,0,"%-6s %-20s %-8s %-8s", "PID", "NAME", "CPU%", "MEM(MB)");
        int row = 2;
        int show = 0;
        for (auto &p : procs) {
            if (show++ >= 25) break;
            mvprintw(row++, 0, "%-6d %-20s %7.2f %-8.1f", p.pid, p.name.c_str(), p.cpu_percent, p.mem_mb);
        }
        refresh();

        // handle keys
        int ch;
        bool killedSomething = false;
        for (int t=0; t<interval*10; ++t) { // poll every 100ms to be responsive to keypress
            ch = getch();
            if (ch == 'q') {
                endwin();
                return 0;
            } else if (ch == 's') {
                sort_by_cpu = !sort_by_cpu;
                break; // break to refresh immediately
            } else if (ch == 'k') {
                // prompt user for PID
                echo();
                nodelay(stdscr, FALSE);
                mvprintw(row+1, 0, "Enter PID to kill (SIGTERM): ");
                char buf[32];
                getnstr(buf, 31);
                int pid = atoi(buf);
                int res = kill(pid, SIGTERM);
                if (res == 0) mvprintw(row+2,0,"SIGTERM sent to %d", pid);
                else mvprintw(row+2,0,"Failed to kill %d: %s", pid, strerror(errno));
                noecho();
                nodelay(stdscr, TRUE);
                killedSomething = true;
                break;
            } else if (ch == ERR) {
                // no input
            }
            napms(100); // 100 ms
        }
        if (killedSomething) {
            // immediate refresh in next loop iteration
            continue;
        }
    }

    endwin();
    return 0;
}
