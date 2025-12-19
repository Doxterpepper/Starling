
#include "player_cache.h"

#include <chrono>

namespace starling
{
    SoundPlayer* PlayerCache::get_player(const SoundFile* soundfile)
    {

        //
        // I'd like a better method to do this, but for now we just track what each previous song settings were and
        // recreate the SoundPlayer if it needs new settings. It's a little awkward, but it works.
        //
        // This is done for performance reasons. The goal is to make the transitions from one song to the next
        // as seamless as possible. So only do this work when absolutely necessary. It takes on the order of 24ms to create
        // a SoundPlayer. That's a noticeable gab in the sound.
        //
        int settings = sound_file_settings(soundfile);
        if (current_player_settings != settings)
        {
            //
            // This is an expensive operation. Track the time spent doing this for awareness.
            //
            auto create_playback_start = std::chrono::high_resolution_clock::now();
            current_player = std::make_unique<SoundPlayer>("starling", "music", soundfile->channels(), soundfile->frequency(), soundfile->bits_per_sample());
            auto create_playback_stop = std::chrono::high_resolution_clock::now();

            auto playback_create_duration = std::chrono::duration_cast<std::chrono::microseconds>(create_playback_stop - create_playback_start);
            std::cout << "Create playback object in " << playback_create_duration.count() << " microseconds." << std::endl;

            current_player_settings = settings;
        }

        return current_player.get();
    }

    int PlayerCache::sound_file_settings(const SoundFile* soundfile)
    {
        return soundfile->bits_per_sample()
            + soundfile->channels()
            + soundfile->frequency();
    }
}