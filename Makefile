.PHONY: all clean

CC = g++
CFLAGS = -Wall -g -Iglad/include/
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lglfw -lpng -ldl -lGLEW -L.

all: main

main: main.o mypng.o log.o myshader.o glad.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

main.o: main.cpp mypng.o log.o
	$(CC) $< -c -o $@ $(CFLAGS)
mypng.o: mypng.c mypng.h log.o
	$(CC) $< -c -o $@ $(CFLAGS)
log.o: log.cpp log.hpp
	$(CC) -DLOG_USE_COLOR $< -c -o $@ $(CFLAGS)
myshader.o: myshader.c myshader.h
	$(CC) $< -c -o $@ $(CFLAGS)
glad.o: glad/src/glad.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	$(RM) main main.o mypng.o myshader.o log.o
