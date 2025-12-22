
#pragma once

#include <mutex>
#include <string>

namespace starling {
class Trace {
    Trace(const std::string &name) : name(name) {}

    int get_count() const { return call_count; }

    const std::string &get_name() const { return name; }

    void increment() { call_count++; }

  private:
    std::string name = "";
    int call_count = 0;
};
} // namespace starling

#ifdef __DEBUG__
static std::mutex trace_mutex;
#define NEW_TRACE(name)                                                                                                                                                       \
    [[maybe_unused]] static std::string name##_name = #name;                                                                                                                  \
    [[maybe_unused]] static int name##_count = 0;

#define DebugTrace(name)                                                                                                                                                      \
    {                                                                                                                                                                         \
        std::lock_guard<std::mutex> trace_lock(trace_mutex);                                                                                                                  \
        std::cout << "Debug Trace - " << name##_name << " " << name##_count << " " << __FUNCTION__ << ":" << __LINE__ << std::endl;                                           \
        name##_count++;                                                                                                                                                       \
    }
#else
#define NEW_TRACE(name)
#define DebugTrace(name)
#endif
