# MDP Render Plugin: Normal renderer.

AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libmdp_render_1x.la
noinst_DIST = mdp_render_1x.txt

INCLUDES = -I@top_srcdir@/src/
libmdp_render_1x_la_CCASFLAGS	= $(INCLUDES)

libmdp_render_1x_la_CFLAGS		= $(AM_CFLAGS)
libmdp_render_1x_la_CXXFLAGS		= $(AM_CXXFLAGS)
libmdp_render_1x_la_LDFLAGS		= $(AM_LDFLAGS)
libmdp_render_1x_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-shared

libmdp_render_1x_la_SOURCES = \
		mdp_render_1x_plugin.c \
		mdp_render_1x.cpp

noinst_HEADERS = \
		mdp_render_1x_plugin.h \
		mdp_render_1x_icon.h \
		mdp_render_1x.hpp

if GENS_X86_ASM
libmdp_render_1x_la_SOURCES += \
		mdp_render_1x_16_x86.S \
		mdp_render_1x_32_x86.S
noinst_HEADERS += \
		mdp_render_1x_x86.h
endif # GENS_X86_ASM
