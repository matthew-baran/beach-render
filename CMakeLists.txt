cmake_minimum_required(VERSION 3.0.0)
project(beach VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)
enable_testing()

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

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
