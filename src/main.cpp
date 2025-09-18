
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
            + 8;
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

    auto playback = starling::SoundPlayer(arguments[0], "music", 2, 44100);

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
        //std::cout << "Read header of size - " << read << std::endl;
        //std::cout << "Audio format - " << wav_header.audio_format() << std::endl;
        //std::cout << "File size - " << wav_header.file_size() << std::endl;
        //std::cout << "Channels - " << wav_header.channels() << std::endl;
        //std::cout << "BLoc ID - " << wav_header.fileTypeBlocId() << std::endl;
        //std::cout << "FileFormatID - " << wav_header.file_format_id() << std::endl;
        //std::cout << "Bytes per block - " << wav_header.bytes_per_block() << std::endl;
        //std::cout << "Bits Per SAmple - " << wav_header.bitsPerSample() << std::endl;
        auto stop_header_load = std::chrono::high_resolution_clock::now();

        {
            // Not totally necessary, but I don't want to keep this memory around. Just scope it to the cout line and
            // so it's released quickly.
            auto header_load_duration = duration_cast<std::chrono::microseconds>(stop_header_load - start_header_load);
            std::cout << "Loaded header in " << header_load_duration.count() << " microseconds." << std::endl;
        }

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
