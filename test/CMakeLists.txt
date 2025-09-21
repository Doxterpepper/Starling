find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
#add_executable(tests test.cpp)
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# These tests need their own main
add_executable(starling-tests
    sound_file_tests/test_wav.cpp
)

target_link_libraries(starling-tests PRIVATE
    Catch2::Catch2WithMain)

target_include_directories(starling-tests
  PRIVATE ${CMAKE_SOURCE_DIR}/src
)