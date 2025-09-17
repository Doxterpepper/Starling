
#pragma once

namespace musicman
{
    class SoundFile
    {
    public:
        static SoundFile read_file(const std::string& path);
    };

    class WavFile
    {
public:
    size_t audio_format() const
    {
        size_t format_value = static_cast<size_t>(data_format[10]) | static_cast<size_t>(data_format[11] << 8);
        return format_value;
    }

    size_t file_size() const
    {
        return (static_cast<size_t>(master_riff_chunk[4])
            | static_cast<size_t>(master_riff_chunk[5] << 8)
            | static_cast<size_t>(master_riff_chunk[6] << 16)
            | static_cast<size_t>(master_riff_chunk[7] << 24))
            + 8; // The header keeps the size as (size - 8 bytes). So Add the 8 bytes here.
    }

    std::string fileTypeBlocId() const
    {
        char blockId[4] = { (char)master_riff_chunk[0x00], (char)master_riff_chunk[0x01], (char)master_riff_chunk[0x02], (char)master_riff_chunk[0x03] };

        return std::string(blockId);
    }

    std::string file_format_id() const
    {
        char formatId[4] = { (char)master_riff_chunk[0x08], (char)master_riff_chunk[0x09], (char)master_riff_chunk[0x0a], (char)master_riff_chunk[0x0c] };
        return std::string(formatId);
    }

    size_t bytes_per_block() const
    {
        return static_cast<size_t>(data_format[0x12]) | (static_cast<size_t>(data_format[0x13]) << 8);
    }

    size_t bitsPerSample() const
    {
        return static_cast<size_t>(data_format[0x14]) | (static_cast<size_t>(data_format[0x15]) << 8);
    }

    size_t channels() const
    {
        return static_cast<size_t>(data_format[0x08]) | (static_cast<size_t>(data_format[0x09]) << 8);
    }
private:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Don't add any extra members here. This is intended to be deserialized from a file and positioning will change things. //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Master riff chunk.
    //
    // 0x00 : FileTypeBlocID  (4 bytes) : Identifier « RIFF »  (0x52, 0x49, 0x46, 0x46)
    // 0x04 : FileSize        (4 bytes) : Overall file size minus 8 bytes
    // 0x08 : FileFormatID    (4 bytes) : Format = « WAVE »  (0x57, 0x41, 0x56, 0x45)
    unsigned char master_riff_chunk[12];

    // chunk describing data format;

    // 0x00 : FormatBlocID    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
    // 0x04 : BlocSize        (4 bytes) : Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
    // 0x06 : AudioFormat     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
    // 0x08 : NbrChannels     (2 bytes) : Number of channels
    // 0x0a : Frequency       (4 bytes) : Sample rate (in hertz)
    // 0x0e : BytePerSec      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
    // 0x12 : BytePerBloc     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
    // 0x14 : BitsPerSample   (2 bytes) : Number of bits per sample
    unsigned char data_format[24];
    
    // Chunk containing sampled data.
    // 
    // DataBlocID      (4 bytes) : Identifier « data »  (0x64, 0x61, 0x74, 0x61)
    // DataSize        (4 bytes) : SampledData size
    unsigned char sampled_data[8];
    };
}