.PHONY: all clean

CC = g++
CFLAGS = -Wall -ggdb -Iglad/include/ -Iimgui/
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lglfw -lpng -ldl -lGLEW -L.

all: main

main: main.o log.o myshader.o glad.o pie.o imgui.o imgui_draw.o imgui_widgets.o imgui_impl_opengl3.o imgui_impl_sdl.o imgui_demo.o wmt.o zip.o imgui_file_dialog.o World3d.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

main.o: main.cpp log.o imgui.o
	$(CC) $< -c -o $@ $(CFLAGS)
pie.o: pie.c pie.h log.o
	$(CC) $< -c -o $@ $(CFLAGS)
log.o: log.cpp log.hpp
	$(CC) -DLOG_USE_COLOR $< -c -o $@ $(CFLAGS)
myshader.o: myshader.c myshader.h
	$(CC) $< -c -o $@ $(CFLAGS)
World3d.o: World3d.cpp World3d.h
	$(CC) $< -c -o $@ $(CFLAGS)

glad.o: glad/src/glad.c
	$(CC) $< -c -o $@ $(CFLAGS)
imgui.o: imgui/imgui.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
imgui_draw.o: imgui/imgui_draw.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
imgui_widgets.o: imgui/imgui_widgets.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
imgui_impl_opengl3.o: imgui/imgui_impl_opengl3.cpp
	$(CC) $< -DIMGUI_IMPL_OPENGL_LOADER_GLAD -c -o $@ $(CFLAGS)
imgui_impl_sdl.o: imgui/imgui_impl_sdl.cpp
	$(CC) $< -c -o $@ $(CFLAGS)
imgui_demo.o: imgui/imgui_demo.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

imgui_file_dialog.o: imgui/ImGuiFileDialog.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

zip.o: WMT/lib/zip.cpp WMT/lib/zip.hpp
	$(CC) $< -o $@ -c
wmt.o: WMT/lib/wmt.cpp WMT/lib/wmt.hpp
	$(CC) $< -o $@ -c -std=c++17 -D_NOLOGBUILD

clean:
	$(RM) main main.o log.o myshader.o glad.o pie.o imgui.o imgui_draw.o imgui_widgets.o imgui_impl_opengl3.o imgui_impl_sdl.o imgui_demo.o wmt.o zip.o imgui_file_dialog.o World3d.o
