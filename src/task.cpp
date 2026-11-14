#include "task.h"

#include <atomic>
#include <cstdio>
#include <mutex>
#include <sys/wait.h>
#include <thread>

namespace task {
namespace {

std::thread       worker;
std::mutex        lock;
std::string       out;
int               rc = 0;
std::atomic<bool> busy{false}, ready{false};

void run(std::string command)
{
    std::string got;
    int code = 127;
    if (FILE* p = popen(command.c_str(), "r")) {
        char buf[512];
        while (fgets(buf, sizeof buf, p)) got += buf;
        const int st = pclose(p);
        code = WIFEXITED(st) ? WEXITSTATUS(st) : 126;
    }
    {
        std::lock_guard<std::mutex> g(lock);
        out = got;
        rc = code;
    }
    ready = true;
    busy = false;
}

} // namespace

bool start(const std::string& command)
{
    if (busy) return false;
    if (worker.joinable()) worker.join();
    ready = false;
    busy = true;
    worker = std::thread(run, command);
    return true;
}

bool running() { return busy; }

bool finished(std::string& output, int& code)
{
    if (!ready) return false;
    if (worker.joinable()) worker.join();
    std::lock_guard<std::mutex> g(lock);
    output = out;
    code = rc;
    ready = false;
    return true;
}

std::string quoted(const std::string& text)
{
    std::string q = "'";
    for (char c : text) {
        if (c == '\'') q += "'\\''";
        else q += c;
    }
    return q + "'";
}

std::string field(const std::string& output, const char* key)
{
    const std::string k = std::string(key) + " ";
    size_t p = 0;
    while (p < output.size()) {
        size_t e = output.find('\n', p);
        if (e == std::string::npos) e = output.size();
        if (output.compare(p, k.size(), k) == 0) return output.substr(p + k.size(), e - p - k.size());
        p = e + 1;
    }
    return "";
}

} // namespace task
