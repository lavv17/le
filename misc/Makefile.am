
nobase_pkgdata_DATA = syntax colors-black colors-blue colors-white colors-defbg\
 colors-green mainmenu mainmenu-ru keymap-emacs\
 syntax.d/c-literal syntax.d/c-comment syntax.d/c++-comment\
 syntax.d/cpp syntax.d/fvwm2 syntax.d/lout

EXTRA_DIST   = $(pkgdata_DATA)

colors: ../src/le
	../src/le --dump-colors > $@
keymap: ../src/le
	../src/le --dump-keymap > $@

CLEANFILES = colors keymap
