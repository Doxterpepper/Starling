
#include "playback_engine.h"

namespace starling
{
    PlaybackEngine::PlaybackEngine(PlayerCache* cache) :
        player_cache(cache)
    {
    }

    void PlaybackEngine::play_song(SoundFile* song)
    {
        running = true;
        auto player = player_cache->get_player(song);

        size_t read_bytes = 0;
        std::vector<uint8_t> sound_buffer(song->bytes_per_block() * buffer_multiplier);
        do
        {
            read_bytes = song->read_sound_chunk(sound_buffer.data(), sound_buffer.size());
            if (read_bytes)
            {
                player->play_buffer(sound_buffer, read_bytes);
            }
        } while(read_bytes && running);
    }

    void PlaybackEngine::stop()
    {
        running = false;
    }
}