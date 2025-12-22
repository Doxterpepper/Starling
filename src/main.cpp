
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include <chrono>

#include <pulse/error.h>
#include <pulse/simple.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QList>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>

#include "sound/music_queue.h"
#include "sound/playback.h"
#include "sound/playback_manager.h"
#include "sound/player_cache.h"
#include "sound/sound_file.h"
#include <sound/playback_engine.h>

#include "file_entry.h"
#include "ui/player_controls.h"

bool isMusicFile(const std::filesystem::path &file) {
    const std::vector<std::string> extensions = {".wave", ".wav"};

    for (const auto &extension : extensions) {
        if (file.extension() == extension) {
            return true;
        }
    }

    return false;
}

int start_gui(int argc, char **argv, starling::PlaybackManager &player, const std::list<const starling::SoundFile *> &file_list) {
    QApplication app(argc, argv);

    QWidget *window = new QWidget();
    QVBoxLayout *windowLayout = new QVBoxLayout(window);
    window->setFixedSize(400, 400);
    window->setLayout(windowLayout);

    QListWidget songListWidget;

    for (auto &song_file : file_list) {
        songListWidget.addItem(new starling_ui::FileEntry(song_file));
    }

    starling_ui::PlayerControls controls(player);

    QObject::connect(&songListWidget, &QListWidget::itemDoubleClicked, [&](QListWidgetItem *song) {
        starling_ui::FileEntry *file_entry = static_cast<starling_ui::FileEntry *>(song);
        player.play(file_entry->playback_file());
        controls.set_playing();
    });

    windowLayout->addWidget(&songListWidget);

    windowLayout->addWidget(&controls);

    // auto show_window_time = std::chrono::high_resolution_clock::now();
    window->show();

    return app.exec();
}

std::list<std::filesystem::path> searchMusic(const std::filesystem::path &musicDir) {
    //
    // This doesn't scale. It will need to be cached eventually if the user has
    // an absurd number of songs or searches their whole file system for songs.
    // That's not even counting the fact that we'll want to load metadata for
    // each song.
    //
    auto start_search_music = std::chrono::high_resolution_clock::now();
    std::list<std::filesystem::path> songs;

    for (const std::filesystem::directory_entry &entry : std::filesystem::recursive_directory_iterator(musicDir)) {
        if (entry.is_regular_file() && isMusicFile(entry)) {
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

int main(int argc, char **argv) {
#ifdef __DEBUG__
    std::cout << "Debug build." << std::endl;
#else
    std::cout << "Release build." << std::endl;
#endif
    std::cout << std::endl;
    // auto start_app_time = std::chrono::high_resolution_clock::now();

    std::list<std::filesystem::path> file_list;
    bool start_gui_app = true;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-cli") == 0) {
            start_gui_app = false;
        } else {
            file_list.push_back(std::filesystem::path(argv[i]));
            std::cout << argv[i] << " ";
        }
    }

    std::cout << std::endl;

    //
    // TODO: How can I speedup startup. It's a minor thing right now, but I'd
    // like to ensure that Starling starts in a reasonable amount of time even
    // for large numbers of files. Right now I'm not worried, but I think it
    // won't scale for larger numbers of files.
    //
    //
    // Still thinking about putting all this in a facade that just passes calls
    // through and has no real logic. It would just setup the playback manager
    // for the application while the backend can be mocked for testing.
    //
    starling::PlayerCache cache;
    starling::MusicQueue queue;
    starling::PlaybackEngine engine(&cache);
    starling::PlaybackManager player(&engine, &queue);

    std::list<const starling::SoundFile *> songs;
    for (const std::filesystem::path &path : file_list) {

        auto song_file = player.queue(path);
        songs.push_back(song_file);
    }

    if (start_gui_app) {
        // auto app_startup_time = duration_cast<std::chrono::microseconds>(show_window_time - start_app_time);
        // std::cout << "Application started in " << app_startup_time.count() << "μs." << std::endl;
        return start_gui(argc, argv, player, songs);
    }

    player.play_sync();
}
