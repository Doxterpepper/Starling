
#include <list>
#include <memory>

#include "sound_file.h"

namespace starling
{
    class SongQueue
    {
    private:
        std::list< std::unique_ptr< SoundFile > > song_queue;
    };
}