cmake_minimum_required(VERSION 3.20)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
#add_executable(tests test.cpp)
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_compile_definitions(__TEST_DEF__)

# These tests need their own main
add_executable(starling-tests
    sound_file_tests/test_wav.cpp
    time_tests/song_time_tests.cpp
    music_queue_tests/queue_tests.cpp
    player_cache_tests.cpp
    playback_engine_tests.cpp
    
    ${CMAKE_SOURCE_DIR}/src/ui/song_time.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/player_cache.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/playback_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/sound/playback/test_playback.cpp
)

target_link_libraries(starling-tests PRIVATE
    Catch2::Catch2WithMain
    #pulse
    #pulse-simple
)

target_include_directories(starling-tests
  PRIVATE ${CMAKE_SOURCE_DIR}/src
  PRIVATE ${CMAKE_SOURCE_DIR}/test
)

#link_directories(
    #/home/dockoneal/Source/pulseaudio/build/src/libpulsecommon-17.0.so
    #/home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse.so
    #/home/dockoneal/Source/pulseaudio/build/src/pulse/libpulse-simple.so
    #)