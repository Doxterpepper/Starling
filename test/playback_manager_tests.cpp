
#include <chrono>
#include <mutex>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <sound/music_queue.h>
#include <sound/playback_manager.h>

#include <mock/mock_engine.h>
#include <mock/mock_sound_file.h>

namespace starling::tests {
TEST_CASE("Initialization causes worker thread to block") {
    bool called = false;
    auto never_called_callback = [&](SoundFile *) { called = true; };

    MockEngine engine;
    engine.set_callback(std::move(never_called_callback));
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CHECK(called == false);
    CHECK(manager.state() == PlaybackState::Stopped);
}

TEST_CASE("Queue song for manager puts a song on the queue") {
    MockEngine engine;
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");
    auto mock_sound_file = std::make_unique<MockSoundFile>(file_path);
    auto sound_file_ptr = mock_sound_file.get();
    manager.queue(std::move(mock_sound_file));

    CHECK(queue.current_song() == sound_file_ptr);
}

TEST_CASE("play starts the worker thread and the engine is invoked.") {
    std::cout << "Play test" << std::endl;
    std::mutex condition_mutex;
    std::condition_variable wait_variable;
    auto play_song_and_notify = [&](SoundFile *) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        wait_variable.notify_all();
    };

    MockEngine engine;
    engine.set_callback(play_song_and_notify);
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");
    auto mock_sound_file = std::make_unique<MockSoundFile>(file_path);
    auto sound_file_ptr = mock_sound_file.get();
    manager.queue(std::move(mock_sound_file));
    manager.play();

    CHECK(manager.state() == PlaybackState::Playing);
    std::unique_lock<std::mutex> thread_lock(condition_mutex);
    wait_variable.wait(thread_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(manager.state() == PlaybackState::Stopped);
    CHECK(queue.current_song() == nullptr);
}

TEST_CASE("stop causes the worker thread to block. song file reset.") {
    std::cout << "Stop test" << std::endl;
    std::mutex condition_mutex;
    std::condition_variable wait_variable;
    auto play_song_and_notify = [&](SoundFile *) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        wait_variable.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    };

    MockEngine engine;
    engine.set_callback(play_song_and_notify);
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");
    auto mock_sound_file = std::make_unique<MockSoundFile>(file_path);
    auto sound_file_ptr = mock_sound_file.get();
    manager.queue(std::move(mock_sound_file));
    manager.play();

    CHECK(manager.state() == PlaybackState::Playing);
    std::unique_lock<std::mutex> thread_lock(condition_mutex);
    wait_variable.wait(thread_lock);
    manager.stop();
    CHECK(manager.state() == PlaybackState::Stopped);
    CHECK(queue.current_song() == sound_file_ptr);
    CHECK(sound_file_ptr->reset_called() == 1);
}

TEST_CASE("pause causes worker thread to block. play picks up where it left off.") {
    std::cout << "Pause test" << std::endl;
    std::mutex condition_mutex;
    std::condition_variable wait_variable;
    auto play_song_and_notify = [&](SoundFile *) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        wait_variable.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    };

    MockEngine engine;
    engine.set_callback(play_song_and_notify);
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");
    auto mock_sound_file = std::make_unique<MockSoundFile>(file_path);
    auto sound_file_ptr = mock_sound_file.get();
    manager.queue(std::move(mock_sound_file));
    manager.play();

    CHECK(manager.state() == PlaybackState::Playing);
    std::unique_lock<std::mutex> thread_lock(condition_mutex);
    wait_variable.wait(thread_lock);
    manager.pause();
    CHECK(manager.state() == PlaybackState::Paused);
    CHECK(queue.current_song() == sound_file_ptr);
    CHECK(sound_file_ptr->reset_called() == 0);
}

TEST_CASE("Finishing a song transitions to the next in the queue.") {
    //
    // Too flakey atm.
    //
    std::cout << "\n\tSong transition" << std::endl;
    std::mutex condition_mutex;
    std::condition_variable wait_variable;
    auto play_song_and_notify = [&](SoundFile *) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        wait_variable.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    };

    MockEngine engine;
    engine.set_callback(play_song_and_notify);
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");

    auto mock_sound_file_1 = std::make_unique<MockSoundFile>(file_path);
    auto mock_sound_file_ptr_1 = mock_sound_file_1.get();

    auto mock_sound_file_2 = std::make_unique<MockSoundFile>(file_path);
    auto mock_sound_file_ptr_2 = mock_sound_file_2.get();

    manager.queue(std::move(mock_sound_file_1));
    manager.queue(std::move(mock_sound_file_2));
    manager.play();

    CHECK(manager.state() == PlaybackState::Playing);
    CHECK(queue.current_song() == mock_sound_file_ptr_1);

    {
        std::unique_lock<std::mutex> thread_lock(condition_mutex);
        wait_variable.wait(thread_lock);
    }

    //
    // The notification comes from the engine itself, now wait for the
    // turnaround to song 2 and verify we're still
    // in the playing state.
    //
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CHECK(manager.state() == PlaybackState::Playing);
    CHECK(queue.current_song() == mock_sound_file_ptr_2);
    CHECK(mock_sound_file_ptr_1->reset_called() == 1);

    {
        std::unique_lock<std::mutex> thread_lock(condition_mutex);
        wait_variable.wait(thread_lock);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CHECK(manager.state() == PlaybackState::Stopped);
    CHECK(queue.current_song() == nullptr);
    CHECK(mock_sound_file_ptr_2->reset_called() == 1);
}

TEST_CASE("Play song, pause, then back a song should not deadlock") {
    std::cout << "\n******************************" << std::endl;
    std::cout << "Play song, pause, back a song." << std::endl;
    std::cout << "******************************" << std::endl;
    std::mutex condition_mutex;
    std::condition_variable wait_variable;
    auto play_song_and_notify = [&](SoundFile *) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wait_variable.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    MockEngine engine;
    engine.set_callback(play_song_and_notify);
    MusicQueue queue;
    PlaybackManager manager(&engine, &queue);
    std::filesystem::path file_path("not areal path");

    auto mock_sound_file_1 = std::make_unique<MockSoundFile>(file_path);
    auto mock_sound_file_ptr_1 = mock_sound_file_1.get();

    auto mock_sound_file_2 = std::make_unique<MockSoundFile>(file_path);
    auto mock_sound_file_ptr_2 = mock_sound_file_2.get();

    manager.queue(std::move(mock_sound_file_1));
    manager.queue(std::move(mock_sound_file_2));
    manager.play(mock_sound_file_ptr_2);
    CHECK(manager.currently_playing_song() == mock_sound_file_ptr_2);

    {
        std::unique_lock wait_play(condition_mutex);
        wait_variable.wait(wait_play);
    }

    manager.pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(6));
    manager.previous_song();

    CHECK(manager.currently_playing_song() == mock_sound_file_ptr_1);

    {
        std::unique_lock wait_play(condition_mutex);
        wait_variable.wait(wait_play);
    }
}
} // namespace starling::tests