noinst_LTLIBRARIES = libhdhomerun.la

libhdhomerun_la_SOURCES = hdhomerun_channels.c \
                          hdhomerun_channelscan.c \
                          hdhomerun_control.c \
                          hdhomerun_debug.c \
                          hdhomerun_device.c \
                          hdhomerun_device_selector.c \
                          hdhomerun_discover.c \
                          hdhomerun_os_posix.c \
                          hdhomerun_pkt.c \
                          hdhomerun_sock_posix.c \
                          hdhomerun_video.c

libhdhomerun_la_CPPFLAGS=-O2

$(LIB): libhdhomerun.la
	cp -f .libs/libhdhomerun.a .
	cp -f .libs/libhdhomerun.la $(LIB)

