bin_PROGRAMS = write_example read_example

read_example_SOURCES = read_example.c
read_example_LDADD = $(top_builddir)/src/libescdf.la

write_example_SOURCES = write_example.c
write_example_LDADD = $(top_builddir)/src/libescdf.la

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src

CLEANFILES = $(EXTRA_PROGRAMS) grid_scalarfield.*h5
