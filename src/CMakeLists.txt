add_executable(beach main.cpp)

target_include_directories(beach PUBLIC C:/include ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(beach PRIVATE camera.cpp camera.hpp)
target_sources(beach PRIVATE geometry.cpp geometry.hpp)
target_sources(beach PRIVATE gl_helpers.cpp gl_helpers.hpp)
target_sources(beach PRIVATE image.hpp)
target_sources(beach PRIVATE mesh.cpp mesh.hpp)
target_sources(beach PRIVATE model.cpp model.hpp)
target_sources(beach PRIVATE shader.hpp)
target_sources(beach PRIVATE terrain.cpp terrain.hpp)
target_sources(beach PRIVATE wave.cpp wave.hpp)
target_sources(beach PRIVATE window_management.cpp window_management.hpp)

# target_link_libraries(beach C:/lib/assimp-vc142-mtd.dll)
target_link_libraries(beach C:/lib/assimp-vc142-mtd.lib)

target_link_libraries(beach stbi)
target_link_libraries(beach glad)

add_custom_command(TARGET beach POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:beach> ${CMAKE_BINARY_DIR}/bin)