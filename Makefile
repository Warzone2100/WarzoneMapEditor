.PHONY: all clean

CC = g++
CFLAGS = -Wall -ggdb -std=c++17 -DLOG_USE_COLOR -DIMGUI_IMPL_OPENGL_LOADER_GLAD -Ilib/WMT/lib/ -Ilib/glad/include/ -Ilib/imgui/ -Ilib/ -Isrc/
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGL -lGLU -lglfw -lpng -ldl -lGLEW

SOURCES  = $(wildcard src/*.cpp lib/*.cpp lib/imgui/*.cpp lib/glad/src/*.c)
SOURCES += lib/WMT/lib/zip.cpp lib/WMT/lib/wmt.cpp
OBJECTS  = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))
DEPS     = $(patsubst %.cpp,%.d,$(patsubst %.c,%.d,$(SOURCES)))

all: main

depend: $(SOURCES)
	$(CC) -MM $(CFLAGS) $(SOURCES) > depend

main: $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

%.o : %.c
	$(CC) $< -c -o $@ $(CFLAGS)
%.o : %.cpp
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	$(RM) main depend $(OBJECTS)

include depend
