
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

    //
    // The two longest operations are loading the file and creating a SoundPlayer. The exact reason for each one taking so long
    // is not determined yet. Intuitively creating the file object will take time because it depends on disk reads. The creation of 
    // the SoundPlayer requires setting up pulse audio. This very well could be an expensive operation.
    //
    // The theory now is that we load each file off the hot path and then only re-create the playback object if we need to. that is
    // only if the settings change between files. Maybe there's a way to update the settings in our pulse setup without having to
    // create the whole thing again.
    //
    // The best timing I have is for the creation of a SoundPlayer. This takes on the order of 24.8 ms on my machine. The bulk of the
    // turnaround time was from this operation.
    //
    // Loading a file into memory takes around 20-50μs. This is fast, but without it it can take around 1-2μs to switch to the next song.
    // There is no guarantee it will always take this long. For one test file I saw load times on the order of 20ms.
    //
    std::list< std::unique_ptr< starling::WavFile2 > > songs;
    for (int song_index = 1; song_index < argc; song_index++)
    {
        auto file_path = std::filesystem::path(arguments[song_index]);
        auto start_load_file = std::chrono::high_resolution_clock::now();
        auto sound_file = starling::open_sound_file(file_path);
        auto end_load_file = std::chrono::high_resolution_clock::now();
        auto load_file_duration = duration_cast<std::chrono::microseconds>(end_load_file - start_load_file);
        std::cout << "Load file into memory in " << load_file_duration.count() << " microseconds" << std::endl;
        songs.push_back(std::move(starling::open_sound_file(std::filesystem::path(arguments[song_index]))));
    }

    auto last_frequency = songs.front()->frequency();
    auto last_channels = songs.front()->channels();
    auto last_bits_per_sample = songs.front()->bits_per_sample();
    auto playback = starling::SoundPlayer(arguments[0], "music", last_channels, last_frequency, last_bits_per_sample);

    auto start_turnaround_time = std::chrono::high_resolution_clock::now();
    auto end_turnaround_time = std::chrono::high_resolution_clock::now();
    for (auto& sound_file : songs)
    {
        if (last_frequency != sound_file->frequency() || last_channels != sound_file->channels() || last_bits_per_sample != sound_file->bits_per_sample())
        {
            auto create_playback_start = std::chrono::high_resolution_clock::now();
            playback = starling::SoundPlayer(arguments[0], "music", sound_file->channels(), sound_file->frequency(), sound_file->bits_per_sample());
            auto create_playback_stop = std::chrono::high_resolution_clock::now();
            auto playback_create_duration = duration_cast<std::chrono::microseconds>(create_playback_stop - create_playback_start);
            std::cout << "Create playback object in " << playback_create_duration.count() << " microseconds." << std::endl;
            last_frequency = sound_file->frequency();
            last_channels = sound_file->channels();
            last_bits_per_sample = sound_file->bits_per_sample();
        }

        size_t read_bytes = 0;
        std::vector< uint8_t > sound_buffer(sound_file->bytes_per_block() * 128);
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
            read_bytes = sound_file->read_sound_chunk(sound_buffer.data(), sound_buffer.size());
            if (read_bytes)
            {
                playback.play_buffer(sound_buffer, read_bytes);
            }

        } while(read_bytes);

        playback.flush();
        start_turnaround_time = std::chrono::high_resolution_clock::now();
        //usleep(5000);// Testing the perceptible gap between songs.
    }
}
