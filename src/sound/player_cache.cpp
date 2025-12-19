
#include "player_cache.h"

namespace starling
{
    SoundPlayer* PlayerCache::get_player(const SoundFile* soundfile)
    {
        int settings = sound_file_settings(soundfile);
        if (current_player_settings != settings)
        {
            current_player = std::make_unique<SoundPlayer>("starling", "music", soundfile->channels(), soundfile->frequency(), soundfile->bits_per_sample());
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