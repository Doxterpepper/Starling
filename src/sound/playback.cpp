
#include "playback.h"

#include <iostream>

namespace starling
{
    SoundPlayer::SoundPlayer(const std::string& application_name, const std::string& stream_name, size_t channels, size_t rate, size_t bits_per_sample)
    {
        if (bits_per_sample == 24)
        {
            pulse_settings.format = PA_SAMPLE_S24LE;

        }
        else if (bits_per_sample == 16)
        {
            pulse_settings.format = PA_SAMPLE_S16LE;
        }

        pulse_settings.channels = channels;
        pulse_settings.rate = rate;

        int error = 0;
        pulse_simple = pa_simple_new(
            nullptr,                  // Use the default server.
            application_name.c_str(), // Our application's name.
            PA_STREAM_PLAYBACK,         // Use the default device.
            nullptr,
            stream_name.c_str(),       // name of this stream.
            &pulse_settings,
            nullptr,                  // Use default channel map
            nullptr,                  // Use default buffering attributes.
            &error    
        );

        if (!pulse_simple)
        {
            // Raise an exception.
        }
    }

    SoundPlayer::~SoundPlayer()
    {
        if (pulse_simple)
        {
            flush();
            pa_simple_free(pulse_simple);
        }
    }

    SoundPlayer::SoundPlayer(SoundPlayer&& other)
    {
        pulse_simple = other.pulse_simple;
        other.pulse_simple = nullptr;
        pulse_settings = other.pulse_settings;
    }

    SoundPlayer& SoundPlayer::operator=(SoundPlayer&& other)
    {
        pulse_simple = other.pulse_simple;
        other.pulse_simple = nullptr;
        pulse_settings = other.pulse_settings;
        return *this;
    }

    void SoundPlayer::flush()
    {
        int error = 0;
        int result = pa_simple_flush(pulse_simple, &error);
        if (result < 0)
        {
            std::cerr << "Could not flush buffer. " << pa_strerror(error) << std::endl;
        }
    }
}