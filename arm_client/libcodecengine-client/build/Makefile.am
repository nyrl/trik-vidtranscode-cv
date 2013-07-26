AM_CPPFLAGS		= -I$(top_srcdir)/include -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-maybe-uninitialized -fno-strict-aliasing

LDADD			= libcodecengine-client.la


lib_LTLIBRARIES 	= libcodecengine-client.la

nodist_libcodecengine_client_la_SOURCES	= $(top_srcdir)/config.h
libcodecengine_client_la_SOURCES	= $(top_srcdir)/src/main.c
libcodecengine_client_la_LIBADD		= libpriv_rtcfg.la libpriv_osal.la libpriv_cstubs.la


noinst_LTLIBRARIES	= libpriv_rtcfg.la libpriv_osal.la libpriv_cstubs.la

libpriv_rtcfg_la_SOURCES		= \
					  $(TIDSP_PATH_RTCFG_SRC)/rtcfg.c \
					  $(TIDSP_PATH_RTCFG_SRC)/rtcfg_fcinit.c \
					  $(TIDSP_PATH_RTCFG_SRC)/rtcfg_remote_config.c

libpriv_osal_la_SOURCES			= \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Assert.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Core-label.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Core-mem.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Core-params.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Diags.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Error.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Gate.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/GateNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/HeapStd.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Log.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/LoggerBuf.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/LoggerSys.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Memory.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Registry.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Rta.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Startup.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/SysMin.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/SysStd.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/System.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Text.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/Timestamp.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/TimestampNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/TimestampStd.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/Cache.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/CacheSupportNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/GateH.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/GateProcess.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/GateProcessSupportNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/GateThread.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/GateThreadSupportNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/Semaphore.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/SemProcess.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/SemThread.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/Sync.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/SyncGeneric.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/SyncNull.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/SyncSemThread.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/knl/Thread.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/GateProcessSupport.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/GateThreadSupport.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/SemProcessSupport.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/SemThreadSupport.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/ThreadSupport.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/xdcruntime/linux/TimestampPosix.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/ti/sdo/utils/loggers/LoggerSysTID_posix.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/xdcruntime/cstubs/LoggerFlex.c \
					  $(TIDSP_PATH_OSAL_ROOT)/packages/xdcruntime/heaps/HeapStdAlign.c \
					  $(TIDSP_PATH_OSAL_ROOT)/sources/xdc/runtime/xdc_init.cpp \
					  $(TIDSP_PATH_OSAL_SRC)/preConfig_pv5T.c

libpriv_cstubs_la_SOURCES		= \
					  $(TIDSP_PATH_CSTUBS_SRC)/LoggerPosix.c



#TESTS					= test_cmem_alloc

#noinst_PROGRAMS				= $(TESTS)

#test_cmem_alloc_SOURCES			= $(top_srcdir)/tests/test_cmem_alloc.c
#test_cmem_alloc_LDFLAGS			= $(LDFLAGS) $(TIDSP_EXPORT_LDFLAGS)
#test_cmem_alloc_LDADD			= $(LDADD) $(TIDSP_EXPORT_LDLIBS)

