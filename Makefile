.PHONY: all clean help liquid-test


# 사용자 지정 명령어 목표
GOAL := $(MAKECMDGOALS)

BACKEND ?= x11

VALID_BACKENDS := x11 wayland fb xrender
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
LDFLAGS	+=	-lX11
endif

ifeq ($(BACKEND), xrender)
PLATFORM_DEFINE = -DLIQUID_BACKEND_XRENDER
PLATFORM_SRC = src/platform/platform_xrender.c
LDFLAGS	+=	-lX11 -lXrender
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
LDFLAGS += -lm

# stb_image DIR 설정
STB_IMAGE_DIR := include/Image/STB_image
STB_IMAGE_LIB := $(STB_IMAGE_DIR)/libstb_image.a
CFLAGS += -I$(STB_IMAGE_DIR)
LDFLAGS += -L$(STB_IMAGE_DIR) -lstb_image


# 소스 파일 목록
SRC = main.c \
      src/liquid_core.c \
      src/liquid_graphics.c \
      src/liquid_event.c \
      src/liquid_path.c \
	  src/liquid_matrix.c	\
	  src/liquid_text.c		\
	  src/liquid_image.c	\
	  src/font.c	\
      src/canvas.c      \

# 최적화 옵션
ifdef LIQUID_LITE
	CFLAGS += -DLIQUID_LITE
    LDFLAGS += -Wl,--gc-sections
endif

# 경량화 단계 -> [ basic, slim, extreme, baremetal ]
# 경량화 단계별 옵션 분기
ifeq ($(OPTIMIZE),basic)
	CFLAGS += -DLIQUID_LITE_BASIC
	CFLAGS += -Os -ffunction-sections -fdata-sections
	LDFLAGS += -Wl,--gc-sections

else ifeq ($(OPTIMIZE),slim)
	CFLAGS += -DLIQUID_LITE_SLIM
	CFLAGS += -Os -s -ffunction-sections -fdata-sections -fomit-frame-pointer
	LDFLAGS += -Wl,--gc-sections

else ifeq ($(OPTIMIZE),baremetal)
	CFLAGS += -DLIQUID_LITE_BAREMETAL
	CFLAGS += -Os -s -march=native -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-exceptions
	LDFLAGS += -Wl,--gc-sections

else ifeq ($(OPTIMIZE),extreme)
	CFLAGS += -DLIQUID_LITE_EXTREME
	CFLAGS += -O3 -s -flto -fno-plt -ffunction-sections -fdata-sections -fomit-frame-pointer -march=native
	LDFLAGS += -flto -Wl,--gc-sections -Wl,-O1

endif


# freetype 없이 빌드 (내장 비트맵 폰트 사용)
ifdef NO_FREETYPE
	CFLAGS += -DNO_FREETYPE
else
    CFLAGS += 	$(shell pkg-config --cflags freetype2)
    LDFLAGS += 	$(shell pkg-config --libs freetype2)
endif

# stb_image 없이 빌드
ifdef NO_STB_IMAGE
	SRC		:= $(filter-out src/liquid_image.c, $(SRC))
	CFLAGS	:= $(filter-out -I$(STB_IMAGE_DIR), $(CFLAGS))
	LDFLAGS := $(filter-out -L$(STB_IMAGE_DIR) -lstb_image, $(LDFLAGS))
	STB_IMAGE_LIB :=
else
	STB_IMAGE_LIB := $(STB_IMAGE_DIR)/libstb_image.a
	CFLAGS += -I$(STB_IMAGE_DIR)
	LDFLAGS += -L$(STB_IMAGE_DIR) -lstb_image
endif

# 기본 빌드 대상
all: $(STB_IMAGE_LIB)	liquid-test

# stb_image용 서브 Makefile 호출
$(STB_IMAGE_LIB):
	$(MAKE) -C $(STB_IMAGE_DIR)


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
	$(MAKE) -C $(STB_IMAGE_DIR) clean

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
