
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

#include "sound/playback.h"
#include "sound/sound_file.h"

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
    for (int song_index = 1; song_index < argc; song_index++)
    {
        auto start_header_load = std::chrono::high_resolution_clock::now();
        
        /*
        FILE* song_file_stream = nullptr;
        song_file_stream = fopen(arguments[song_index].c_str(), "rb");

        if (!song_file_stream)
        {
            std::cerr << "Could not open file - " << arguments[1] << std::endl;
            return -1;
        }

        std::cout << "Playing file - " << arguments[1] << std::endl;
        starling::WavFile wav_header;
        size_t read = fread(&wav_header, sizeof(unsigned char), sizeof(wav_header), song_file_stream);
        //std::cout << wav_header << std::endl;
        fseek(song_file_stream, wav_header.data_size(), SEEK_CUR);

        */
        std::unique_ptr<starling::WavFile2> sound_file = starling::open_sound_file(std::filesystem::path(arguments[song_index]));
        std::cout << sound_file.get() << std::endl;

        auto stop_header_load = std::chrono::high_resolution_clock::now();
        {
            // Not totally necessary, but I don't want to keep this memory around. Just scope it to the cout line and
            // so it's released quickly.
            auto header_load_duration = duration_cast<std::chrono::microseconds>(stop_header_load - start_header_load);
            std::cout << "Loaded header in " << header_load_duration.count() << " microseconds." << std::endl;
        }

        auto playback = starling::SoundPlayer(arguments[0], "music", sound_file->channels(), sound_file->frequency());
        size_t read_bytes = 0;
        std::vector< uint8_t > sound_buffer(BUFSIZE);
        end_turnaround_time = std::chrono::high_resolution_clock::now();

        {
            // Not totally necessary, but I don't want to keep this memory around. Just scope it to the cout line and
            // so it's released quickly.
            //
            // So far coming in around 24 milliseconds on my machine. This doesn't appear audible, but one of the major goals of
            // this project is continuous playback. Keep an eye on this.
            auto turnaround_time_duration = duration_cast<std::chrono::microseconds>(end_turnaround_time - start_turnaround_time);
            std::cout << "turnaround in " << turnaround_time_duration.count() << " microseconds." << std::endl;
        }

        do
        {
            //read_bytes = fread(sound_buffer.data(), sizeof(uint8_t), sound_buffer.size(), song_file_stream);
            read_bytes = sound_file->read_sound_chunk(sound_buffer.data(), sound_buffer.size());
            if (read_bytes)
            {
                playback.play_buffer(sound_buffer, read_bytes);
            }
        } while(read_bytes);

        //fclose(song_file_stream);

        playback.flush();
        start_turnaround_time = std::chrono::high_resolution_clock::now();
    }
}
