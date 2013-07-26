#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([libcodecengine-client], [0.0.1-trunk])
AM_INIT_AUTOMAKE([foreign color-tests -Wall -Werror])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])


# We want more functions
AC_GNU_SOURCE


# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX


# Checks for libraries.
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_ASSERT
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_CHECK_HEADERS([string.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T


# Codec engine and related stuff paths configuration
AC_ARG_WITH([ti-dsp-xdctools],[AS_HELP_STRING([--with-ti-dsp-xdctools=path],[define location of TI DSP XDC Tools])],,[with_ti_dsp_xdctools="no"])
AS_IF([test "x$with_ti_dsp_xdctools" = xno], [AC_MSG_ERROR([TI DSP XDC Tools path must be defined])])
AC_ARG_WITH([ti-dsp-xdais],[AS_HELP_STRING([--with-ti-dsp-xdais=path],[define location of TI DSP XDAIS])],,[with_ti_dsp_xdais="no"])
AS_IF([test "x$with_ti_dsp_xdais" = xno], [AC_MSG_ERROR([TI DSP XDAIS path must be defined])])
AC_ARG_WITH([ti-dsp-ce],[AS_HELP_STRING([--with-ti-dsp-ce=path],[define location of TI DSP Codec Engine])],,[with_ti_dsp_ce="no"])
AS_IF([test "x$with_ti_dsp_ce" = xno], [AC_MSG_ERROR([TI DSP Codec Engine path must be defined])])
AC_ARG_WITH([ti-dsp-fc],[AS_HELP_STRING([--with-ti-dsp-fc=path],[define location of TI DSP Framework Components])],,[with_ti_dsp_fc="no"])
AS_IF([test "x$with_ti_dsp_fc" = xno], [AC_MSG_ERROR([TI DSP Framework Components path must be defined])])
AC_ARG_WITH([ti-dsp-osal],[AS_HELP_STRING([--with-ti-dsp-osal=path],[define location of TI DSP OSAL])],,[with_ti_dsp_osal="no"])
AS_IF([test "x$with_ti_dsp_osal" = xno], [AC_MSG_ERROR([TI DSP OSAL path must be defined])])
AC_ARG_WITH([ti-dsp-cmem],[AS_HELP_STRING([--with-ti-dsp-cmem=path],[define location of TI DSP CMEM (LinuxUtils)])],,[with_ti_dsp_cmem="no"])
AS_IF([test "x$with_ti_dsp_cmem" = xno], [AC_MSG_ERROR([TI DSP CMEM (LinuxUtils) path must be defined])])
AC_ARG_WITH([ti-dsp-syslink],[AS_HELP_STRING([--with-ti-dsp-syslink=path],[define location of TI DSP SysLink])],,[with_ti_dsp_syslink="no"])
AS_IF([test "x$with_ti_dsp_syslink" = xno], [AC_MSG_ERROR([TI DSP SysLink path must be defined])])

AC_SUBST([TIDSP_EXPORT_CPPFLAGS_DEFS], "-Dxdc_target_types__=gnu/targets/arm/std.h -Dxdc_target_name__=GCArmv5T")
AC_SUBST([TIDSP_EXPORT_CPPFLAGS_INCS], "-I${with_ti_dsp_xdctools}/packages -I${with_ti_dsp_xdais}/packages -I${with_ti_dsp_ce}/packages")
AC_SUBST([TIDSP_PRIVATE_CPPFLAGS_INCS], "-I${with_ti_dsp_fc}/packages -I${with_ti_dsp_osal}/packages")
AC_SUBST([TIDSP_EXPORT_LDFLAGS], "-L${with_ti_dsp_xdctools}/packages -L${with_ti_dsp_xdais}/packages -L${with_ti_dsp_ce}/packages -L${with_ti_dsp_fc}/packages -L${with_ti_dsp_osal}/packages -L${with_ti_dsp_cmem}/packages -L${with_ti_dsp_syslink}/packages")
AC_SUBST([TIDSP_EXPORT_LDLIBS], "${with_ti_dsp_ce}/packages/ti/sdo/ce/utils/rtcfg/linux/OMAPL138/ce_remote_release.cmd ${with_ti_dsp_ce}/packages/ti/sdo/ce/utils/rtcfg/linux/OMAPL138/fc_release.cmd")
AC_SUBST([TIDSP_PRIVATE_LDLIBS], "-lpthread -lrt -ldl")
CPPFLAGS+=" ${TIDSP_EXPORT_CPPFLAGS_DEFS} ${TIDSP_EXPORT_CPPFLAGS_INCS} ${TIDSP_PRIVATE_CPPFLAGS_INCS}"
LDFLAGS+=" ${TIDSP_PRIVATE_LDLIBS}"

AC_CHECK_HEADERS([xdc/std.h],,[AC_MSG_ERROR([cannot find TI-DSP XDC headers])])
AC_CHECK_HEADERS([ti/sdo/ce/Engine.h ti/sdo/ce/Server.h ti/sdo/ce/CERuntime.h],,[AC_MSG_ERROR([cannot find TI DSP Codec Engine headers])],[[#include <xdc/std.h>]])

AC_SUBST([TIDSP_PATH_RTCFG_SRC], [${with_ti_dsp_ce}/packages/ti/sdo/ce/utils/rtcfg])
AC_SUBST([TIDSP_PATH_CSTUBS_SRC], [${with_ti_dsp_osal}/packages/linuxdist/cstubs])
AC_SUBST([TIDSP_PATH_OSAL_SRC], [${with_ti_dsp_osal}/packages/linuxdist/preconfig/production])
AC_SUBST([TIDSP_PATH_OSAL_ROOT], [${with_ti_dsp_osal}])


# Checks for library functions.
gl_VISIBILITY


# Check for pkgconfig dependencies and setup flags
PKGCONFIG_REQUIRES=""
AC_SUBST([PKGCONFIG_REQUIRES])
#PKG_CHECK_MODULES([PKGCONFIG], [${PKGCONFIG_REQUIRES}])
#CPPFLAGS+=" ${PKGCONFIG_CFLAGS}"
#LIBS+=" ${PKGCONFIG_LIBS}"


AC_CONFIG_FILES([Makefile
		 build/Makefile
		 ${PACKAGE}.pc])

AC_OUTPUT
