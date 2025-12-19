
#pragma once

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

    size_t read_sound_chunk(uint8_t*, size_t) override
    {
        return 0;
    }

    size_t bytes_per_block() const override
    {
        return 0;
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
private:
    size_t freq = 0;
    size_t chan = 0;
    size_t bps = 0;
};