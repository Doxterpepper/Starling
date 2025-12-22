
#pragma once

#include <sound/player_cache.h>

namespace starling::test {
class MockCache : public PlayerCache {
  public:
    SoundPlayer *get_player(const SoundFile *soundfile) override {
        call_count++;
        return local_player;
    }

    void set_player(SoundPlayer *player) { local_player = player; }

    int called() const { return call_count; }

  private:
    SoundPlayer *local_player = nullptr;

    int call_count = 0;
};
} // namespace starling::test