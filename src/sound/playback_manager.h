
#pragma once

#include <memory>
#include <list>

#include "sound_file.h"
#include "playback.h"

namespace starling
{
    enum PlaybackState
    {
        Playing,
        Paused,
        Stopped
    };

    /*
    * Handle playback state.
    */
    class PlaybackManager
    {
    public:
        PlaybackManager() = default;

        ~PlaybackManager() = default;

        PlaybackManager(const PlaybackManager&) = delete;
        PlaybackManager(PlaybackManager&& other);

        PlaybackManager& operator=(const PlaybackManager&) = delete;
        PlaybackManager& operator=(PlaybackManager&&);

        void queue(std::unique_ptr< SoundFile > file);

        void queue(const std::filesystem::path& file_path);

        void play();

        void previous_song();

        void next_song();

        void pause();

        void stop();

        PlaybackState state() const;
    private:
        void setup_sound_player(const SoundFile* song);
        void play_song(SoundFile* song);
    private:
        std::list< std::unique_ptr< SoundFile > > file_queue;
        PlaybackState current_state = PlaybackState::Paused;
        std::unique_ptr< SoundPlayer > sound_player = nullptr;
        std::list<std::unique_ptr<SoundFile>>::iterator current_song;

        size_t previous_song_frequency = 0;
        size_t previous_song_channels = 0;
        size_t previous_song_bits_per_sample = 0;
    };
}