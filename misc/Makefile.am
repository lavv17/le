colors = colors-black colors-blue colors-defbg colors-green colors-white
keymaps = keymap-emacs keymap-default
mainmenus = mainmenu-default mainmenu-ru
syntax = syntax                 \
    syntax.d/ada                \
    syntax.d/as                 \
    syntax.d/awk                \
    syntax.d/c                  \
    syntax.d/c++                \
    syntax.d/c++-comment        \
    syntax.d/c-comment          \
    syntax.d/c-literal          \
    syntax.d/cpp                \
    syntax.d/css                \
    syntax.d/eiffel             \
    syntax.d/fvwm2              \
    syntax.d/gema               \
    syntax.d/golang             \
    syntax.d/haskell            \
    syntax.d/html               \
    syntax.d/java               \
    syntax.d/javascript         \
    syntax.d/jinja2             \
    syntax.d/logrotate-conf     \
    syntax.d/lout               \
    syntax.d/make-file          \
    syntax.d/markdown           \
    syntax.d/metafont           \
    syntax.d/named-conf         \
    syntax.d/nroff              \
    syntax.d/ocaml              \
    syntax.d/pascal             \
    syntax.d/perl               \
    syntax.d/php                \
    syntax.d/pod                \
    syntax.d/puppet             \
    syntax.d/python             \
    syntax.d/sather             \
    syntax.d/shell              \
    syntax.d/snmp-mib           \
    syntax.d/sql                \
    syntax.d/tcl                \
    syntax.d/tex                \
    syntax.d/texinfo            \
    syntax.d/txt2tags           \
    syntax.d/verilog            \
    syntax.d/yaml

nobase_pkgdata_DATA = $(colors) $(keymaps) $(mainmenus) $(syntax)
pkgdata_SCRIPTS = help

appdir = $(datadir)/applications
icondir = $(datadir)/icons/hicolor/48x48/apps

app_DATA = le.desktop
icon_DATA = le-icon.png

EXTRA_DIST = $(nobase_pkgdata_DATA) $(pkgdata_SCRIPTS) $(app_DATA) $(icon_DATA) \
    gnulib.patch

colors: ../src/le
	../src/le --dump-colors > $@

CLEANFILES = colors
