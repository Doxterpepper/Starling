
#include <catch2/catch_test_macros.hpp>

#include <mock/mock_cache.h>
#include <mock/mock_sound_file.h>
#include <sound/playback.h>
#include <sound/playback_engine.h>

namespace starling::test {
TEST_CASE("Play music to end.") {
    MockCache mock_cache;
    SoundPlayer sound_player("test app", "new string", 2, 1, 1);
    mock_cache.set_player(&sound_player);

    PlaybackEngine engine(&mock_cache);

    std::filesystem::path file_path("not a real path");

    const size_t chunks = 100;
    MockSoundFile mock_sound(file_path);
    mock_sound.set_bytes_per_block(1);
    mock_sound.set_num_sound_chunks(chunks);
    engine.play_song(&mock_sound);

    CHECK(mock_cache.called() == 1);
    CHECK(sound_player.called() == chunks);
}

TEST_CASE("Play music until stopped.") {
    MockCache mock_cache;
    SoundPlayer sound_player("test app", "new string", 2, 1, 1);
    mock_cache.set_player(&sound_player);

    PlaybackEngine engine(&mock_cache);

    std::filesystem::path file_path("not a real path");

    const size_t chunks = 100;
    const size_t target_stop = 5;
    MockSoundFile mock_sound(file_path);
    mock_sound.set_bytes_per_block(1);
    mock_sound.set_num_sound_chunks(chunks);
    size_t call_count = 0;
    mock_sound.set_read_callback([&]() {
        call_count++;
        if (call_count == target_stop) {
            engine.stop();
        }
    });
    engine.play_song(&mock_sound);

    CHECK(mock_cache.called() == 1);
    CHECK(sound_player.called() == target_stop);
}

TEST_CASE("Play file no data.") {
    MockCache mock_cache;
    SoundPlayer sound_player("test app", "new string", 2, 1, 1);
    mock_cache.set_player(&sound_player);

    PlaybackEngine engine(&mock_cache);

    std::filesystem::path file_path("not a real path");

    const size_t chunks = 0;
    MockSoundFile mock_sound(file_path);
    mock_sound.set_bytes_per_block(1);
    mock_sound.set_num_sound_chunks(chunks);
    engine.play_song(&mock_sound);

    CHECK(mock_cache.called() == 1);
    CHECK(sound_player.called() == chunks);
}
} // namespace starling::test
