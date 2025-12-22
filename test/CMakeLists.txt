cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)
find_package(Catch2 3 REQUIRED)

add_compile_definitions(__DEBUG__)
add_compile_definitions(__TEST_DEF__)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# These tests need their own main
add_executable(starling-tests
    sound_file_tests/test_wav.cpp
    time_tests/song_time_tests.cpp
    music_queue_tests/queue_tests.cpp
    player_cache_tests.cpp
    playback_engine_tests.cpp
    playback_manager_tests.cpp
    
    ${CMAKE_SOURCE_DIR}/src/ui/song_time.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/player_cache.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/playback_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/playback_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/playback/test_playback.cpp
)

target_link_libraries(starling-tests PRIVATE
    Catch2::Catch2WithMain
)

target_include_directories(starling-tests
  PRIVATE ${CMAKE_SOURCE_DIR}/src
  PRIVATE ${CMAKE_SOURCE_DIR}/test
)