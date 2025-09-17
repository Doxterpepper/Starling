
#include "playback.h"

#include <iostream>

namespace starling
{
    SoundPlayer::SoundPlayer(const std::string& application_name, const std::string& stream_name, size_t channels = 2, size_t rate = 44100)
    {
        pulse_settings.format = PA_SAMPLE_S16NE;
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

    void SoundPlayer::set_buffer(PlaybackBuffer< uint8_t >* buffer)
    {
        sound_buffer = buffer;
    }

    void SoundPlayer::play_buffer()
    {
        while (sound_buffer->has_data())
        {
            const std::vector< uint8_t >& playback_data = sound_buffer->peek_front();
            int error = 0;
            int result = pa_simple_write(pulse_simple, playback_data.data(), playback_data.size(), &error);
            sound_buffer->pop_front();

            if (result < 0)
            {
                std::cerr << "pa_simple_write failed " << pa_strerror(error) << std::endl;
            }
        }
    }
}