#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>

#include "../sound/playback_manager.h"

namespace starling_ui
{
    class PlayerControls : public QWidget
    {
    public:
        PlayerControls(starling::PlaybackManager& playback_manager, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    
    private:
        void register_signals();

        void play_pause();

        void next_song();

        void prev_song();
    private:
        QHBoxLayout* layout;
        QPushButton* previous_song_button;
        QPushButton* play_pause_button;
        QPushButton* next_song_button;
        starling::PlaybackManager& playback_manager;
    };
}