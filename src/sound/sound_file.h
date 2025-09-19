
#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdio>
#include <filesystem>

namespace starling
{
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

        virtual size_t read_sound_chunk(uint8_t* buffer, size_t buffer_size) = 0;

        //virtual friend std::ostream& operator<<(std::ostream& os, const SoundFile& wave_header) = 0;

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

        size_t bytes_per_block() const
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x1e);
        }

        size_t bits_per_sample() const
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x20);
        }

        size_t channels() const override
        {
            return *reinterpret_cast<const uint16_t*>(header + 0x16);
        }

        size_t frequency() const override
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x18);
        }

        std::string data_bloc_id() const
        {
            return std::string(reinterpret_cast<const char*>(header + 0x24), 4);
        }

        size_t data_size() const
        {
            return *reinterpret_cast<const uint32_t*>(header + 0x28);
        }

        size_t read_sound_chunk(uint8_t* buffer, size_t buffer_size) override
        {
            if (!sound_file)
            {
                sound_file = fopen(file_path.c_str(), "rb");
                seek_data();
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
            os << "Data Bloc ID : " << wave_header.data_bloc_id() << std::endl;
            os << "Sampled Data : " << wave_header.data_size() << std::endl;
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
            os << "Data Bloc ID : " << wave_header->data_bloc_id() << std::endl;
            os << "Sampled Data : " << wave_header->data_size() << std::endl;
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

            if (read_bytes != sizeof(header[0]))
            {
                std::cerr << "Expected to read a header of " << sizeof(header[0]) << " bytes but got " << read_bytes << std::endl;
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

        void seek_data()
        {
            if (data_bloc_id() == "LIST")
            {
                fseek(sound_file, sizeof(header) + data_size(), SEEK_SET);
            }

            std::vector<char> search_buffer(0xFF); // Arbitrary length of data to search.

            std::string search_tag = "data";
            size_t tag_index = 0;
            size_t read_bytes = 0;
            do
            {
                //
                // Read chunks of data and see if they contain the "data" key word. Search can span multiple buffers.
                //
                read_bytes = fread(search_buffer.data(), sizeof(search_buffer[0]), search_buffer.size(), sound_file);
                std::cout << "Read " << read_bytes << " while searching" << std::endl;

                for (size_t buffer_element = 0; buffer_element < read_bytes; buffer_element++)
                {
                    // Go through the buffer read and look for each element of search_tag. This should ammount to the
                    // key word "data". If we find the 'd' character, increment the tag_index to then search for 'a'.
                    // do this until we've exhausted the letters in search_tag.
                    // Once we've found the search tag, seek to the position right after the tag. This is where the
                    // sound data begins.
                    if (search_buffer[buffer_element] == search_tag[tag_index])
                    {
                        std::cout << search_buffer[buffer_element] << std::endl;
                        ++tag_index;
                        if (tag_index >= search_tag.length())
                        {
                            //
                            // Seek to the beginning of the sound data. we've found the data tag and the current
                            // buffer_element represents the final 'a' in the tag.
                            // The current file position is at the end of the last read buffer. So we need to move it
                            // back to the start of the data.
                            long seek_difference = read_bytes - buffer_element - 1;
                            std::cout << "Found data tag. going back " << seek_difference << " bytes" << std::endl;;
                            fseek(sound_file, -seek_difference, SEEK_CUR); // Go back this distance.
                            fseek(sound_file, 0, SEEK_CUR);
                            size_t pos = ftell(sound_file);
                            std::cout << "Currently at position - " << pos << std::endl;
                            return;
                        }
                    }
                    else
                    {
                        tag_index = 0;
                    }
                }
            }while(read_bytes);

            std::cerr << "Could not find data tag."<< std::endl;
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
    };

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

    std::unique_ptr< WavFile2 > open_sound_file(const std::filesystem::path& file_path)
    {
        if (file_path.extension() == ".wav")
        {
            return std::make_unique<WavFile2>(file_path);
        }

        throw std::exception();
    }
}