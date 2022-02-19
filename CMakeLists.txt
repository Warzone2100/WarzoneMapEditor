cmake_minimum_required(VERSION 3.9)

include("cmake/git.cmake")

project(warzone_map_editor VERSION ${GIT_TAG})

set(CMAKE_COLOR_MAKEFILE   ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(WMT_BUILD_EXECUTABLE OFF CACHE BOOL "Build WMT executable" FORCE)
set(WMT_INCLUDE_LOG OFF CACHE BOOL "Build WMT and log library" FORCE)
add_definitions(-DLOG_USE_COLOR)
add_subdirectory(lib/WMT)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(LibConfig REQUIRED)

configure_file(src/build.h.in src/build.h)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
file(GLOB srcfiles "src/*.cpp" "src/*.c" "src/*.hpp" "src/*.h" "lib/imgui/*.cpp" "lib/imgui/*.h")
file(GLOB libfiles "lib/*.cpp" "lib/*.c" "lib/*.hpp" "lib/*.h")
file(GLOB gladfiles "lib/glad/src/glad.cpp" "lib/glad/include/glad.h")

add_executable(main ${libfiles} ${srcfiles} ${gladfiles})
set_property(TARGET main PROPERTY CXX_STANDARD 17)
link_directories("lib/" "lib/glad/src/")
include_directories(${CMAKE_CURRENT_BINARY_DIR} "lib/glad/include/" "lib/imgui/" "lib/" ${GLFW3_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})
target_link_libraries(main libwmt GL GLU glfw GLEW config++ ${GLFW3_LIBRARY} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${LIBCONFIGPP_LIBRARY})

set(GLAD_DIR "lib/glad")
add_library("glad" "${GLAD_DIR}/src/glad")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(main PRIVATE "${GLAD_DIR}/include")
target_link_libraries(main "glad" "${CMAKE_DL_LIBS}")

add_custom_command( TARGET main PRE_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:main>/data/)

add_custom_command( TARGET main PRE_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/src/wmeglobal.cfg $<TARGET_FILE_DIR:main>)
