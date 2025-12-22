
#include <catch2/catch_test_macros.hpp>

#include <ui/song_time.h>

TEST_CASE("00 seconds", "[ui]") {
    std::vector<std::string> expected_strings = {"00:00", "00:01", "00:02", "00:03", "00:04", "00:05", "00:06", "00:07", "00:08", "00:09", "00:10", "00:11"};

    for (size_t time_seconds = 0; time_seconds < expected_strings.size(); time_seconds++) {
        std::string time_str = starling_ui::time_from_int(time_seconds);

        CHECK(time_str == expected_strings[time_seconds]);
    }
}

TEST_CASE("00 minutes", "[ui]") {
    std::vector<std::string> expected_strings = {
        "00:00", "01:00", "02:00", "03:00", "04:00", "05:00", "06:00", "07:00", "08:00", "09:00", "10:00", "11:00",
    };

    for (size_t i = 0; i < expected_strings.size(); i++) {
        std::string time_str = starling_ui::time_from_int(i * 60);

        CHECK(time_str == expected_strings[i]);
    }
}

TEST_CASE("00 hours", "[ui]") {
    std::vector<std::string> expected_strings = {
        "00:00", "01:00:00", "02:00:00", "03:00:00", "04:00:00", "05:00:00", "06:00:00", "07:00:00", "08:00:00", "09:00:00", "10:00:00", "11:00:00",
    };

    for (size_t i = 0; i < expected_strings.size(); i++) {
        std::string time_str = starling_ui::time_from_int(i * 60 * 60);

        CHECK(time_str == expected_strings[i]);
    }
}

TEST_CASE("00 hours and 00 minutes", "[ui]") {
    std::vector<std::string> expected_strings = {
        "00:00", "01:01:00", "02:02:00", "03:03:00", "04:04:00", "05:05:00", "06:06:00", "07:07:00", "08:08:00", "09:09:00", "10:10:00", "11:11:00",
    };

    for (size_t i = 0; i < expected_strings.size(); i++) {
        std::string time_str = starling_ui::time_from_int(i * 60 * 60 + i * 60);

        CHECK(time_str == expected_strings[i]);
    }
}