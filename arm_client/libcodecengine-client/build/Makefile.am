AM_CPPFLAGS		= -I$(top_srcdir)/include -Wall

LDADD			= libcodecengine-client.la

SUBDIRS			= osal cstubs

lib_LTLIBRARIES 	= libcodecengine-client.la
nodist_libcodecengine_client_la_SOURCES	= $(top_srcdir)/config.h
libcodecengine_client_la_SOURCES	= $(top_srcdir)/src/main.c \
					  $(TIDSP_PATH_RTCFG)/rtcfg.c \
					  $(TIDSP_PATH_RTCFG)/rtcfg_fcinit.c \
					  $(TIDSP_PATH_RTCFG)/rtcfg_remote_config.c




#TESTS					=

#noinst_PROGRAMS				= $(TESTS)

#test_name_SOURCES			= $(SOURCES) $(top_srcdir)/tests/test_name.c
