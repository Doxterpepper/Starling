
#include "file_entry.h"

namespace starling_ui {
FileEntry::FileEntry(const starling::SoundFile *sound_file, QListWidget *parent,
                     int type)
    : QListWidgetItem(QString::fromStdString(sound_file->name()), parent, type),
      sound_file(sound_file) {}

const starling::SoundFile *FileEntry::playback_file() const {
    return sound_file;
}
} // namespace starling_ui