
#include "player_controls.h"

namespace starling_ui
{
    PlayerControls::PlayerControls(starling::PlaybackManager& playback_manager, QWidget *parent, Qt::WindowFlags f) :
        QWidget(parent, f),
        playback_manager(playback_manager)
    {
        layout = new QHBoxLayout(this);
        previous_song_button = new QPushButton("prev", this);
        play_pause_button = new QPushButton("play", this);
        next_song_button = new QPushButton("next", this);
        this->setLayout(layout);
        layout->addWidget(previous_song_button);
        layout->addWidget(play_pause_button);
        layout->addWidget(next_song_button);

        register_signals();
    }

    void PlayerControls::register_signals()
    {
        QObject::connect(play_pause_button, &QAbstractButton::clicked, [&](){ play_pause(); });
        QObject::connect(previous_song_button, &QAbstractButton::clicked, [&]() { prev_song(); });
        QObject::connect(next_song_button, &QAbstractButton::clicked, [&]() { next_song(); });
    }

    void PlayerControls::play_pause()
    {
        if (playback_manager.state() == starling::PlaybackState::Playing)
        {
            playback_manager.pause();
        }
        else
        {
            playback_manager.play();
        }

        if (playback_manager.state() == starling::PlaybackState::Playing)
        {
            play_pause_button->setText("pause");
        }
        else
        {
            play_pause_button->setText("play");
        }
    }

    void PlayerControls::next_song()
    {
        playback_manager.next_song();
    }

    void PlayerControls::prev_song()
    {
        playback_manager.previous_song();
    }
}