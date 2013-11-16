VERSION=unknown
VPATH=src_main_service:src_shutdown_service:src_client_service:luna-service

ifdef DEVICE
# Device build
STAGING_DIR=/srv/preware/build/staging/armv7
ifeq ($(shell uname -s),Darwin)
CC=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc
else
CC=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-gcc
endif
else
# Emulator build
STAGING_DIR=/srv/preware/build/staging/i686
ifeq ($(shell uname -s),Darwin)
CC=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-gcc --sysroot=/opt/PalmPDK/i686-gcc/sys-root
else
CC=/srv/preware/build/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-gcc
endif
endif

CPPFLAGS := -g -DVERSION=\"${VERSION}\" -I${STAGING_DIR}/usr/include/glib-2.0 -I${STAGING_DIR}/usr/lib/glib-2.0/include -I${STAGING_DIR}/usr/include -I${STAGING_DIR}/usr/include/SDL -Iluna-service -I/home/nin/AndroidChroot/linux-2.6.32/include
LDFLAGS  := -g -L${STAGING_DIR}/usr/lib -llunaservice -lmjson -lglib-2.0 -lSDL -Wl,--allow-shlib-undefined

all: main_service shutdown_service client_service client
main_service: main_service.o luna_service.o luna_methods_main.o
shutdown_service: shutdown_service.o luna_service.o luna_methods_shutdown.o
client_service: client_service.o luna_service.o luna_methods_client.o
client: client.o

clobber:
	rm -rf *.o main_service shutdown_service client_service client
