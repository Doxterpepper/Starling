
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
#include "sound/playback_manager.h"

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
    starling::PlaybackManager playback_manager;
    for (int song_index = 1; song_index < argc; song_index++)
    {
        auto file_path = std::filesystem::path(arguments[song_index]);
        auto start_load_file = std::chrono::high_resolution_clock::now();
        auto sound_file = starling::open_sound_file(file_path);
        auto end_load_file = std::chrono::high_resolution_clock::now();
        auto load_file_duration = duration_cast<std::chrono::microseconds>(end_load_file - start_load_file);
        std::cout << "Load file into memory in " << load_file_duration.count() << " microseconds" << std::endl;
        playback_manager.queue(std::move(sound_file));
    }

    playback_manager.play();
}
