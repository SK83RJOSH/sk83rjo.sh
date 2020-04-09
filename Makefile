rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CXX=em++
CPPFLAGS=-std=c++17 -MMD -MP -g -Wall -Isrc -Ilib -O2 -g
LDFLAGS=-s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s OFFSCREENCANVAS_SUPPORT=1 -s OFFSCREEN_FRAMEBUFFER=1 -s FULL_ES3=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s ALLOW_MEMORY_GROWTH=1
EMFLAGS=-g4 -o index.html --use-preload-plugins --preload-file assets --shell-file shell.html --source-map-base http://localhost:8080/

SRCS=$(call rwildcard,src,*.cpp)
OBJS=$(SRCS:.cpp=.o)
DEPS=$(OBJS:.o=.d)

.PHONY: clean all depend

all: index

%.o: %.c
	$(CXX) -c $(CPPFLAGS) $(LDFLAGS) -o $@ $<

index: $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) $(EMFLAGS)

clean:
	del /q /f /s *.d
	del /q /f /s *.o

-include $(DEPS)
