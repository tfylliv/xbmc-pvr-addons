#
# Makefile for the SiliconDust HDHomeRun add-on for XBMC PVR
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = XBMC_Hdhomerun
ADDONNAME       = pvr.hdhomerun
LIBNAME         = libpvrhdhomerun-addon
lib_LTLIBRARIES = libpvrhdhomerun-addon.la

LIBS            = @abs_top_srcdir@/lib/libhdhomerun/libhdhomerun.la

AM_CPPFLAGS = -I@abs_top_srcdir@/lib/libhdhomerun

include ../Makefile.include.am

libpvrhdhomerun_addon_la_SOURCES = src/client.cpp \
                              src/PVRHDHomeRun.cpp \
                              src/HDHomerunChannel.cpp
libpvrhdhomerun_addon_la_LDFLAGS = @TARGET_LDFLAGS@

