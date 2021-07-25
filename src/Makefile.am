bin_PROGRAMS = le

le_SOURCES = about.cc block.cc calc.cc chset.cc cmd.cc color.cc edit.cc\
 editcalc.cc ext.cc file.cc format.cc frames.cc getch.cc getstr.cc\
 help.cc history.cc kern.cc keymap.cc keymapdf.cc keymapfn.cc loadsave.cc\
 mainmenu.cc menu.cc menu1.cc options.cc pipe.cc rus.cc screen.cc search.cc\
 signals.cc tables.cc textpoin.cc user.cc window.cc about.h block.h\
 calc.h chset.h cmd.h color.h edit.h file.h history.h inline.h keymap.h\
 menu.h menu1.h options.h rus.h screen.h search.h textpoin.h user.h\
 window.h highli.cc highli.h clipbrd.cc clipbrd.h keynames.h keynames.cc\
 mouse.h mouse.cc getch.h format.h colormnu.cc colormnu.h bm.cc bm.h\
 mb.cc mb.h undo.cc undo.h regex.c regex.h wcwidth.c wcwidth1.c\
 action-name-func

GNULIB = $(top_builddir)/lib/libgnu.a

le_DEPENDENCIES = $(GNULIB)
le_LDADD = $(GNULIB) $(CURSES_LIBS) $(LIBINTL) $(LIBSOCKET) $(LIB_POLL) $(LIB_SELECT)

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(CURSES_INCLUDES) -DPKGDATADIR=\"$(DESTDIR)$(pkgdatadir)\"

EXTRA_DIST = le.hlp make-action-enum.pl make-action-name-func.pl make-keymap.pl make-mainmenu.pl
pkgdata_DATA = le.hlp

action-enum.h: action-name-func make-action-enum.pl
	$(PERL) $(srcdir)/make-action-enum.pl $(srcdir)/action-name-func > $@
action-name-func.h: action-name-func make-action-name-func.pl
	$(PERL) $(srcdir)/make-action-name-func.pl $(srcdir)/action-name-func > $@
keymap-default.h: $(top_srcdir)/misc/keymap-default make-keymap.pl
	$(PERL) $(srcdir)/make-keymap.pl $(top_srcdir)/misc/keymap-default > $@
mainmenu-default.h: $(top_srcdir)/misc/mainmenu-default make-mainmenu.pl
	$(PERL) $(srcdir)/make-mainmenu.pl $(top_srcdir)/misc/mainmenu-default > $@

block.o chset.o edit.o file.o format.o frames.o getstr.o help.o kern.o \
 keymap.o keymapdf.o keymapfn.o loadsave.o mainmenu.o menu1.o menu.o \
 options.o search.o user.o: action-enum.h
keymap.o: action-name-func.h
keymapdf.o: keymap-default.h
mainmenu.o: mainmenu-default.h

CLEANFILES = action-enum.h action-name-func.h keymap-default.h mainmenu-default.h
