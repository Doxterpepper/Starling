
#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdio>
#include <filesystem>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <memory>

namespace starling
{
    /**
     * Searches a binary file for the provided pattern. Returns the position of the first index of the pattern and sets
     * binary_file to the position of the buffer.
     * Returns -1 if the pattern is not found.
     */
    inline long file_search(FILE* binary_file, const std::vector<uint8_t>& pattern)
    {
        std::vector<uint8_t> search_buffer(255);
        size_t pattern_index = 0;
        size_t read_bytes = 0;

        do
        {
            read_bytes = fread(search_buffer.data(), sizeof(search_buffer[0]), search_buffer.size(), binary_file);

            for (size_t search_index = 0; search_index < read_bytes; search_index++)
            {
                if (pattern[pattern_index] == search_buffer[search_index])
                {
                    ++pattern_index;

                    if (pattern_index >= pattern.size())
                    {
                        long current_position = ftell(binary_file);
                        fseek(binary_file, current_position, SEEK_SET);
                        return static_cast<long>(current_position - read_bytes) + (static_cast<long>(search_index + 1) - static_cast<long>(pattern_index));
                    }
                }
                else
                {
                    pattern_index = 0;
                }
            }
        } while(read_bytes);
        return -1;
    }

    inline long file_search(const std::filesystem::path& file_path, const std::vector<uint8_t>& pattern)
    {
        FILE* file = fopen(file_path.c_str(), "rb");
        if (!file)
        {
            return -1;
        }
        
        auto found_position = file_search(file, pattern);
        fclose(file);
        return found_position;
    }

    inline long file_search(const std::filesystem::path& file_path, const std::string& pattern)
    {
        std::vector<uint8_t> pattern_buffer(pattern.begin(), pattern.end());
        return file_search(file_path, pattern_buffer);
    }

    inline long file_search(FILE* binary_file, const std::string& pattern)
    {
        std::vector<uint8_t> pattern_buffer(pattern.begin(), pattern.end());
        return file_search(binary_file, pattern_buffer);
    }

    /**
    * Things are still subject to change, but for now I want to have a common interface between pulse audio and the sound files. That is to say
    * pulse shouldn't need to worry about if it's a wav, flac, or mp3 file. Each file type should handle decoding and providing a valid buffer
    * for pulseaudio to play.
    *
    * How well this works in practice isn't totally clear though. The issue may be that I need SoundFile to have explicit ownership and lifetime. 
    * There can only be one. But other processes or objects may want access to them.
    */
    class SoundFile
    {
    public:
        SoundFile(const std::filesystem::path& file_path) :
            file_path(file_path)
        {
        }

        SoundFile(const SoundFile& other)
        {
            sound_file = other.sound_file;
            file_path = other.file_path;
        }

        SoundFile(SoundFile&& other)
        {
            sound_file = other.sound_file;
            other.sound_file = nullptr;
            file_path = other.file_path;
        }

        ~SoundFile()
        {
            if (sound_file)
            {
                fclose(sound_file);
                sound_file = nullptr;
            }
        }

        virtual size_t channels() const = 0;

        virtual size_t frequency() const = 0;

        virtual size_t bits_per_sample() const = 0;

        virtual size_t read_sound_chunk(uint8_t* buffer, size_t buffer_size) = 0;

        virtual size_t bytes_per_block() const = 0;

        virtual size_t sound_length() const = 0;

        virtual size_t current_time() const = 0;

        virtual std::string name() const
        {
            return file_path.filename();
        }

        virtual void seek_song(size_t time) = 0;

        virtual void reset()
        {
            if (sound_file)
            {
                //fseek(sound_file, 0, SEEK_SET);
                fclose(sound_file);
                sound_file = nullptr;
            }
        }

        SoundFile& operator=(const SoundFile& other)
        {
            sound_file = other.sound_file;
            file_path = other.file_path;

            return *this;
        }

        SoundFile& operator=(SoundFile&& other)
        {
            sound_file = other.sound_file;
            other.sound_file = nullptr;
            file_path = other.file_path;

            return *this;
        }

    protected:
        FILE* sound_file = nullptr;
        std::filesystem::path file_path;
    };

    class WavFile2 : public SoundFile
    {
    public:
        WavFile2(const std::filesystem::path& file_path) :
            SoundFile(file_path)
        {
            load_header();
            load_data_tag();
            file_bytes_in_1s = channels() * (bits_per_sample() / 8) * frequency();
        }

        std::string file_Type_bloc_id() const
        {
            return std::string(reinterpret_cast<const char*>(header), 4);
        }

        size_t file_size() const
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x04);
        }

        std::string file_format_id() const
        {
            return std::string(reinterpret_cast<const char*>(header) + 0x08, 4);
        }

        std::string format_block_id() const
        {
            return std::string(reinterpret_cast<const char*>(header + 0x0c), 4);
        }

        size_t bloc_size() const
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x10);
        }

        size_t audio_format() const
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x14);
        }

        size_t byte_per_sec() const
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x1c);
        }

        size_t bytes_per_block() const override
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x20);
        }

        std::string data_bloc_id() const
        {
            return std::string(reinterpret_cast<const char*>(header + 0x24));
        }

        size_t data_size() const
        {
            return data_length;
        }

        size_t bits_per_sample() const override
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x22);
        }

        size_t channels() const override
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x16);
        }

        size_t frequency() const override
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x18);
        }

        size_t sound_length() const override
        {
            return data_length / (bits_per_sample() * channels() * bits_per_sample() / 8);
        }

        size_t current_time() const override
        {
            if (sound_file == nullptr)
            {
                return 0;
            }

            size_t current_offset = ftell(sound_file);
            size_t current_position_sound_data = current_offset - (data_block_offset + 4);
            return current_position_sound_data / file_bytes_in_1s;
        }

        void seek_song(size_t offset_seconds) override
        {
            size_t bytes_offset = offset_seconds * file_bytes_in_1s;
            size_t time_position = data_block_offset + 4 + bytes_offset;
            fseek(sound_file, time_position, SEEK_SET);
        }

        size_t read_sound_chunk(uint8_t* buffer, size_t buffer_size) override
        {
            if (!sound_file)
            {
                sound_file = fopen(file_path.c_str(), "rb");
                seek_data();
            }

            size_t current_position = ftell(sound_file);
            if (current_position > data_length + data_block_offset + 8)
            {
                std::cout << "Done with audio data. Not playing the rest of the file." << std::endl;
                return 0;
            }
            int read_bytes = fread(buffer, sizeof(uint8_t), buffer_size, sound_file);
            return read_bytes;
        }

        friend std::ostream& operator<<(std::ostream& os, const WavFile2& wave_header)
        {
            os << "File Type Block ID : " << wave_header.file_Type_bloc_id() << std::endl;
            os << "File Size : " << wave_header.file_size() << std::endl;
            os << "File Format ID : " << wave_header.file_format_id() << std::endl;
            os << std::endl;
            os << "Fomat Bloc ID : " << wave_header.format_block_id() << std::endl;
            os << "Block Size : " << wave_header.bloc_size() << std::endl;
            os << "Audio Format : " << wave_header.audio_format() << std::endl;
            os << "Channels : " << wave_header.channels() << std::endl;
            os << "Frequency : " << wave_header.frequency() << std::endl;
            os << "Bytes Per Second : " << wave_header.byte_per_sec() << std::endl;
            os << "Bytes Per Block : " << wave_header.bytes_per_block() << std::endl;
            os << "Bits Per Sample : " << wave_header.bits_per_sample() << std::endl;
            os << std::endl;
            return os;
        }

        friend std::ostream& operator<<(std::ostream& os, const WavFile2* wave_header)
        {
            os << "File Type Block ID : " << wave_header->file_Type_bloc_id() << std::endl;
            os << "File Size : " << wave_header->file_size() << std::endl;
            os << "File Format ID : " << wave_header->file_format_id() << std::endl;
            os << std::endl;
            os << "Fomat Bloc ID : " << wave_header->format_block_id() << std::endl;
            os << "Block Size : " << wave_header->bloc_size() << std::endl;
            os << "Audio Format : " << wave_header->audio_format() << std::endl;
            os << "Channels : " << wave_header->channels() << std::endl;
            os << "Frequency : " << wave_header->frequency() << std::endl;
            os << "Bytes Per Second : " << wave_header->byte_per_sec() << std::endl;
            os << "Bytes Per Block : " << wave_header->bytes_per_block() << std::endl;
            os << "Bits Per Sample : " << wave_header->bits_per_sample() << std::endl;
            os << std::endl;
            return os;
        }
    private:
        void load_header()
        {
            if (!sound_file)
            {
                sound_file = fopen(file_path.c_str(), "rb");
            }

            if (!sound_file)
            {
                std::cerr << "Could not open file " << file_path << std::endl;
                // raise exception.
            }

            size_t read_bytes = fread(header, sizeof(header[0]), sizeof(header), sound_file);

            if (read_bytes != sizeof(header))
            {
                std::cerr << "Expected to read a header of " << sizeof(header) << " bytes but got " << read_bytes << std::endl;
                // raise exception.
            }

            if (sound_file)
            {
                //
                // Don't keep it open after reading the header.
                //
                fclose(sound_file);
                sound_file = nullptr;
            }
        }

        void load_data_tag()
        {
            sound_file = fopen(file_path.c_str(), "rb");
            data_block_offset = file_search(sound_file, "data");

            // Put the cursor on the data block size. The data_block_offset points
            // to the string "data". The data size is the next 4 bytes.
            fseek(sound_file, data_block_offset + 4, SEEK_SET);

            uint8_t data_buffer[4];
            size_t bytes_read = fread(data_buffer, sizeof(uint8_t), sizeof(data_buffer), sound_file);

            if (bytes_read < 4)
            {
                //std::cerr < "Only read " << bytes_read << " for the data length" << std::endl;
                throw std::length_error("Could not parse data size.");
            }

            data_length = *reinterpret_cast<uint32_t*>(data_buffer);

            fclose(sound_file);
            sound_file = nullptr;
        }

        void seek_data()
        {
            fseek(sound_file, data_block_offset + 8, SEEK_SET);
        }
    private:
        // Master riff chunk.
        //
        // 0x00 : FileTypeBlocID  (4 bytes) : Identifier « RIFF »  (0x52, 0x49, 0x46, 0x46)
        // 0x04 : FileSize        (4 bytes) : Overall file size minus 8 bytes
        // 0x08 : FileFormatID    (4 bytes) : Format = « WAVE »  (0x57, 0x41, 0x56, 0x45)

        // chunk describing data format;

        // 0x0c : FormatBlocID    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
        // 0x10 : BlocSize        (4 bytes) : Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
        // 0x14 : AudioFormat     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
        // 0x16 : NbrChannels     (2 bytes) : Number of channels
        // 0x18 : Frequency       (4 bytes) : Sample rate (in hertz)
        // 0x1c : BytePerSec      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
        // 0x20 : BytePerBloc     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
        // 0x22 : BitsPerSample   (2 bytes) : Number of bits per sample

        // Chunk containing sampled data.
        // 
        // 0x24 DataBlocID      (4 bytes) : Identifier « data »  (0x64, 0x61, 0x74, 0x61)
        // 0x28 DataSize        (4 bytes) : SampledData size
        unsigned char header[44];
        size_t data_block_offset = 0;
        size_t data_length = 0;
        size_t file_bytes_in_1s = 0;
    };

    /**
    Deprecated. First iteration of Wav files. Superseded by WavFile2.
    */
    class WavFile
    {
    public:
        std::string file_Type_bloc_id() const
        {
            return std::string(reinterpret_cast<const char*>(master_riff_chunk), 4);
        }

        size_t file_size() const
        {
            return *reinterpret_cast<const uint32_t*>(master_riff_chunk + 0x04);
        }

        std::string file_format_id() const
        {
            return std::string(reinterpret_cast<const char*>(master_riff_chunk) + 0x08, 4);
        }

        std::string format_block_id() const
        {
            return std::string(reinterpret_cast<const char*>(data_format), 4);
        }

        size_t bloc_size() const
        {
            return *reinterpret_cast<const uint32_t*>(data_format + 0x04);
        }

        size_t audio_format() const
        {
            return *reinterpret_cast<const uint16_t*>(data_format + 0x08);
        }

        size_t byte_per_sec() const
        {
            return *reinterpret_cast<const uint32_t*>(data_format + 0x10);
        }

        size_t bytes_per_block() const
        {
            return *reinterpret_cast<const uint16_t*>(data_format + 0x14);
        }

        size_t bits_per_sample() const
        {
            return *reinterpret_cast<const uint16_t*>(data_format + 0x16);
        }

        size_t channels() const
        {
            return *reinterpret_cast<const uint16_t*>(data_format + 0x0a);
        }

        size_t frequency() const
        {
            return *reinterpret_cast<const uint32_t*>(data_format + 0x0c);
        }

        std::string data_bloc_id() const
        {
            return std::string(reinterpret_cast<const char*>(sampled_data), 4);
        }

        size_t data_size() const
        {
            return *reinterpret_cast<const uint32_t*>(sampled_data + 0x04);
        }

        friend std::ostream& operator<<(std::ostream& os, const WavFile& wave_header)
        {
            os << "File Type Block ID : " << wave_header.file_Type_bloc_id() << std::endl;
            os << "File Size : " << wave_header.file_size() << std::endl;
            os << "File Format ID : " << wave_header.file_format_id() << std::endl;
            os << std::endl;
            os << "Fomat Bloc ID : " << wave_header.format_block_id() << std::endl;
            os << "Block Size : " << wave_header.bloc_size() << std::endl;
            os << "Audio Format : " << wave_header.audio_format() << std::endl;
            os << "Channels : " << wave_header.channels() << std::endl;
            os << "Frequency : " << wave_header.frequency() << std::endl;
            os << "Bytes Per Second : " << wave_header.byte_per_sec() << std::endl;
            os << "Bytes Per Block : " << wave_header.bytes_per_block() << std::endl;
            os << "Bits Per Sample : " << wave_header.bits_per_sample() << std::endl;
            os << std::endl;
            os << "Data Bloc ID : " << wave_header.data_bloc_id() << std::endl;
            os << "Sampled Data : " << wave_header.data_size() << std::endl;
            return os;
        }
    private:
        // Master riff chunk.
        //
        // 0x00 : FileTypeBlocID  (4 bytes) : Identifier « RIFF »  (0x52, 0x49, 0x46, 0x46)
        // 0x04 : FileSize        (4 bytes) : Overall file size minus 8 bytes
        // 0x08 : FileFormatID    (4 bytes) : Format = « WAVE »  (0x57, 0x41, 0x56, 0x45)
        unsigned char master_riff_chunk[12];

        // chunk describing data format;

        // 0x00 : FormatBlocID    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
        // 0x04 : BlocSize        (4 bytes) : Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
        // 0x08 : AudioFormat     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
        // 0x0a : NbrChannels     (2 bytes) : Number of channels
        // 0x0c : Frequency       (4 bytes) : Sample rate (in hertz)
        // 0x10 : BytePerSec      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
        // 0x14 : BytePerBloc     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
        // 0x16 : BitsPerSample   (2 bytes) : Number of bits per sample
        unsigned char data_format[24];
        
        // Chunk containing sampled data.
        // 
        // DataBlocID      (4 bytes) : Identifier « data »  (0x64, 0x61, 0x74, 0x61)
        // DataSize        (4 bytes) : SampledData size
        unsigned char sampled_data[8];
    };

    inline std::unique_ptr< SoundFile > open_sound_file(const std::filesystem::path& file_path)
    {
        if (file_path.extension() == ".wav")
        {
            return std::make_unique<WavFile2>(file_path);
        }

        throw std::exception();
    }

    //
    // The list of std::unique_ptrs is really annoying. I want to have strict lifetime,
    // but this is a little awkward. Will need to rethink this interface.
    //
    typedef std::list<std::unique_ptr<SoundFile>>::iterator QueuedSong;
}