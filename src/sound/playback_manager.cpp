
#include "playback_manager.h"

namespace starling
{
    PlaybackManager::PlaybackManager(PlaybackManager&& other)
    {
        file_queue = std::move(other.file_queue);
        current_state = other.current_state;
        other.current_state = PlaybackState::Paused;
    }

    PlaybackManager& PlaybackManager::operator=(PlaybackManager&& other)
    {
        file_queue = std::move(other.file_queue);
        current_state = other.current_state;
        other.current_state = PlaybackState::Paused;
        return *this;
    }

    void PlaybackManager::queue(std::unique_ptr< SoundFile > file)
    {
        file_queue.push_back(std::move(file));
        current_song = file_queue.begin();
        std::cout << "Queued song" << std::endl;
    }

    void PlaybackManager::queue(const std::filesystem::path& file_path)
    {
        std::unique_ptr<SoundFile> sound_file = open_sound_file(file_path);
        queue(std::move(sound_file));
    }

    void PlaybackManager::setup_sound_player(const SoundFile* song)
    {
        if (!song)
        {
            std::cerr << "Got null SoundFile while setting up sound player" << std::endl;
            return;
        }

        if (!sound_player
            || previous_song_bits_per_sample != song->bits_per_sample()
            || previous_song_channels != song->channels()
            || previous_song_frequency != song->frequency())
        {
            //
            // I'd like a better method to do this, but for now we just track what each previous song settings were and
            // recreate the SoundPlayer if it needs new settings. It's a little awkward, but it works.
            //
            // This is done for performance and speed reasons. The goal is to make the transitions from one song to the next
            // as seamless as possible. So only do this work when absolutely necessary. It takes on the order of 24 ms to create
            // a SoundPlayer. That's a noticeable gab in the sound.
            //
            auto create_playback_start = std::chrono::high_resolution_clock::now();
            sound_player = std::make_unique<SoundPlayer>("starling", "music", song->channels(), song->frequency(), song->bits_per_sample());
            auto create_playback_stop = std::chrono::high_resolution_clock::now();

            //
            // This is an expensive operation. Track the time spent doing this for awareness.
            //
            auto playback_create_duration = duration_cast<std::chrono::microseconds>(create_playback_stop - create_playback_start);
            std::cout << "Create playback object in " << playback_create_duration.count() << " microseconds." << std::endl;

            previous_song_bits_per_sample = song->bits_per_sample();
            previous_song_channels = song->channels();
            previous_song_frequency = song->frequency();
        }
    }

    void PlaybackManager::play()
    {
        //
        // I had this down to a 2μs turnaround time in the original version where it ran in main. I imagine this is because there were
        // no function calls involved. Now we're sitting at a 10μs turnaround assuming we don't need to re-create the sound_player.
        //
        current_state = PlaybackState::Playing;

        auto end_turnaround_time = std::chrono::high_resolution_clock::now();
        auto start_turnaround_time = std::chrono::high_resolution_clock::now();

        while (current_state == PlaybackState::Playing && current_song != file_queue.end())
        {
            SoundFile* song = current_song->get();
            std::cout << "Playing current song " << song->name() << std::endl;
            setup_sound_player(song);

            end_turnaround_time = std::chrono::high_resolution_clock::now();
            auto turnaround_time_duration = duration_cast<std::chrono::microseconds>(end_turnaround_time - start_turnaround_time);
            std::cout << "turnaround in " << turnaround_time_duration.count() << " microseconds." << std::endl;

            play_song(song);
            start_turnaround_time = std::chrono::high_resolution_clock::now();
            ++current_song;
        }
        current_state = PlaybackState::Paused;
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

    void PlaybackManager::stop()
    {
        current_state = PlaybackState::Stopped;
    }

    void PlaybackManager::previous_song()
    {
        stop();
        --current_song;
        play();
    }

    void PlaybackManager::next_song()
    {
        stop();
        ++current_song;
        play();
    }

    void PlaybackManager::pause()
    {
        current_state = PlaybackState::Paused;
    }

    PlaybackState PlaybackManager::state() const
    {
        return current_state;
    }
}