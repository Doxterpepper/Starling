
#pragma once

#include "playback.h"
#include "sound_file.h"

namespace starling
{
    class PlayerCache
    {
    public:
        virtual SoundPlayer* get_player(const SoundFile* soundfile);

        int sound_file_settings(const SoundFile* soundfile);
    private:
        std::unique_ptr< SoundPlayer > current_player = nullptr;
        int current_player_settings = -1;
    };
}