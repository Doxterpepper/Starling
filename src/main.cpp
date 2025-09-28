
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
        ".wav"
    };

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
    //
    // This doesn't scale. It will need to be cached eventually if the user has an absurd number of songs or searches
    // their whole file system for songs.
    // That's not even counting the fact that we'll want to load metadata for each song.
    //
    auto start_search_music = std::chrono::high_resolution_clock::now();
    std::list< std::filesystem::path > songs;

    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(musicDir))
    {
        if (entry.is_regular_file() && isMusicFile(entry))
        {
            songs.push_back(entry);
        }
    }

    auto end_search_music = std::chrono::high_resolution_clock::now();
    auto search_music_duration = duration_cast<std::chrono::microseconds>(end_search_music - start_search_music);
    std::cout << "Found " << songs.size() << " songs in " << search_music_duration.count() << "μs" << std::endl;
    return songs;
}

#define PCM_DEVICE "default"
 
#define BUFSIZE 1024

int main(int argc, char** argv)
{
    auto start_app_time = std::chrono::high_resolution_clock::now();
    //std::string currentLocalPath = std::getenv("HOME");

    std::list<std::filesystem::path> file_list;
    for (int arg_index = 1; arg_index < argc; arg_index++)
    {
        file_list.push_back(std::filesystem::path(argv[arg_index]));
    }

    QApplication app(argc, argv);
    starling::PlaybackManager player;

    QWidget* window = new QWidget();
    QHBoxLayout* windowLayout = new QHBoxLayout(window);
    window->setFixedSize(400, 400);
    window->setLayout(windowLayout);

    QListWidget songListWidget;

    for (const std::filesystem::path& path : file_list)
    {
        songListWidget.addItem(QString::fromStdString(path));
    }
    
    QObject::connect(&songListWidget, &QListWidget::itemDoubleClicked, [&](QListWidgetItem* song)
    {
        std::cout << "clicked " << song->text().toStdString() << std::endl;
        std::filesystem::path song_path(song->text().toStdString());
        auto sound_file = starling::open_sound_file(song_path);
        player.queue(std::move(sound_file));

        if (player.state() != starling::PlaybackState::Playing)
        {
            player.play();
        }
    });

    windowLayout->addWidget(&songListWidget);

    /*
    QObject::connect(songView, &QListView::clicked, [](const QModelIndex& model_index) {
        std::cout << "Clicked item " << model_index.column() << model_index.row();
        std::cout << " "<< model_index.model() << std::endl;
    });
    */

    auto show_window_time = std::chrono::high_resolution_clock::now();
    window->show();
    auto app_startup_time = duration_cast<std::chrono::microseconds>(show_window_time - start_app_time);
    std::cout << "Application started in " << app_startup_time.count() << "μs." << std::endl;

    return app.exec();

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
