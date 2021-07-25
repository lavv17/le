install(FILES le.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)

install(FILES le-icon.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps)

install(FILES help DESTINATION ${CMAKE_INSTALL_PREFIX}/share/le)

install(FILES
    syntax.d/ada
    syntax.d/as
    syntax.d/awk
    syntax.d/c++-comment
    syntax.d/c-comment
    syntax.d/c-literal
    syntax.d/cpp
    syntax.d/eiffel
    syntax.d/fvwm2
    syntax.d/gema
    syntax.d/html
    syntax.d/java
    syntax.d/logrotate-conf
    syntax.d/lout
    syntax.d/make-file
    syntax.d/metafont
    syntax.d/named-conf
    syntax.d/pascal
    syntax.d/perl
    syntax.d/php
    syntax.d/python
    syntax.d/sather
    syntax.d/shell
    syntax.d/snmp-mib
    syntax.d/sql
    syntax.d/tcl
    syntax.d/tex
    syntax.d/texinfo
    syntax.d/txt2tags
    syntax.d/verilog
    syntax.d/javascript
    syntax.d/ocaml
    syntax.d/haskell
    syntax.d/golang
    syntax.d/nroff
    syntax.d/pod
    syntax.d/markdown
    syntax.d/css
    syntax.d/c
    syntax.d/c++
    syntax.d/puppet
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/le/syntax.d)
