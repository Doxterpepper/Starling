
#include <iostream>
#include <filesystem>
#include <string>
#include <list>
#include <fstream>

#include <chrono>

#include <alsa/asoundlib.h>


#include <pulse/simple.h>
#include <pulse/error.h>

#include <QListWidget>
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QListWidgetItem>
#include <QList>
#include <QString>
#include <QWidget>
#include <QApplication>
#include <QHBoxLayout>

#include "playback.h"

class wav
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

    friend std::ostream& operator<<(std::ostream& os, const wav& wave_header)
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

bool isMusicFile(const std::filesystem::path& file)
{
    const std::vector< std::string > extensions = {
        ".wave",
        ".wav",
        ".flac",
        ".mp3",
        ".m4a"
    };

    std::cout << file << " - ";
    for (const auto& extension : extensions)
    {
        if (file.extension() == extension)
        {
            return true;
        }
    }

    return false;
}

std::list< std::filesystem::path > searchMusic(const std::filesystem::path& musicDir)
{
    std::list< std::filesystem::path > songs;

    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(musicDir))
    {
        std::cout << entry << std::endl;
        if (entry.is_regular_file() && isMusicFile(entry))
        {
            songs.push_back(entry);
        }
    }

    return songs;
}

#define PCM_DEVICE "default"
 
#define BUFSIZE 1024

int main(int argc, char** argv)
{
    /*
    std::string currentLocalPath = std::getenv("HOME");
    
    {
        std::list< std::string > args;

        for (size_t arg_index = 1; arg_index < argc; arg_index++)
        {
            args.push_back(argv[arg_index]);
        }

        if (args.size() > 0)
        {
            currentLocalPath = *args.begin();
        }
    }

    std::cout << "Using directory - " << currentLocalPath << std::endl;
    QApplication app(argc, argv);

    QWidget* window = new QWidget();
    QHBoxLayout* windowLayout = new QHBoxLayout(window);
    window->setFixedSize(2000, 1000);
    window->setLayout(windowLayout);

    std::list< std::filesystem::path > song_files = searchMusic(currentLocalPath);
    std::cout << "Found " << song_files.size() << " songs." << std::endl;

    QListView* songView = new QListView(window);
    songView->move(10, 10);
    songView->resize(280, 180);

    QStringListModel* songListmodel = new QStringListModel(window);
    QStringList songList;

    for (const auto& song_path : song_files)
    {
        songList << QString::fromStdString(song_path.string());
    }

    songListmodel->setStringList(songList);
    songView->setModel(songListmodel);

    std::cout << "here" << std::endl;
    std::cout << songListmodel.rowCount() << std::endl;
    //windowLayout->addWidget(songView);

    window->show();
    return app.exec();
    */

    if (argc < 2)
    {
        std::cout << "Please specify an audio file." << std::endl;
        return 0;
    }

    std::vector< std::string > arguments(argc);

    for (int arg_index = 0; arg_index < argc; arg_index++)
    {
        arguments[arg_index] = std::string(argv[arg_index]);
    }


    auto start_turnaround_time = std::chrono::high_resolution_clock::now();
    auto end_turnaround_time = std::chrono::high_resolution_clock::now();
    for (size_t song_index = 1; song_index < argc; song_index++)
    {
        auto start_header_load = std::chrono::high_resolution_clock::now();
        FILE* song_file_stream = nullptr;
        song_file_stream = fopen(arguments[song_index].c_str(), "rb");

        if (!song_file_stream)
        {
            std::cerr << "Could not open file - " << arguments[1] << std::endl;
            return -1;
        }

        std::cout << "Playing file - " << arguments[1] << std::endl;
        wav wav_header;
        size_t read = fread(&wav_header, sizeof(unsigned char), sizeof(wav_header), song_file_stream);
        std::cout << wav_header << std::endl;
        fseek(song_file_stream, wav_header.data_size(), SEEK_CUR);
        auto stop_header_load = std::chrono::high_resolution_clock::now();

        {
            // Not totally necessary, but I don't want to keep this memory around. Just scope it to the cout line and
            // so it's released quickly.
            auto header_load_duration = duration_cast<std::chrono::microseconds>(stop_header_load - start_header_load);
            std::cout << "Loaded header in " << header_load_duration.count() << " microseconds." << std::endl;
        }

        auto playback = starling::SoundPlayer(arguments[0], "music", wav_header.channels(), wav_header.frequency());
        size_t read_bytes = 0;
        std::vector< uint8_t > sound_buffer(BUFSIZE);
        end_turnaround_time = std::chrono::high_resolution_clock::now();

        {
            // Not totally necessary, but I don't want to keep this memory around. Just scope it to the cout line and
            // so it's released quickly.
            auto turnaround_time_duration = duration_cast<std::chrono::microseconds>(end_turnaround_time - start_turnaround_time);
            std::cout << "turnaround in " << turnaround_time_duration.count() << " microseconds." << std::endl;
        }

        do
        {
            read_bytes = fread(sound_buffer.data(), sizeof(uint8_t), sound_buffer.size(), song_file_stream);

            if (read_bytes)
            {
                playback.play_buffer(sound_buffer, read_bytes);
            }
        } while(read_bytes);

        fclose(song_file_stream);

        playback.flush();
        start_turnaround_time = std::chrono::high_resolution_clock::now();
    }
}
