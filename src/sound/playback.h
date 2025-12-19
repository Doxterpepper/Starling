
#pragma once

#ifdef _WIN32
#error "Windows not supported yet."
#elif __APPLE__
#error "Apple not support yet"
#endif

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#ifdef __TEST_DEF__
#elif __linux__
#include <pulse/error.h>
#include <pulse/simple.h>
#endif

namespace starling {
//
// Starting off with the linux implementation. There will need to be a different
// API called for windows and mac(?). MacOs will need more research, I don't
// know anything about their sound APIs. For linux, I'm using pulseaudio. This
// seems better than ALSA, and probably more portable for different linux
// systems. I'll consider ALSA later if it becomes a need. I think my audience
// will usually have pulseaudio these days.
//
// Pulse audio is clear in its documentation that they are not thread safe. I
// plan on a multithreaded design, and I plan to only allow pulse audio to run
// on one thread.
//
// In the case of skipping or tracking, a little delay should be okay. But the
// primary goal is to remove all delay between songs.
class SoundPlayer {
public:
  SoundPlayer(const std::string &application_name,
              const std::string &stream_name, size_t channels, size_t rate,
              size_t bits_per_sample);
  SoundPlayer(const SoundPlayer &) = delete;
  SoundPlayer(SoundPlayer &&);
  ~SoundPlayer();

  SoundPlayer &operator=(const SoundPlayer &) = delete;
  SoundPlayer &operator=(SoundPlayer &&);

  template <typename buffer_type>
  void play_buffer(const std::vector<buffer_type> &data, size_t length)
#ifdef __TEST_DEF__
  {
    call_count++;
  }
#elif __linux__
  {
    int error = 0;
    int result = pa_simple_write(pulse_simple, data.data(), length, &error);

    if (result < 0) {
      std::cerr << "pa_simple_write_failed " << pa_strerror(error) << std::endl;
    }
  }
#endif

#ifdef __TEST_DEF__
  int called() const;
#endif

  void flush();

private:
#ifdef __TEST_DEF__
  int call_count = 0;
#elif __linux__
  pa_simple *pulse_simple = nullptr;
  pa_sample_spec pulse_settings{};
#endif
};
} // namespace starling