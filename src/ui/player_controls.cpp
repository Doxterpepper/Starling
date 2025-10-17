
#include "player_controls.h"
#include <chrono>

namespace starling_ui
{
    using namespace std::chrono_literals;

    PlayerControls::PlayerControls(starling::PlaybackManager& playback_manager, QWidget *parent, Qt::WindowFlags f) :
        QWidget(parent, f),
        playback_manager(playback_manager)
    {
        timer_lock.lock();
        timer = std::thread(&PlayerControls::update_time, this);
        layout = new QGridLayout(this);
        previous_song_button = new QPushButton("prev", this);
        play_pause_button = new QPushButton("play", this);
        next_song_button = new QPushButton("next", this);
        tracking = new QSlider(Qt::Horizontal, this);
        time = new QLabel(current_time_string(), this);
        this->setLayout(layout);
        layout->addWidget(time, 0, 0);
        layout->addWidget(tracking, 1, 0, 1, -1);
        layout->addWidget(previous_song_button, 2, 0);
        layout->addWidget(play_pause_button, 2, 1);
        layout->addWidget(next_song_button, 2, 2);

        register_signals();
    }

    PlayerControls::~PlayerControls()
    {
        running = false;
        timer_lock.unlock();
        timer.join();
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

        set_playing();
    }

    void PlayerControls::next_song()
    {
        playback_manager.next_song();
    }

    void PlayerControls::prev_song()
    {
        playback_manager.previous_song();
    }

    void PlayerControls::set_playing()
    {
        if (playback_manager.state() == starling::PlaybackState::Playing)
        {
            play_pause_button->setText("pause");
            timer_lock.unlock();
        }
        else
        {
            play_pause_button->setText("play");
            bool mutex_available = timer_lock.try_lock();
            if (mutex_available)
            {
                mutex_available = timer_lock.try_lock();
            }
        }
    }

    QString PlayerControls::current_time_string() const
    {
        int hours = current_time / 60 / 60;
        int minutes = current_time / 60;
        int seconds = current_time % 60;

        std::string final_str = "";
        if (hours)
        {
            if (hours < 10)
            {
                final_str += "0";
            }
            std::string hours_str = std::to_string(hours);
            final_str += hours_str + ":";
        }

        std::string minutes_str = std::to_string(minutes);
        std::string seconds_str = std::to_string(seconds);

        if (minutes < 10)
        {
            final_str += "0";
        }
        final_str += minutes_str;
        final_str += ":";
        
        if (seconds < 10)
        {
            final_str += "0";
        }
        final_str += seconds_str;

        return QString::fromStdString(final_str);
    }

    void PlayerControls::update_time()
    {
        while(running)
        {
            {
                std::lock_guard time_callback(timer_lock);
            }
            std::this_thread::sleep_for(1s);
            auto currently_playing_song = playback_manager.currently_playing_song();
            if (currently_playing_song == nullptr)
            {
                current_time = 0;
            }
            else
            {
                current_time = playback_manager.currently_playing_song()->current_time();
            }

            time->setText(current_time_string());

        }
    }
}