
#pragma once

// #include <thread>
#include <mutex>

#include "sound_file.h"

namespace starling {
/////
// Music queue requirements.
// 1. Single ownership of the SongFiles.
// 2. Track the current song and allow incrementing and decrementing
//    to the next/previous songs.
// 3. Random access for playing.
/////
class MusicQueue {
public:
  typedef std::list<SoundFile *>::iterator QueuedSong;

  MusicQueue() { CurrentSong = ViewerList.begin(); }

  ~MusicQueue() = default;

  MusicQueue(MusicQueue const &) = delete;
  MusicQueue(MusicQueue &&) = default;

  MusicQueue &operator=(MusicQueue &&) = default;
  MusicQueue &operator=(MusicQueue const &) = delete;

  int size() const { return ViewerList.size(); }

  SoundFile *current_song() {
    std::lock_guard<std::mutex> guard(QueueLock);
    if (CurrentSong == ViewerList.end()) {
      return nullptr;
    }

    return *CurrentSong;
  }

  //
  // Next could iterate to end, resulting in a nullptr from current_song. This
  // kinda makes sense, if you increment the songs, eventually you run out of
  // songs and playback stops.
  //
  void next() {
    std::lock_guard<std::mutex> guard(QueueLock);
    if (CurrentSong != ViewerList.end()) {
      ++CurrentSong;
    }
  }

  //
  // Can't iterate to a previous songs that is invalid. Stops at the first song.
  //
  void previous() {
    std::lock_guard<std::mutex> guard(QueueLock);
    if (CurrentSong != ViewerList.begin()) {
      --CurrentSong;
    }
  }

  void add_song(std::unique_ptr<SoundFile> &&song) {
    std::lock_guard<std::mutex> guard(QueueLock);
    ViewerList.push_back(song.get());
    OwningList.push_back(std::move(song));
    CurrentSong = ViewerList.begin();
  }

  bool set_current_song(SoundFile const *song) {
    for (std::list<SoundFile *>::iterator songElement = ViewerList.begin();
         songElement != ViewerList.end(); songElement++) {
      if (*songElement == song) {
        CurrentSong = songElement;
        return true;
      }
    }
    return false;
  }

private:
  std::list<std::unique_ptr<SoundFile>> OwningList;
  std::list<SoundFile *> ViewerList;
  QueuedSong CurrentSong;
  std::mutex QueueLock;
};
} // namespace starling