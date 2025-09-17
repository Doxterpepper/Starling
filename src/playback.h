
#pragma once

#include <list>

#include <pulse/simple.h>
#include <pulse/error.h>

namespace musicman
{
    //
    // Starting off with the linux implementation. There will need to be a different API called for windows and mac(?).
    // MacOs will need more research, I don't know anything about their sound APIs. For linux, I'm using pulseaudio. 
    // This seems better than ALSA, and probably more portable for different linux systems. I'll consider ALSA later
    // if it becomes a need. I think my audience will usually have pulseaudio these days.
    //
    // Pulse audio is clear in its documentation that they are not thread safe. I plan on a multithreaded design, though,
    // but I plan to only allow pulse audio to run on one thread. The theory right now is to have a continuous buffer
    // handed to pulse audio that is irrespective of the song being played. Another thread should manage the song queue,
    // reading the song data, transcoding it if necessary, and adding it to the play buffer. This should allow seamless
    // transition between songs.
    //
    // In the case of skipping or tracking, a little delay should be okay. But the primary goal is to remove all delay
    // between songs. This way we don't get gaps, even small ones, between songs that should flow seamlessly.
    class SoundPlayer
    {
    public:
        SoundPlayer(const std::string& application_name, const std::string& stream_name, size_t channels, size_t rate);
        SoundPlayer(const SoundPlayer&) = delete;
        SoundPlayer(SoundPlayer&&);
        ~SoundPlayer();

        SoundPlayer& operator=(const SoundPlayer&) = delete;
        SoundPlayer& operator=(SoundPlayer&&);

    private:
        pa_simple* pulse_simple = nullptr;
        pa_sample_spec pulse_settings{};

        PlaybackBuffer< uint8_t > playback_buffer;
    };
}