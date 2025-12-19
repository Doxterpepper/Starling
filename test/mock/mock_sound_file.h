
#pragma once

#include <functional>

class MockSoundFile : public starling::SoundFile
{
public:
    MockSoundFile(const std::filesystem::path& file_path) : SoundFile(file_path) {}
    size_t channels() const override
    {
        return chan;
    }

    size_t frequency() const override
    {
        return freq;
    }

    size_t bits_per_sample() const override
    {
        return bps;
    }

    size_t read_sound_chunk(uint8_t* data, size_t data_length) override
    {
        read_callback();
        if (mock_sound_length > 0)
        {
            std::memset(data, 0, data_length);
            --mock_sound_length;
            return data_length;
        }
        return 0;
    }

    size_t bytes_per_block() const override
    {
        return bpb;
    }

    size_t sound_length() const override
    {
        return 0;
    }

    size_t current_time() const override
    {
        return 0;
    }

    std::string name() const override
    {
        return "test";
    }

    void seek_song(size_t) override
    {
    }

    void reset() override
    {
        reset_call_count++;
    }

    size_t reset_called()
    {
        return reset_call_count;
    }

    void set_frequency(size_t new_freq)
    {
        freq = new_freq;
    }

    void set_chan(size_t new_chan)
    {
        chan = new_chan;
    }

    void set_bps(size_t new_bps)
    {
        bps = new_bps;
    }

    void set_bytes_per_block(size_t bytes)
    {
        bpb = bytes;
    }

    void set_num_sound_chunks(size_t chunks)
    {
        mock_sound_length = chunks;
    }

    void set_read_callback(std::function<void()> callback)
    {
        read_callback = callback;
    }
private:
    size_t freq = 0;
    size_t chan = 0;
    size_t bps = 0;
    size_t bpb = 0;
    size_t mock_sound_length = 1;
    std::function<void()> read_callback = []() {};
    size_t reset_call_count = 0;
};