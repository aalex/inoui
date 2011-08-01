bin_PROGRAMS = inoui
EXTRA_DIST = doxyfile.in

html-local: doxygen
doxygen:
	@if [ -z `which doxygen` ]; then echo "No Doxygen!"; else doxygen; fi
clean-local:
	rm -rf html

headers = \
	avatar.h \
	circle.h \
	point.h \
	map.h \
	maths.h \
	pprint.h \
	grid.h \
	soundinfo.h \
	timer.h \
	unused.h \
	statesaving.h \
	config.h

inoui_SOURCES = \
	avatar.cpp \
	circle.cpp \
	grid.cpp \
	map.cpp \
	maths.cpp \
	main.cpp \
	pprint.cpp \
	point.cpp \
	soundinfo.cpp \
	statesaving.cpp \
	timer.cpp \
	$(headers)

inoui_CXXFLAGS = \
	$(CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(SPATOSC_CFLAGS) \
	$(SNDFILE_CFLAGS) \
	$(LIBM_CXXFLAGS) \
	$(LIBTHREAD_CXXFLAGS) \
	$(LIBXML_CFLAGS)
 
inoui_LDADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LDFLAGS) \
	$(BOOST_LIBS)  \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_THREAD_LIB) \
	$(CLUTTER_LIBS) \
	$(SNDFILE_LIBS) \
	$(SPATOSC_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) \
	$(LDFLAGS) \
	$(LIBXML_LIBS)

