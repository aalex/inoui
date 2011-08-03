#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(INOUI, 0.1.0, [alexandre@quessy.net], [inoui], [http://alexandre.quessy.net])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4]) # check for m4 macros in directory m4
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE()

AS_VERSION
AC_DEFINE_UNQUOTED(PACKAGE_VERSION_MAJOR, $PACKAGE_VERSION_MAJOR, [major version])
AC_DEFINE_UNQUOTED(PACKAGE_VERSION_MINOR, $PACKAGE_VERSION_MINOR, [minor version])
AC_DEFINE_UNQUOTED(PACKAGE_VERSION_MICRO, $PACKAGE_VERSION_MICRO, [micro version])

# Check for C++ compiler
AC_PROG_CXX

# check for programs
AC_PROG_INSTALL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
PKG_PROG_PKG_CONFIG

AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([inet_ntoa memset socket])

AC_CHECK_LIB([pthread],[pthread_exit],[AC_SUBST([LIBTHREAD_CXXFLAGS],["-lpthread"])])
AC_CHECK_LIB([m],[sin],[AC_SUBST([LIBM_CXXFLAGS],["-lm"])])

# Set LIBLO with pkg-config
PKG_CHECK_MODULES(SPATOSC, spatosc-0.2, have_spatosc=true, have_spatosc=false)
if test "x${have_spatosc}" = "xfalse" ; then
    AC_MSG_ERROR([Missing spatosc])
fi
AC_SUBST([SPATOSC_LIBS])
AC_SUBST([SPATOSC_CFLAGS])

# Clutter
PKG_CHECK_MODULES([CLUTTER], [clutter-1.0 >= 1.0], have_clutter=true, have_clutter=false)
if test "x${have_clutter}" = "xfalse" ; then
    AC_MSG_ERROR([missing package: libclutter])
fi
AC_SUBST([CLUTTER_LIBS])
AC_SUBST([CLUTTER_CFLAGS])
# Clutter depends on:
# libjson-glib-dev and others

# sndfile
PKG_CHECK_MODULES([SNDFILE], [sndfile >= 1.0], have_sndfile=true, have_sndfile=false)
if test "x${have_sndfile}" = "xfalse" ; then
    AC_MSG_ERROR([missing package: libsndfile])
fi
AC_SUBST([SNDFILE_LIBS])
AC_SUBST([SNDFILE_CFLAGS])

# libxml2
PKG_CHECK_MODULES([LIBXML], [libxml-2.0], have_libxml=true, have_libxml=false)
if test "x${have_libxml}" = "xfalse" ; then
    AC_MSG_ERROR([missing package: libxml2-dev])
fi
AC_SUBST([LIBXML_LIBS])
AC_SUBST([LIBXML_CFLAGS])

# BOOST
AX_BOOST_BASE([1.35])

AX_BOOST_PROGRAM_OPTIONS
if test "x${ax_cv_boost_program_options}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-program-options-dev])
fi

AX_BOOST_FILESYSTEM
if test "x${ax_cv_boost_filesystem}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-filesystem-dev])
fi

AX_BOOST_THREAD
if test "x${ax_cv_boost_thread}" = "xno" ; then
    AC_MSG_ERROR([missing package: libboost-thread-dev])
fi

AX_BOOST_DATE_TIME
if test "x${ax_cv_boost_date_time}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-date-time-dev])
fi

AX_BOOST_SYSTEM
if test "x${ax_cv_boost_system}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-system-dev])
fi

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
    AM_MISSING_PROG(HELP2MAN, help2man)
else
    HELP2MAN=:
fi

AS_AC_EXPAND(DATADIR, $datadir)
AC_MSG_NOTICE(Storing data files in $DATADIR)

PIXMAPS_DIR=$DATADIR/pixmaps
AC_SUBST(PIXMAPS_DIR)
AC_MSG_NOTICE(Storing pixmaps files in $PIXMAPS_DIR)

AC_DEFINE_UNQUOTED([DATADIR], "${DATADIR}", "Data dir")
AC_DEFINE_UNQUOTED([PIXMAPS_DIR], "${PIXMAPS_DIR}", "Pixmaps dir")

AC_SUBST(CXXFLAGS, " -O2 -Wall -Werror -Wextra -Wfatal-errors") 

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/doxyfile
    tests/Makefile
    data/Makefile
    ])
AC_OUTPUT

