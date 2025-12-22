/**
 * Test definitions for player. Adds some test functions for inspecting the
 * state of the player. Should be platform independent. Each platform may have a
 * different way of handling sound, so this doesn't link any of those libraries.
 */

#ifdef __TEST_DEF__

#include <sound/playback.h>
namespace starling {
SoundPlayer::SoundPlayer(const std::string &, const std::string &, size_t, size_t, size_t) {}

SoundPlayer::~SoundPlayer() {}

SoundPlayer::SoundPlayer(SoundPlayer &&) = default;

SoundPlayer &SoundPlayer::operator=(SoundPlayer &&) = default;

void SoundPlayer::flush() {}

int SoundPlayer::called() const { return call_count; }
} // namespace starling
#endif