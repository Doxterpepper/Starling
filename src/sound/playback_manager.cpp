
#include "playback_manager.h"

namespace starling
{
    PlaybackManager::PlaybackManager(PlayerCache* cache, MusicQueue* song_queue) :
        player_cache(cache),
        song_queue(song_queue)
    {
        worker_thread_lock.lock();
        worker_thread = std::thread(&PlaybackManager::playback_thread, this);
    }

    PlaybackManager::~PlaybackManager()
    {
        running = false;
        current_state = PlaybackState::Stopped;
        worker_thread_lock.unlock();
        worker_thread.join();
    }

    const SoundFile* PlaybackManager::queue(std::unique_ptr< SoundFile > file)
    {
        SoundFile* file_ptr = file.get();
        song_queue->add_song(std::move(file));
        return file_ptr;
    }

    const SoundFile* PlaybackManager::queue(const std::filesystem::path& file_path)
    {
        std::unique_ptr<SoundFile> sound_file = open_sound_file(file_path);
        return queue(std::move(sound_file));
    }

    void PlaybackManager::play()
    {
        if (song_queue->size() == 0 || song_queue->current_song() == nullptr)
        {
            return;
        }
        std::lock_guard<std::mutex> guard(state_mutex);
        current_state = PlaybackState::Playing;
        worker_thread_lock.unlock();
    }

    void PlaybackManager::play(const SoundFile* queue_item)
    {
        current_state = PlaybackState::Stopped;
        
        if(!song_queue->set_current_song(queue_item))
        {
            throw std::exception();
        }

        play();
    }

    void PlaybackManager::play_song(SoundFile* song)
    {
        size_t read_bytes = 0;
        //
        // Use 128 so that we end up with a decent size buffer for both 16 bit and 24 bit audio that still
        // fits the alignment for both.
        //
        std::vector<uint8_t> sound_buffer(song->bytes_per_block() * 128);
        do
        {
            read_bytes = song->read_sound_chunk(sound_buffer.data(), sound_buffer.size());
            if (read_bytes)
            {
                sound_player->play_buffer(sound_buffer, read_bytes);
            }
        } while(read_bytes && current_state == PlaybackState::Playing);

        if (current_state != PlaybackState::Paused)
        {
            // Note: At the moment, the file is not closed until the file is deleted. May want to consider
            // releasing the file at this step.
            song->reset();
        }
    }

    void PlaybackManager::playback_thread()
    {
        while(running)
        {
            //
            // The controlling thread will wait for the playback thread to turnaround to this point when changing the state. The playback thread
            // will continually play the current song until it reaches the end of the sound data. Changing the state from Playing to Stopped or Paused
            // will break out of that playing loop and eventually reach this point. We notify all when we reach this point so they can continue under
            // the assumption that we are officially stopped.
            //
            // This allows for actions like previous_song, or next_song where we want to enter a stopped state, change the iterator postion, then play
            // again.
            //
            state_condition.notify_all();
            std::lock_guard<std::mutex> play_guard(worker_thread_lock);

            //
            // I had this down to a 2μs turnaround time in the original version where it ran in main. I imagine this is because there were
            // no function calls involved. Now we're sitting at a 10μs turnaround assuming we don't need to re-create the sound_player.
            //
            auto end_turnaround_time = std::chrono::high_resolution_clock::now();
            auto start_turnaround_time = std::chrono::high_resolution_clock::now();
            while (state() == PlaybackState::Playing && song_queue->current_song() != nullptr)
            {
                //
                // This is the hot path. We don't want anything too expensive running in this thread or in this loop.
                // One of the main goals of this project is low latency between songs. There should be no perceptible
                // gap between songs like "Speak to me" and "Breathe (In the Air)". The most expensive call is
                // setup_sound_player clocking in around 27ms. This function creates a starling::SoundPlayer which is
                // pretty expensive. This player is only constructed when a new file playback type is encountered. The
                // settings that will define this are channels, sampling rate (frequency), and bits per sample. If any
                // of these settings change between files a new SoundPlayer is constructed and a gap between songs will
                // likely be audible. The hope is that songs that should flow together will have the same settings.
                //
                // Sub 500μs should not be perceptible from what I can tell. I'm aiming for less than 50μs to be extra safe.
                // In my first iteration of this project, I had it at 2μs, but it was all running in the main thread
                // with no function calls and on a ryzen 9 7950.
                //
                // On my laptop with an intel i7-8665U I can get a turnaround of 33μs. This is sufficient for now but I'd
                // like to get it even faster.
                //
                // Note: Turns out this is all running the debug builds. When running release with -O3 optimizations, it's
                // a turnaround time of 1μs. Likely optimizes the function calls. That is between songs with the same playback
                // parameters so reusing the SoundPlayer between songs.
                //
                SoundFile* song = song_queue->current_song();
                sound_player = player_cache->get_player(song);

                end_turnaround_time = std::chrono::high_resolution_clock::now();
                auto turnaround_time_duration = duration_cast<std::chrono::microseconds>(end_turnaround_time - start_turnaround_time);
                std::cout << "turnaround in " << turnaround_time_duration.count() << " microseconds." << std::endl;

                play_song(song);
                start_turnaround_time = std::chrono::high_resolution_clock::now();

                if (state() == PlaybackState::Playing)
                {
                    song_queue->next();
                }
            }
        }
    }

    void PlaybackManager::stop()
    {
        std::unique_lock<std::mutex> state_lock(state_mutex);
        current_state = PlaybackState::Stopped;
        // I don't care about the result of try_lock.
        #pragma GCC diagnostic ignored "-Wunused-variable"
        bool _ = worker_thread_lock.try_lock();
        state_condition.wait(state_lock);
    }

    void PlaybackManager::previous_song()
    {
        if (song_queue->size() == 0)
        {
            return;
        }

        stop();
        song_queue->previous();
        play();
    }

    void PlaybackManager::next_song()
    {
        if (song_queue->size() == 0)
        {
            return;
        }

        stop();
        song_queue->next();
        //
        // If we go on past the end of the queue, we won't play anything.
        //
        play();
    }

    void PlaybackManager::pause()
    {
        std::unique_lock<std::mutex> state_lock(state_mutex);
        current_state = PlaybackState::Paused;
        bool _ = worker_thread_lock.try_lock();
        state_condition.wait(state_lock);
    }

    PlaybackState PlaybackManager::state()
    {
        std::lock_guard<std::mutex> state_lock(state_mutex);
        return current_state;
    }

    const SoundFile* PlaybackManager::currently_playing_song()
    {
        return song_queue->current_song();
    }

    void PlaybackManager::seek(size_t seek_seconds)
    {
        auto current_song_ptr = song_queue->current_song();
        current_song_ptr->seek_song(seek_seconds);
    }
}