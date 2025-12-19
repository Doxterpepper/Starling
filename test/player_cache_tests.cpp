
#include <catch2/catch_test_macros.hpp>

#define __TSET_DEF__
#include <sound/player_cache.h>

#include "mock/mock_sound_file.h"

TEST_CASE("Get cache entry gives value.") {
  starling::PlayerCache cache;

  std::filesystem::path sample_path("not a path");
  MockSoundFile testSoundFile(sample_path);

  auto player = cache.get_player(&testSoundFile);
  CHECK(player != nullptr);
}

TEST_CASE("Calling same settings return same player") {

  starling::PlayerCache cache;

  std::filesystem::path sample_path("not a path");
  MockSoundFile testSoundFile(sample_path);
  testSoundFile.set_bps(264600);
  testSoundFile.set_frequency(44100);
  testSoundFile.set_chan(2);

  auto player1 = cache.get_player(&testSoundFile);
  CHECK(player1 != nullptr);

  MockSoundFile testSoundFile2(sample_path);
  testSoundFile2.set_bps(264600);
  testSoundFile2.set_frequency(44100);
  testSoundFile2.set_chan(2);

  auto player2 = cache.get_player(&testSoundFile2);
  CHECK(player1 == player2);
}

TEST_CASE("Calling with different channel gives new player.") {

  starling::PlayerCache cache;

  std::filesystem::path sample_path("not a path");
  MockSoundFile testSoundFile(sample_path);
  testSoundFile.set_bps(264600);
  testSoundFile.set_frequency(44100);
  testSoundFile.set_chan(2);

  auto player1 = cache.get_player(&testSoundFile);
  CHECK(player1 != nullptr);

  MockSoundFile testSoundFile2(sample_path);
  testSoundFile2.set_bps(264600);
  testSoundFile2.set_frequency(44100);
  testSoundFile2.set_chan(1);

  auto player2 = cache.get_player(&testSoundFile2);
  CHECK(player2 != nullptr);
  CHECK(player1 != player2);
}

TEST_CASE("Calling with different frequency gives new player.") {

  starling::PlayerCache cache;

  std::filesystem::path sample_path("not a path");
  MockSoundFile testSoundFile(sample_path);
  testSoundFile.set_bps(264600);
  testSoundFile.set_frequency(44100);
  testSoundFile.set_chan(2);

  auto player1 = cache.get_player(&testSoundFile);
  CHECK(player1 != nullptr);

  MockSoundFile testSoundFile2(sample_path);
  testSoundFile2.set_bps(264600);
  testSoundFile2.set_frequency(512);
  testSoundFile2.set_chan(2);

  auto player2 = cache.get_player(&testSoundFile2);
  CHECK(player2 != nullptr);
  CHECK(player1 != player2);
}

TEST_CASE("Calling with different bits p/s gives new player.") {

  starling::PlayerCache cache;

  std::filesystem::path sample_path("not a path");
  MockSoundFile testSoundFile(sample_path);
  testSoundFile.set_bps(264600);
  testSoundFile.set_frequency(44100);
  testSoundFile.set_chan(2);

  auto player1 = cache.get_player(&testSoundFile);
  CHECK(player1 != nullptr);

  MockSoundFile testSoundFile2(sample_path);
  testSoundFile2.set_bps(264601);
  testSoundFile2.set_frequency(44100);
  testSoundFile2.set_chan(2);

  auto player2 = cache.get_player(&testSoundFile2);
  CHECK(player2 != nullptr);
  CHECK(player1 != player2);
}