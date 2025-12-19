
#include <catch2/catch_test_macros.hpp>

#include <sound/music_queue.h>
#include <sound/sound_file.h>

#include "../mock/mock_sound_file.h"

TEST_CASE("No queued songs returns nullptr") {
  starling::MusicQueue queue;
  CHECK(queue.current_song() == nullptr);
  CHECK(queue.size() == 0);
}

TEST_CASE("Add song to queue") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);
}

TEST_CASE("Add multiple songs") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);
}

TEST_CASE("Set current song.") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  auto *new_current = mockSong2.get();
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);

  CHECK(queue.set_current_song(new_current) == true);

  CHECK(queue.current_song() == new_current);
}

TEST_CASE("Set current song invalid song.") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  auto *new_current = mockSong2.get();
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);

  CHECK(queue.set_current_song(nullptr) == false);

  CHECK(queue.current_song() == current);
}

TEST_CASE("Increment song to next song - valid") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  auto *new_current = mockSong2.get();
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);

  queue.next();

  CHECK(queue.current_song() == new_current);
}

TEST_CASE("Increment song at end - valid") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  queue.next();

  CHECK(queue.current_song() == nullptr);
}

TEST_CASE("Decrement song - valid") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  auto *new_current = mockSong2.get();
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);

  CHECK(queue.set_current_song(new_current) == true);

  CHECK(queue.current_song() == new_current);

  queue.previous();

  CHECK(queue.current_song() == current);
}

TEST_CASE("Decrement to begin - valid") {
  std::filesystem::path path("Not a real path");
  auto mockSong = std::make_unique<MockSoundFile>(path);

  starling::MusicQueue queue;

  MockSoundFile *current = mockSong.get();

  CHECK(queue.current_song() == nullptr);
  queue.add_song(std::move(mockSong));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 1);

  auto mockSong2 = std::make_unique<MockSoundFile>(path);
  auto *new_current = mockSong2.get();
  queue.add_song(std::move(mockSong2));

  CHECK(current == queue.current_song());
  CHECK(queue.size() == 2);

  CHECK(queue.set_current_song(new_current) == true);

  CHECK(queue.current_song() == new_current);

  queue.previous();

  CHECK(queue.current_song() == current);

  queue.previous();

  CHECK(queue.current_song() == current);
}