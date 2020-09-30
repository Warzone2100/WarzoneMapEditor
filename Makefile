.PHONY: all clean

CC = g++
CFLAGS = -Wall -g -Iglad/include/ -Iimgui/
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lglfw -lpng -ldl -lGLEW -L.

all: main

main: main.o log.o myshader.o glad.o pie.o imgui.o imgui_draw.o imgui_widgets.o imgui_impl_opengl3.o imgui_impl_sdl.o imgui_demo.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

main.o: main.cpp log.o imgui.o
	$(CC) $< -c -o $@ $(CFLAGS)
pie.o: pie.c pie.h log.o
	$(CC) $< -c -o $@ $(CFLAGS)
log.o: log.cpp log.hpp
	$(CC) -DLOG_USE_COLOR $< -c -o $@ $(CFLAGS)
myshader.o: myshader.c myshader.h
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

clean:
	$(RM) main main.o mypng.o myshader.o log.o imgui.o
