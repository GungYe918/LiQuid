# stb_image 컴파일(정적 라이브러리 생성)용 Makefile

CC = gcc
CFLAGS = -Os -s -I.
TARGET = libstb_image.a

all: $(TARGET)

stb_image.o: stb_image.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): stb_image.o
	ar rcs $@ $^

clean:
	rm -f *.o *.a
