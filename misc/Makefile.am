
pkgdata_DATA = syntax colors-black colors-blue colors-white colors-defbg\
 colors-green mainmenu mainmenu-ru keymap-emacs
EXTRA_DIST   = $(pkgdata_DATA)

colors: ../src/le
	../src/le --dump-colors > $@
keymap: ../src/le
	../src/le --dump-keymap > $@

CLEANFILES = colors keymap
