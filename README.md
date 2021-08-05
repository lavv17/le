LE text editor
===

LE has many block operations with stream and rectangular blocks, can edit
both unix and dos style files (LF/CRLF), is binary clean, has hex mode, can
edit text with multibyte character encoding, has full undo/redo, can edit
files and mmap'able devices in mmap shared mode (only replace), has tunable
syntax highlighting, tunable color scheme (can use default colors), tunable
key map, tunable menu. It is slightly similar to Norton Editor for DOS, but
has more features.

You can use either auto-tools or cmake to build from sources.

* Build with cmake:
```shell
    mkdir build
    cd build
    cmake ..
    make
    make install
```
* Build with auto-tools:
```shell
    ./autogen.sh
    mkdir build
    cd build
    ../configure
    make
    make install
```
