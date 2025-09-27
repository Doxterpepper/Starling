
#include <vector>
#include <filesystem>
#include <iostream>
#include <chrono>

#include <catch2/catch_test_macros.hpp>

#include <sound/sound_file.h>

TEST_CASE("Read wav header.", "[wav]")
{
    //std::cout << std::filesystem::current_path() << std::endl;

    auto start_wav_creation = std::chrono::high_resolution_clock::now();
    starling::WavFile2 wav_file("./sound_file_tests/sine-24le.wav");
    auto end_wav_creation = std::chrono::high_resolution_clock::now();

    //
    // Note: The time to create a wav file will depend on the speed of the storage device.
    // In my case, I'm seeing times on average of 36 microseconds.
    //
    auto creation_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_wav_creation - start_wav_creation);
    std::cout << "WavFile construction took " << creation_duration.count() << " us";
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
    CHECK(wav_file.data_size() == 7938000);
}

TEST_CASE("Read 24-bit wav buffer", "[wav]")
{
    //std::cout << std::filesystem::current_path() << std::endl;

    auto start_wav_creation = std::chrono::high_resolution_clock::now();
    starling::WavFile2 wav_file("./sound_file_tests/sine-24le.wav");
    auto end_wav_creation = std::chrono::high_resolution_clock::now();

    //
    // Note: The time to create a wav file will depend on the speed of the storage device.
    // In my case, I'm seeing times on average of 36 microseconds.
    //
    auto creation_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_wav_creation - start_wav_creation);
    std::cout << "WavFile construction took " << creation_duration.count() << " us";
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
    CHECK(wav_file.data_size() == 7938000);

    std::vector<uint8_t> expected_buffer = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x6A, 0x06, 0x49, 0x6A, 0x06,
        0x21, 0xCE, 0x0C, 0x22, 0xCE, 0x0C,
        0x14, 0x25, 0x13, 0x13, 0x25, 0x13
    };

    std::vector<uint8_t> buffer(expected_buffer.size());

    size_t read_bytes = wav_file.read_sound_chunk(buffer.data(), buffer.size());

    CHECK(read_bytes == expected_buffer.size());
    //CHECK(std::equal(buffer.begin(), buffer.end(), expected_buffer.begin(), expected_buffer.end()));

    for (size_t i = 0; i < buffer.size(); i++)
    {
        CHECK(buffer[i] == expected_buffer[i]);
    }
}

TEST_CASE("Search binary file 1", "[binary_search]")
{
    uint8_t test_data[] = {
        0x71, 0x12, 0x0b, 0xaf, 0xe1, 0xe2, 0x4d, 0x6c,
        0x26, 0x4f, 0xd5, 0x12, 0x29, 0xe4, 0xaf, 0x5b,
        0x16, 0xca, 0x72, 0x9a, 0x92, 0x70, 0x1a, 0xe7,
        0x08, 0x63, 0xae, 0xbf, 0xfd, 0x9c, 0x90, 0xc6
    };

    std::vector< uint8_t > pattern = { 0x71, 0x12, 0x0b };

    FILE* binary_file = fopen("test-file.bin", "wb");
    size_t written_bytes = fwrite(test_data, sizeof(test_data[0]), sizeof(test_data), binary_file);
    fclose(binary_file);
    CHECK(written_bytes == sizeof(test_data));

    long position = starling::file_search("test-file.bin", pattern);

    CHECK(position == 0);
}

TEST_CASE("Search binary file 2", "[binary_search]")
{
    uint8_t test_data[] = {
        0x71, 0x12, 0x0b, 0xaf, 0xe1, 0xe2, 0x4d, 0x6c,
        0x26, 0x4f, 0xd5, 0x12, 0x29, 0xe4, 0xaf, 0x5b,
        0x16, 0xca, 0x72, 0x9a, 0x92, 0x70, 0x1a, 0xe7,
        0x08, 0x63, 0xae, 0xbf, 0xfd, 0x9c, 0x90, 0xc6
    };

    std::vector< uint8_t > pattern = { 0xe4, 0xaf, 0x5b };

    FILE* binary_file = fopen("test-file.bin", "wb");
    size_t written_bytes = fwrite(test_data, sizeof(test_data[0]), sizeof(test_data), binary_file);
    fclose(binary_file);
    CHECK(written_bytes == sizeof(test_data));

    long position = starling::file_search("test-file.bin", pattern);

    CHECK(position == 13);
}