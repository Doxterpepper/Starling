
#pragma once

#include <iostream>
#include <string>
namespace starling
{
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

    
}