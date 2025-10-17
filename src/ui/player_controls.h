#pragma once

#include <thread>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QLabel>

#include "../sound/playback_manager.h"

namespace starling_ui
{
    class PlayerControls : public QWidget
    {
    public:
        PlayerControls(starling::PlaybackManager& playback_manager, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~PlayerControls();

        void set_playing();
    
    private:
        void register_signals();

        void play_pause();

        void next_song();

        void prev_song();

        QString current_time_string() const;

        void update_time();
    private:
        QGridLayout* layout;
        QPushButton* previous_song_button;
        QPushButton* play_pause_button;
        QPushButton* next_song_button;
        QSlider* tracking;
        QLabel* time;
        starling::PlaybackManager& playback_manager;

        int current_time = 0;

        std::thread timer;
        std::mutex timer_lock;
        bool running = true;
    };
}