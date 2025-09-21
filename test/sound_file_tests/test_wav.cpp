
#include <vector>
#include <filesystem>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <sound/sound_file.h>

TEST_CASE("Read wav header.", "[wav]")
{
    std::cout << std::filesystem::current_path() << std::endl;
    starling::WavFile2 wav_file("./sound_file_tests/sine-24le.wav");

    CHECK(wav_file.file_Type_bloc_id() == "RIFF");
    CHECK(wav_file.file_size() == 7938036);
    CHECK(wav_file.file_format_id() == "WAVE");
    CHECK(wav_file.format_block_id() == "fmt ");
    CHECK(wav_file.bloc_size() == 16);
    CHECK(wav_file.audio_format() == 1);
    CHECK(wav_file.channels() == 2);
    CHECK(wav_file.frequency() == 44100);
    CHECK(wav_file.byte_per_sec() == 264600);
    CHECK(wav_file.bytes_per_block() == 6);
    CHECK(wav_file.bits_per_sample() == 24);
}