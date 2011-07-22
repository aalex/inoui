bin_PROGRAMS = inoui
EXTRA_DIST = doxyfile.in

html-local: doxygen
doxygen:
	@if [ -z `which doxygen` ]; then echo "No Doxygen!"; else doxygen; fi
clean-local:
	rm -rf html

headers = \
	circle.h \
	point.h \
	map.h \
	grid.h \
	main.cpp \
	config.h

inoui_SOURCES = \
	circle.cpp \
	grid.cpp \
	map.cpp \
	point.cpp \
	$(headers)

inoui_CXXFLAGS = \
	$(CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(SPATOSC_CFLAGS) \
	$(LIBM_CXXFLAGS) \
	$(LIBTHREAD_CXXFLAGS) \
	$(LIBXML_XFLAGS)
 
inoui_LDADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LDFLAGS) \
	$(BOOST_LIBS)  \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_THREAD_LIB) \
	$(CLUTTER_LIBS) \
	$(SPATOSC_LIBS) \
	$(LIBM_LIBS) \
	$(LIBTHREAD_LIBS) \
	$(LDFLAGS) \
	$(LIBXML_LIBS)

