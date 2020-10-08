.PHONY: all clean

CC = g++
CFLAGS = -Wall -ggdb -Ilib/WMT/lib/ -Ilib/glad/include/ -Ilib/imgui/ -Ilib/ -Isrc/
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lglfw -lpng -ldl -lGLEW

all: main

main: src/terrain.o lib/WMT/lib/wmt.o lib/WMT/lib/zip.o lib/imgui/imgui_file_dialog.o lib/imgui/imgui_impl_sdl.o lib/imgui/imgui_impl_opengl3.o lib/imgui/imgui_widgets.o lib/imgui/imgui_draw.o lib/imgui/imgui.o lib/glad/src/glad.o src/World3d.o src/myshader.o lib/log.o src/pie.o src/main.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

src/main.o: src/main.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
src/pie.o: src/pie.cpp src/pie.h
	$(CC) $< -c -o $@ $(CFLAGS)
lib/log.o: lib/log.cpp lib/log.hpp
	$(CC) -DLOG_USE_COLOR $< -c -o $@ $(CFLAGS)
src/myshader.o: src/myshader.cpp src/myshader.h
	$(CC) $< -c -o $@ $(CFLAGS)
src/World3d.o: src/World3d.cpp src/World3d.h
	$(CC) $< -c -o $@ $(CFLAGS)
src/terrain.o: src/terrain.cpp src/terrain.h
	$(CC) $< -c -o $@ $(CFLAGS)
src/other.o: src/other.cpp src/other.h
	$(CC) $< -c -o $@ $(CFLAGS)

lib/glad/src/glad.o: lib/glad/src/glad.c
	$(CC) $< -c -o $@ $(CFLAGS)
lib/imgui/imgui.o: lib/imgui/imgui.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
lib/imgui/imgui_draw.o: lib/imgui/imgui_draw.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
lib/imgui/imgui_widgets.o: lib/imgui/imgui_widgets.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
lib/imgui/imgui_impl_opengl3.o: lib/imgui/imgui_impl_opengl3.cpp
	$(CC) $< -DIMGUI_IMPL_OPENGL_LOADER_GLAD -c -o $@ $(CFLAGS)
lib/imgui/imgui_impl_sdl.o: lib/imgui/imgui_impl_sdl.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

lib/imgui/imgui_file_dialog.o: lib/imgui/ImGuiFileDialog.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

lib/WMT/lib/zip.o: lib/WMT/lib/zip.cpp lib/WMT/lib/zip.hpp
	$(CC) $< -o $@ -c
lib/WMT/lib/wmt.o: lib/WMT/lib/wmt.cpp lib/WMT/lib/wmt.hpp
	$(CC) $< -o $@ -c -std=c++17 -D_NOLOGBUILD

clean:
	$(RM) main lib/WMT/lib/wmt.o lib/WMT/lib/zip.o lib/imgui/imgui_file_dialog.o lib/imgui/imgui_impl_sdl.o lib/imgui/imgui_impl_opengl3.o lib/imgui/imgui_widgets.o lib/imgui/imgui_draw.o lib/imgui/imgui.o lib/glad/src/glad.o src/World3d.o src/myshader.o lib/log.o src/pie.o src/main.o
