set(CMAKE_CXX_FLAGS "-O2 -Wall -Wwrite-strings -Woverloaded-virtual -Wno-invalid-source-encoding -fno-exceptions -fno-rtti -fno-implement-inlines")

add_definitions(-DVERSION="${CMAKE_PROJECT_VERSION}")
add_definitions(-DPKGDATADIR="${CMAKE_INSTALL_PREFIX}/share/le")

include(FindNcursesw.cmake)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/action-enum.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make-action-enum.pl
                ${CMAKE_CURRENT_SOURCE_DIR}/action-name-func
                > ${CMAKE_CURRENT_BINARY_DIR}/action-enum.h
    DEPENDS make-action-enum.pl action-name-func
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/action-name-func.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make-action-name-func.pl
                ${CMAKE_CURRENT_SOURCE_DIR}/action-name-func
                > ${CMAKE_CURRENT_BINARY_DIR}/action-name-func.h
    DEPENDS make-action-name-func.pl action-name-func
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/keymap-default.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make-keymap.pl
                ${CMAKE_SOURCE_DIR}/misc/keymap-default
                > ${CMAKE_CURRENT_BINARY_DIR}/keymap-default.h
    DEPENDS make-keymap.pl ../misc/keymap-default
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mainmenu-default.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make-mainmenu.pl
                ${CMAKE_SOURCE_DIR}/misc/mainmenu-default
                > ${CMAKE_CURRENT_BINARY_DIR}/mainmenu-default.h
    DEPENDS make-mainmenu.pl ../misc/mainmenu-default
)

include(make-config.cmake)

add_executable(le
    about.cc
    block.cc
    calc.cc
    chset.cc
    cmd.cc
    color.cc
    edit.cc
    editcalc.cc
    ext.cc
    file.cc
    format.cc
    frames.cc
    getch.cc
    getstr.cc
    help.cc
    history.cc
    kern.cc
    keymap.cc
    keymapdf.cc
    keymapfn.cc
    loadsave.cc
    mainmenu.cc
    menu.cc
    menu1.cc
    options.cc
    pipe.cc
    rus.cc
    screen.cc
    search.cc
    signals.cc
    tables.cc
    textpoin.cc
    user.cc
    window.cc
    highli.cc
    clipbrd.cc
    keynames.cc
    mouse.cc
    colormnu.cc
    bm.cc
    mb.cc
    undo.cc
    regex.c
    wcwidth.c
    wcwidth1.c
    ../gnulib/localcharset.c
    action-enum.h
    action-name-func.h
    keymap-default.h
    mainmenu-default.h
)
target_include_directories(le PUBLIC
    .
    ../lib
    ../gnulib
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CURSES_INCLUDE_DIRS}
)
target_link_libraries(le ${CURSES_LIBRARIES})
install(TARGETS le DESTINATION bin)

install(FILES le.hlp DESTINATION ${CMAKE_INSTALL_PREFIX}/share/le)
