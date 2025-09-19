cmake_minimum_required(VERSION 3.20)

project(starling)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_ON)
set(CMAKE_AUTORCC_ON)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Werror)
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

find_package(Qt5 COMPONENTS Core Gui Widgets Svg REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    playback.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core Qt5::Gui Qt5::Widgets
    Qt5::Svg
    pulse
    pulse-simple
)