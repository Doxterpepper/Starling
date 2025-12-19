
#pragma once
#include "player_cache.h"

namespace starling {
//
// Handles actually reading the sound buffer and writing to the player.
//
class PlaybackEngine {
public:
  PlaybackEngine(PlayerCache *cache);
  virtual void play_song(SoundFile *song);
  void stop();

private:
  PlayerCache *player_cache = nullptr;

  //
  // Use 128 so that we end up with a decent size buffer for both 16 bit and 24
  // bit audio that still fits the alignment for both.
  //
  static const size_t buffer_multiplier = 128;
  bool running = false;
};
} // namespace starling