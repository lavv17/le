colors = colors-black colors-blue colors-defbg colors-green colors-white
keymaps = keymap-emacs
mainmenus = mainmenu mainmenu-ru
syntax = syntax syntax.d/CVS syntax.d/ada syntax.d/as syntax.d/awk	     \
 syntax.d/c++-comment syntax.d/c-comment syntax.d/c-literal syntax.d/cpp     \
 syntax.d/eiffel syntax.d/fvwm2 syntax.d/gema syntax.d/html syntax.d/java    \
 syntax.d/logrotate-conf syntax.d/lout syntax.d/make-file syntax.d/metafont  \
 syntax.d/named-conf syntax.d/pascal syntax.d/perl syntax.d/php		     \
 syntax.d/python syntax.d/sather syntax.d/shell syntax.d/snmp-mib	     \
 syntax.d/sql syntax.d/tcl syntax.d/tex syntax.d/texinfo syntax.d/txt2tags   \
 syntax.d/verilog


nobase_pkgdata_DATA = $(colors) $(keymaps) $(mainmenus) $(syntax)
EXTRA_DIST = $(nobase_pkgdata_DATA)

colors: ../src/le
	../src/le --dump-colors > $@
keymap: ../src/le
	../src/le --dump-keymap > $@

CLEANFILES = colors keymap
