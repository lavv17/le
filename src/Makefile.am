bin_PROGRAMS = le

le_SOURCES = about.cc block.cc calc.cc chset.cc cmd.cc color.cc edit.cc\
 editcalc.cc ext.cc file.cc format.cc frames.cc getch.cc getstr.cc\
 help.cc history.cc kern.cc keymap.cc keymapdf.cc keymapfn.cc loadsave.cc\
 mainmenu.cc menu.cc menu1.cc options.cc pipe.cc rus.cc screen.cc search.cc\
 signals.cc tables.cc textpoin.cc user.cc window.cc about.h block.h\
 calc.h chset.h cmd.h color.h edit.h file.h history.h inline.h keymap.h\
 menu.h menu1.h options.h rus.h screen.h search.h textpoin.h user.h\
 window.h highli.cc highli.h clipbrd.cc clipbrd.h keynames.h keynames.cc\
 mouse.h mouse.cc getch.h format.h colormnu.cc colormnu.h bm.cc bm.h

le_LDADD = $(top_builddir)/lib/liblib.a $(CURSES_LIBS)

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include $(CURSES_INCLUDES)
AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST = le.hlp
pkgdata_DATA = le.hlp
