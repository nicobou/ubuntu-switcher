pluginsdir = $(prefix)/nodejs/node-switcher

plugins_LTLIBRARIES = \
    switcher_addon.la

#libmyplugin info
switcher_addon_la_SOURCES = \
    switcher_addon.cpp

switcher_addon_la_includedir = \
    $(includedir)/switcher-$(LIBSWITCHER_API_VERSION)/switcher

switcher_addon_la_LDFLAGS = \
    $(GLIB_LIBS) \
    $(GMODULE_LIBS) \
    $(GST_LIBS) \
    $(JSONGLIB_LIBS) \
    $(SHMDATA_LIBS) \
    -L$(top_builddir)/switcher/.libs \
    -avoid-version \
    -bundle \
    -flat_namespace \
    -lswitcher-@LIBSWITCHER_API_VERSION@ \
    -module \
    -shrext .node \
    -undefined suppress 

switcher_addon_la_CXXFLAGS = \
    $(GLIB_CFLAGS) \
    $(GMODULE_CFLAGS) \
    $(GST_CFLAGS) \
    $(JSONGLIB_CFLAGS) \
    $(SHMDATA_CFLAGS) \
    -DEV_MULTIPLICITY=0 \
    -DG_LOG_DOMAIN=\"switcher\" \
    -DPIC \
    -D_GNU_SOURCE \
    -D_LARGEFILE_SOURCE \
    -I$(top_srcdir) \
    -I/usr/include/nodejs/deps/uv/include/ \
    -I/usr/include/nodejs/src \
    -fPIC

#    -D_FILE_OFFSET_BITS=64 \
#    -m64 
