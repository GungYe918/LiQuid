CC = gcc
CFLAGS = -Wall -Iinclude -Isrc -Isrc/platform   -Iinclude
LDFLAGS = -lX11   -lm

SRC = main.c \
      src/liquid.c \
      render/software_renderer.c \
      src/platform/platform_x11.c	\
	  src/liquid_core.c \
      src/liquid_graphics.c \
      src/liquid_event.c \
      src/liquid_path.c \
      src/platform/platform_x11.c

ifdef LIQUID_NO_FLOAT
      CFLAGS      +=    -DLIQUID_NO_FLOAT
endif

all: liquid-test

liquid-test: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f liquid-test