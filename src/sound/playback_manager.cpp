
#include "playback_manager.h"

namespace starling {
PlaybackManager::PlaybackManager(PlaybackEngine *engine, MusicQueue *song_queue) : engine(engine), song_queue(song_queue) {
    lock_thread();
    worker_thread = std::thread(&PlaybackManager::playback_thread, this);
}

PlaybackManager::~PlaybackManager() {
    running = false;
    engine->stop();
    current_state = PlaybackState::Stopped;
    unlock_thread();
    worker_thread.join();
}

const SoundFile *PlaybackManager::queue(std::unique_ptr<SoundFile> file) {
    SoundFile *file_ptr = file.get();
    song_queue->add_song(std::move(file));
    return file_ptr;
}

const SoundFile *PlaybackManager::queue(const std::filesystem::path &file_path) {
    std::unique_ptr<SoundFile> sound_file = open_sound_file(file_path);
    return queue(std::move(sound_file));
}

void PlaybackManager::play() {
    if (song_queue->size() == 0 || song_queue->current_song() == nullptr) {
        return;
    }
    DebugTrace(playback_locking) set_state(PlaybackState::Playing);
    DebugTrace(playback_locking) std::cout << "Current state in play - " << state() << std::endl;
    unlock_thread();
}

void PlaybackManager::play(const SoundFile *queue_item) {
    current_state = PlaybackState::Stopped;

    if (!song_queue->set_current_song(queue_item)) {
        throw std::exception();
    }

    play();
}

void PlaybackManager::playback_thread() {
    while (running) {
        //
        // I had this down to a 2μs turnaround time in the original version
        // where it ran in main. I imagine this is because there were no
        // function calls involved. Now we're sitting at a 10μs turnaround
        // assuming we don't need to re-create the sound_player.
        //
        auto end_turnaround_time = std::chrono::high_resolution_clock::now();
        auto start_turnaround_time = std::chrono::high_resolution_clock::now();
        std::cout << "Current song - " << song_queue->current_song() << std::endl;
        if (state() == PlaybackState::Playing && song_queue->current_song() != nullptr) {
            DebugTrace(playback_locking)
                //
                // This is the hot path. We don't want anything too expensive
                // running in this thread or in this loop. One of the main goals
                // of this project is low latency between songs. There should be
                // no perceptible gap between songs like "Speak to me" and
                // "Breathe (In the Air)". The most expensive call is
                // setup_sound_player clocking in around 27ms. This function
                // creates a starling::SoundPlayer which is pretty expensive.
                // This player is only constructed when a new file playback type
                // is encountered. The settings that will define this are
                // channels, sampling rate (frequency), and bits per sample. If
                // any of these settings change between files a new SoundPlayer
                // is constructed and a gap between songs will likely be
                // audible. The hope is that songs that should flow together
                // will have the same settings.
                //
                // Sub 500μs should not be perceptible from what I can tell. I'm
                // aiming for less than 50μs to be extra safe. In my first
                // iteration of this project, I had it at 2μs, but it was all
                // running in the main thread with no function calls and on a
                // ryzen 9 7950.
                //
                // On my laptop with an intel i7-8665U I can get a turnaround of
                // 33μs. This is sufficient for now but I'd like to get it even
                // faster.
                //
                // Note: Turns out this is all running the debug builds. When
                // running release with -O3 optimizations, it's a turnaround
                // time of 1μs. Likely optimizes the function calls. That is
                // between songs with the same playback parameters so reusing
                // the SoundPlayer between songs.
                //
                SoundFile *song = song_queue->current_song();

            end_turnaround_time = std::chrono::high_resolution_clock::now();
            auto turnaround_time_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_turnaround_time - start_turnaround_time);
            std::cout << "turnaround in " << turnaround_time_duration.count() << " microseconds." << std::endl;

            engine->play_song(song);
            start_turnaround_time = std::chrono::high_resolution_clock::now();

            if (state() != PlaybackState::Paused) {
                song->reset();
            }

            if (state() == PlaybackState::Playing) {
                std::cout << "Next song." << std::endl;
                song_queue->next();
            }
        } else {
            //
            // The controlling thread will wait for the playback thread to
            // turnaround to this point when changing the state. The playback
            // thread will continually play the current song until it reaches
            // the end of the sound data. Changing the state from Playing to
            // Stopped or Paused will break out of that playing loop and
            // eventually reach this point. We notify all when we reach this
            // point so they can continue under the assumption that we are
            // officially stopped.
            //
            // This allows for actions like previous_song, or next_song where we
            // want to enter a stopped state, change the iterator postion, then
            // play again.
            //
            DebugTrace(playback_locking) notify_turnaround();
            DebugTrace(playback_locking) thread_wait();
        }
    }
}

void PlaybackManager::stop() {
    if (state() == PlaybackState::Stopped) {
        return;
    }

    if (state() == PlaybackState::Paused) {
        auto current_song = song_queue->current_song();
        current_song->reset();
        return;
    }

    set_state(PlaybackState::Stopped);
    engine->stop();
    lock_thread();
    DebugTrace(playback_locking) wait_turnaround();
    DebugTrace(playback_locking)
}

void PlaybackManager::previous_song() {
    if (song_queue->size() == 0) {
        return;
    }

    stop();
    song_queue->previous();
    play();
}

void PlaybackManager::next_song() {
    if (song_queue->size() == 0) {
        return;
    }

    stop();
    song_queue->next();
    //
    // If we go on past the end of the queue, we won't play anything.
    //
    play();
}

void PlaybackManager::pause() {
    auto playback_state = state();
    if (playback_state == PlaybackState::Paused || playback_state == PlaybackState::Stopped) {
        return;
    }

    set_state(PlaybackState::Paused);
    engine->stop();
    DebugTrace(playback_locking) lock_thread();
    DebugTrace(playback_locking) wait_turnaround();
    DebugTrace(playback_locking)
}

PlaybackState PlaybackManager::state() {
    std::lock_guard<std::mutex> state_lock(state_mutex);
    return current_state;
}

void PlaybackManager::set_state(PlaybackState state) {
    std::lock_guard<std::mutex> state_lock(state_mutex);
    std::cout << "setting state - " << state << std::endl;
    current_state = state;
}

const SoundFile *PlaybackManager::currently_playing_song() { return song_queue->current_song(); }

void PlaybackManager::seek(size_t seek_seconds) {
    auto current_song_ptr = song_queue->current_song();
    current_song_ptr->seek_song(seek_seconds);
}

void PlaybackManager::lock_thread() {
    /*
    for (size_t i = 0; i < 5; i++)
    {
        if (worker_thread_lock.try_lock())
        {
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    //throw std::exception();
    */
}

void PlaybackManager::unlock_thread() {
    DebugTrace(playback_locking) std::lock_guard lk(worker_thread_lock);
    std::cout << "Current state - " << state() << std::endl;
    thread_condition.notify_all();
    DebugTrace(playback_locking)
    // worker_thread_lock.unlock();
}

void PlaybackManager::wait_turnaround() {
    std::unique_lock turnaround_lock(turnaround_mutex);
    state_condition.wait(turnaround_lock);
}

void PlaybackManager::notify_turnaround() {
    std::lock_guard<std::mutex> turnaround_lock(turnaround_mutex);
    state_condition.notify_all();
}

void PlaybackManager::thread_wait() {
    DebugTrace(playback_locking) std::unique_lock<std::mutex> play_guard(worker_thread_lock);
    DebugTrace(playback_locking) if (state() == PlaybackState::Playing) { set_state(PlaybackState::Stopped); }
    DebugTrace(playback_locking) thread_condition.wait(play_guard);
    DebugTrace(playback_locking)
}
} // namespace starling