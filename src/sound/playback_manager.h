
#pragma once

#include <memory>
#include <list>
#include <thread>
#include <condition_variable>

#include "sound_file.h"
#include "playback.h"
#include "music_queue.h"

#include "playback_state.h"

namespace starling
{
    /*
    * Handle playback state. Playback happens in a different thread owned by the PlaybackManager.
    * playback manager maintains this thread and the states of this thread. The thread will block
    * until there is something to play.
    */
    class PlaybackManager
    {
    public:
        PlaybackManager();

        ~PlaybackManager();

        PlaybackManager(const PlaybackManager&) = delete;
        PlaybackManager(PlaybackManager&& other);

        PlaybackManager& operator=(const PlaybackManager&) = delete;
        PlaybackManager& operator=(PlaybackManager&&);

        const SoundFile* queue(std::unique_ptr< SoundFile > file);

        const SoundFile* queue(const std::filesystem::path& file_path);

        void play();

        void play(const SoundFile* queue_item);

        void previous_song();

        void next_song();

        void pause();

        void stop();

        PlaybackState state();

        const SoundFile* currently_playing_song();

        void seek(size_t seek_seconds);
    private:
        void setup_sound_player(const SoundFile* song);
        void play_song(SoundFile* song);

        void playback_thread();
    private:
        std::list< std::unique_ptr< SoundFile > > file_queue;
        std::mutex state_mutex;
        std::mutex current_song_mutex;
        std::condition_variable state_condition;
        PlaybackState current_state = PlaybackState::Paused;
        std::unique_ptr< SoundPlayer > sound_player = nullptr;
        QueuedSong current_song;

        std::thread worker_thread;
        //
        // TODO: Is there a better way to handle this besides a mutex?
        //
        std::mutex worker_thread_lock;
        bool running = true;

        size_t previous_song_frequency = 0;
        size_t previous_song_channels = 0;
        size_t previous_song_bits_per_sample = 0;
    };
}