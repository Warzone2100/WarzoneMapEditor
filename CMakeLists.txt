cmake_minimum_required(VERSION 3.9)
project(warzone_map_editor)

set(CMAKE_COLOR_MAKEFILE   ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(WMT_BUILD_EXECUTABLE OFF CACHE BOOL "Build WMT executable" FORCE)
set(WMT_INCLUDE_LOG OFF CACHE BOOL "Build WMT and log library" FORCE)
add_definitions(-DLOG_USE_COLOR)
add_subdirectory(lib/WMT)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

file(GLOB srcfiles "src/*.cpp" "src/*.c" "src/*.hpp" "src/*.h")
file(GLOB libfiles "lib/*.cpp" "lib/*.c" "lib/*.hpp" "lib/*.h")
file(GLOB imguifiles "lib/imgui/*.cpp" "lib/imgui/*.h")
file(GLOB gladfiles "lib/glad/src/glad.cpp" "lib/glad/include/glad.h")

add_executable(main ${libfiles} ${srcfiles} ${imguifiles} ${gladfiles})
link_directories("lib/" "lib/glad/src/")
include_directories("lib/glad/include/" "lib/imgui/" ${GLFW3_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})
target_link_libraries(main libwmt ${GLFW3_LIBRARY} ${CMAKE_DL_LIBS} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
