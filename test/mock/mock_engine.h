
#pragma once

#include <functional>
#include <sound/playback_engine.h>

namespace starling::tests
{
    class MockEngine : public PlaybackEngine
    {
    public:
        MockEngine() : PlaybackEngine(nullptr) {};

        ~MockEngine() = default;
        void play_song(SoundFile* song) override
        {
            play_callback(song);
        }

        void set_callback(std::function<void(SoundFile*)>&& callback)
        {
            play_callback = std::move(callback);
        }
    private:
        std::function<void(SoundFile* song)> play_callback = [](SoundFile*) {};
    };
}