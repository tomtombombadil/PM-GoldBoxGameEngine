// One shell command at a time on a thread of its own (the updater's and
// Send Logs' helper scripts in tools/ - they use the firmware's curl), its
// output kept for the screen that started it.
#pragma once

#include <string>

namespace task {

// False: one is running already.
bool start(const std::string& command);
bool running();
// True once, when the command has ended: its output (stdout) and exit code.
bool finished(std::string& output, int& code);

// 'text' made safe as one word of a shell command
std::string quoted(const std::string& text);
// The value of the output's line starting "KEY " ("" if none)
std::string field(const std::string& output, const char* key);

} // namespace task
