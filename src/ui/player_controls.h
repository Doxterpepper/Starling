#pragma once

#include <mutex>
#include <thread>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QVBoxLayout>
#include <QWidget>

#include "../sound/playback_manager.h"

namespace starling_ui {
class PlayerControls : public QWidget {
  public:
    PlayerControls(starling::PlaybackManager &playback_manager, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~PlayerControls();

    void set_playing();

  private:
    void register_signals();

    void play_pause();

    void next_song();

    void prev_song();

    QString current_time_string() const;

    void update_time();

    void slider_move(int value);

    void slider_release();

    void seek_song(int seconds);

    void slider_press();

    void wait_playing();

    void unlock_playing();

  private:
    QGridLayout *layout;
    QPushButton *previous_song_button;
    QPushButton *play_pause_button;
    QPushButton *next_song_button;
    QSlider *tracking;
    QLabel *time;
    starling::PlaybackManager &playback_manager;

    int current_time = 0;

    std::thread timer;
    std::condition_variable timer_cv;
    std::mutex timer_lock;
    bool running = true;
};
} // namespace starling_ui