
#pragma once

#include <QListWidget>
#include <QListWidgetItem>
#include "sound/sound_file.h"

namespace starling_ui
{
    class FileEntry : public QListWidgetItem
    {
    public:
        FileEntry(const starling::SoundFile* sound_file, QListWidget *parent = nullptr, int type = Type);

        const starling::SoundFile* playback_file() const;
        
    private:
        const starling::SoundFile* sound_file;
    };
}