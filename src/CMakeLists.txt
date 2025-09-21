cmake_minimum_required(VERSION 3.20)

project(starling)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_ON)
set(CMAKE_AUTORCC_ON)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

add_compile_options(-Wall -Werror)
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)


find_package(ALSA)

add_executable(${PROJECT_NAME}
    main.cpp
    sound/playback.cpp
)


link_directories(
    /home/dockoneal/Source/pulseaudio/build/src/libpulsecommon-17.0.so
    /home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse.so
    /home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse-simple.so
    )
target_link_libraries(${PROJECT_NAME}
    Qt5::Core Qt5::Gui Qt5::Widgets
    Qt5::Svg
    ALSA::ALSA
    pulse
    pulse-simple
)