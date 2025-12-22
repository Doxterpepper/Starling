
#include "song_time.h"

#include <sstream>

namespace starling_ui {
std::string time_from_int(size_t time_seconds) {
    int hours = time_seconds / 60 / 60;
    int minutes = (time_seconds / 60) % 60;
    int seconds = time_seconds % 60;

    std::string final_str = "";

    if (hours) {
        if (hours < 10) {
            final_str += "0";
        }

        std::string hours_str = std::to_string(hours);
        final_str += hours_str + ":";
    }

    std::string minutes_str = std::to_string(minutes);
    std::string seconds_str = std::to_string(seconds);

    if (minutes < 10) {
        final_str += "0";
    }

    final_str += minutes_str + ":";

    if (seconds < 10) {
        final_str += "0";
    }

    final_str += seconds_str;

    return final_str;
}
} // namespace starling_ui