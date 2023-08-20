cmake_minimum_required(VERSION 3.5.0)
project(beach VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP True
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(stbi STATIC C:/pkg/stbi/stb_image_init.cpp)
target_include_directories(stbi PUBLIC C:/pkg/stbi)

add_library(glad STATIC C:/pkg/glad/src/glad.c)
target_include_directories(glad PRIVATE C:/include)
target_link_libraries(glad C:/lib/glfw3.lib)

add_custom_target(copy_shaders ALL 
                    ${CMAKE_COMMAND} -E copy_directory
                        "${CMAKE_SOURCE_DIR}/src/shaders" "${CMAKE_BINARY_DIR}/bin/shaders"
                    COMMENT "Copy shaders to build tree" VERBATIM)
add_custom_target(copy_resources ALL
                    ${CMAKE_COMMAND} -E copy_directory
                        "${CMAKE_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/resources"
                    COMMENT "Copy resources to build tree" VERBATIM)

add_subdirectory(src)
add_subdirectory(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
