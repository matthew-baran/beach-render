add_executable(
  beach_test
  test_camera.cpp
)

target_include_directories(beach_test PUBLIC "${CMAKE_SOURCE_DIR}/src" "C:/include" )

target_link_libraries(
  beach_test
  glad
  GTest::gtest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(beach_test)
