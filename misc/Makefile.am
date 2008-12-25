
nobase_pkgdata_DATA = colors-* mainmenu* keymap-* syntax syntax.d/[a-z]*

EXTRA_DIST   = $(nobase_pkgdata_DATA)

colors: ../src/le
	../src/le --dump-colors > $@
keymap: ../src/le
	../src/le --dump-keymap > $@

CLEANFILES = colors keymap
