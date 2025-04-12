.PHONY: all clean help liquid-test


# 사용자 지정 명령어 목표
GOAL := $(MAKECMDGOALS)

BACKEND ?= x11

VALID_BACKENDS := x11 wayland fb
ifneq ($(filter $(BACKEND),$(VALID_BACKENDS)), $(BACKEND))
$(info [ERROR] Invalid BACKEND: '$(BACKEND)')
$(info Supported values are:)
$(info   - x11)
$(info   - wayland)
$(info   - fb)
$(error Invalid BACKEND specified)
endif

# Wayland 프로토콜 생성 관련 경로
WAYLAND_PROTOCOLS = /usr/share/wayland-protocols
WAYLAND_HEADERS = src/platform/xdg-shell-client-protocol.h
LDFLAGS_WAYLAND = 

# 백엔드 분기 정의
ifeq ($(BACKEND), x11)
PLATFORM_DEFINE = -DLIQUID_BACKEND_X11
PLATFORM_SRC = src/platform/platform_x11.c
endif

ifeq ($(BACKEND), wayland)
PLATFORM_DEFINE = -DLIQUID_BACKEND_WAYLAND
PLATFORM_SRC = src/platform/platform_wayland.c  \
                  src/platform/xdg-shell-protocol.c    \
                  $(WAYLAND_CODE)
LDFLAGS += -lwayland-client
WAYLAND_GENERATED = $(WAYLAND_HEADER) $(WAYLAND_CODE)
LDFLAGS_WAYLAND = -lwayland-client
endif

ifeq ($(BACKEND), fb)
PLATFORM_DEFINE = -DLIQUID_BACKEND_FB
PLATFORM_SRC = src/platform/platform_fb.c
endif


# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Iinclude -Isrc -Isrc/platform $(PLATFORM_DEFINE)
LDFLAGS = -lX11 -lm

# 소스 파일 목록
SRC = main.c \
      src/liquid_core.c \
      src/liquid_graphics.c \
      src/liquid_event.c \
      src/liquid_path.c \
      src/canvas.c      \

# 경량화 옵션이 있는 경우
ifdef LIQUID_LITE
CFLAGS += -DLIQUID_LITE
endif

# 기본 빌드 대상
all: liquid-test

# xdg-shell 헤더 자동 생성
# Wayland 헤더/코드 자동 생성 (wayland 백엔드일 때만)
ifeq ($(BACKEND), wayland)
$(WAYLAND_HEADER):
	@echo "🛠 Generating xdg-shell-client-protocol.h..."
	wayland-scanner client-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml \
		$@

$(WAYLAND_CODE):
	@echo "🛠 Generating xdg-shell-protocol.c..."
	wayland-scanner private-code \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml \
		$@
endif

liquid-test:
	$(CC) $(CFLAGS) -o $@ $(SRC) $(WAYLAND_GENERATED)     $(LDFLAGS)  \
      $(LDFLAGS_WAYLAND)

# 빌드 정리
clean:
	rm -f liquid-test

# 도움말 출력
help:
	@echo ""
	@echo "💧 Liquid Graphics Library Build System"
	@echo ""
	@echo "Usage:"
	@echo "  make BACKEND=<backend> [LIQUID_LITE=1] [target]"
	@echo ""
	@echo "Available BACKEND values:"
	@echo "  x11       - X11 backend"
	@echo "  wayland   - Wayland backend"
	@echo "  fb        - Framebuffer backend"
	@echo ""
	@echo "Extra Options:"
	@echo "  LIQUID_LITE=1   - Enable float-free (MCU-optimized) mode"
	@echo ""
	@echo "Available Targets:"
	@echo "  liquid-test     - Build the main test binary"
	@echo "  clean           - Remove the build output"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make BACKEND=x11"
	@echo "  make BACKEND=fb LIQUID_LITE=1"
	@echo ""
