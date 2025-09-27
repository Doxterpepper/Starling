
#pragma once

#include "sound_file.h"

namespace starling
{
    /*
    * Handle playback state.
    */
    class PlaybackManager
    {
    public:
    private:
        std::list< std::unique_ptr< SoundFile > > file_queue;
    };
}