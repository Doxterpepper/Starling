cmake_minimum_required(VERSION 3.20)

project(starling)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_ON)
set(CMAKE_AUTORCC_ON)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# TODO: Add this to top level so we can lint tests too. Tests aren't in good shape so
# we only lint the main project.
set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(__DEBUG__)
#endif()

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    sound/playback/linux_playback.cpp
    sound/playback_manager.cpp
    sound/player_cache.cpp
    sound/playback_engine.cpp
    file_entry.cpp
    ui/player_controls.cpp
    ui/song_time.cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/src
)


link_directories(
    /home/dockoneal/Source/pulseaudio/build/src/libpulsecommon-17.0.so
    /home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse.so
    /home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse-simple.so
    )
target_link_libraries(${PROJECT_NAME}
    Qt5::Core Qt5::Gui Qt5::Widgets
    pulse
    pulse-simple
)
