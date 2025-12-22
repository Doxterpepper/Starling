
#pragma once

#include <condition_variable>
#include <list>
#include <memory>
#include <thread>

#include "music_queue.h"
#include "playback.h"
#include "sound_file.h"

#include "music_queue.h"
#include "playback_engine.h"
#include "playback_state.h"
#include "player_cache.h"

namespace starling {
/*
 * Handle playback state. Playback happens in a different thread owned by the
 * PlaybackManager. playback manager maintains this thread and the states of
 * this thread. The thread will block until there is something to play.
 *
 * Currently refactoring into only control. Shouldn't be responsible for
 * maintaining the queue or playing music.
 */
class PlaybackManager {
  public:
    PlaybackManager(PlaybackEngine *engine, MusicQueue *song_queue);

    ~PlaybackManager();

    PlaybackManager(const PlaybackManager &) = delete;
    PlaybackManager(PlaybackManager &&other) = default;

    PlaybackManager &operator=(const PlaybackManager &) = delete;
    PlaybackManager &operator=(PlaybackManager &&) = default;

    const SoundFile *queue(std::unique_ptr<SoundFile> file);

    const SoundFile *queue(const std::filesystem::path &file_path);

    void play();

    void play(const SoundFile *queue_item);

    void previous_song();

    void next_song();

    void pause();

    void stop();

    PlaybackState state();

    const SoundFile *currently_playing_song();

    void seek(size_t seek_seconds);

  private:
    void lock_thread();
    void unlock_thread();
    void set_state(PlaybackState state);

    void playback_thread();

  private:
    PlaybackEngine *engine = nullptr;
    MusicQueue *song_queue = nullptr;
    std::mutex state_mutex;
    std::condition_variable state_condition;
    PlaybackState current_state = PlaybackState::Stopped;

    std::thread worker_thread;
    std::condition_variable thread_condition;
    //
    // TODO: Is there a better way to handle this besides a mutex?
    //
    std::mutex worker_thread_lock;
    bool running = true;
};
} // namespace starling