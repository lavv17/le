/* 
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

/* ehelp.c : English help for the editor */

#include <config.h>
#include <stdio.h>

const char  *MainHelp=
   "The default keymap is described below:\n"
   "\n"
   "            Keys to move\n"
   "Up               - line up\n"
   "Down             - line down\n"
   "Left             - character left\n"
   "Right            - character right\n"
   "Home             - to begin of line\n"
   "End              - to end of line\n"
   "PgUp             - page up\n"
   "PgDn             - page down\n"
   "~Left            - word left\n"
   "~Right           - word right\n"
   "~Home | ^G+B     - to begin of file\n"
   "~End  | ^G+E     - to end of file\n"
   "F8    | ^G+G     - move to line number\n"
   "~F8   | ^G+O     - move to offset\n"
   "\n"
   "            Keys to delete\n"
   "Del | ^D         - delete char right\n"
   "BackSp | ^H      - delete char left\n"
   "^Y               - delete line\n"
   "^K               - delete to EOL\n"
   "^W               - delete word\n"
   "^U               - undelete\n"
   "\n"
   "                 Files\n"
   "F2               - save current file\n"
   "F3               - load another file\n"
   "~F3              - load previous file\n"
   "~F2              - save as...\n"
   "F9               - make\n"
   "~F9              - shell\n"
   "^F9              - run\n"
   "~^F9             - compile\n"
   "\n"
   "                Blocks\n"
   "F5 | F4+B        - set block begin\n"
   "F6 | F4+E        - set block end\n"
   "F4+H             - hide/display block\n"
   "F4+C | F11       - copy block\n"
   "F4+D             - delete block\n"
   "F4+M | F12       - move block\n"
   "F4+W             - write block\n"
   "F4+R             - read block\n"
   "F4+I             - indent block\n"
   "F4+U             - unindent block\n"
   "F4+T             - change block type\n"
   "F4+^B | ^F5      - find block begin\n"
   "F4+^E | ^F6      - find block end\n"
   "~F5              - mark line\n"
   "~F6              - mark to eol\n"
   "F4+P             - to upper case\n"
   "F4+L             - to lower case\n"
   "F4+X             - exchange cases\n"
   "F4+| | M-|       - pipe block through a\n"
   "                   command\n"
   "\n"
   "           Search & Replace\n"
   "F7 | ^F          - search forwards\n"
   "~^F7 | ^N        - search backwards\n"
   "~F7 | ^C         - cont. search/replace\n"
   "^F7 | ^R         - start replace\n"
   "Ctrl-]           - find matching bracket\n"
   "\n"
   "          Exit from the editor\n"
   "^X               - ask to save, then exit\n"
   "\n"
   "             Editor control\n"
   "^O               - start setting options\n"
   "^A+H             - switch HEX mode\n"
   "^A+T             - switch TEXT mode\n"
   "^A+A             - switch autoindent\n"
   "^A+I             - switch insert\n"
   "^A+R             - switch RUS/LAT\n"
   "\n"
   "                Options\n"
   "^O               - start setting options\n"
   "  Enter          - use the setup\n"
   "  F2             - save to current opt file\n"
   "  ~F2            - save to working directory\n"
   "  ^X             - restore old setup\n"
   "  Space          - change item\n"
   "  F1             - help\n"
   "\n"
   " F10 - enter main menu\n"
   "\n"
   "                Others\n"
   "^P               - put control character\n"
   "^E               - insert char with a code\n"
   "^L               - refresh screen\n"
   "^V               - copy char below\n"
   "^T               - copy char above\n"
   "~F4              - format functions\n"
   "^F1              - run 'help' program\n"
   "M-/              - comment line (C, C++)\n"
   "\n"
   "                 Note\n"
   "   Some of ^key, ~key and any M-key\n"
   "combinations can be typed as ESC+key\n"
;

const char    *setup_page1[]=
{
    "   Use arrows to navigate, space to",
    "change current option. You can also",
    "edit the strings and the numbers.",
    "",
    "   Enter    - use the options within",
    "              the current editing",
    "              session",
    "   Shift-F2 - save the options in",
    "              the current directory",
    "   F2       - update current options",
    "              file",
    "   ^X       - quit, do not use the new",
    "              options.",
NULL
};
const char    *setup_page2[]=
{
    "1. Insert. In this mode characters",
    "will be inserted before the character",
    "on the cursor and the cursor will be",
    "moved one position right.",
    "2. Autoindent. In this mode on",
    "pressing Enter a new line will be",
    "inserted and the cursor will be placed",
    "with the margin of the previous",
    "line.",
    "3. Save Positions. In this mode after",
    "loading a file cursor will be placed",
    "on its latest position in this file",
    "even if you had exited from the",
    "editor. Positions are saved in file",
    "'$HOME/.le/history'.",
NULL};
const char    *setup_page3[]=
{
    "4. Save History. In this mode you can",
    "choose a file name with arrows",
    "even if you had exited from the",
    "editor. History is saved in file",
    "'$HOME/.le/history'.",
    "5. Rectangle Blocks. There are two",
    "types of blocks: string and rectangle",
    "ones. A string block is a continuous",
    "sequence of characters, including new",
    "line and tab characters. A rectangle",
    "block is a rectangle array of",
    "characters. A special subtype of",
    "rectangle blocks is line block which",
    "is unlimited to the right. To mark it",
    "set begin and end at the same column.",
NULL};
const char    *setup_page4[]=
{
    "6. Make Backup. This option enables",
    "creating backup files with extension",
    "'.bak'. These are created in current",
    "or in specified directory.",
    "7. No Windows. This option disables",
    "windows. Most messages and questions",
    "will be written at the last line of",
    "the screen. It can make the editor",
    "faster on slow terminals.",
    "8. No Regular Expressions. This",
    "option disables using regular",
    "expressions. You will be able not to",
    "type '\\' before special characters.",
    "9. Latin. In this mode latin letters",
    "will be typed.",
NULL};
const char    *setup_page5[]={
    "10. Russian. In this mode russian",
    "letters will be typed.",
    "11. Graphic. In this mode graphic",
    "characters from extended ASCII will be",
    "typed.",
    "12. Exact. This mode will do for both",
    "text and binary files. Strings are",
    "right limited, that is you have to",
    "type a space to get right from EOL.",
    "13. Text. This mode is only for text",
    "files. Strings are not right limited.",
    "14. Hex. This is hexadesimal mode for",
    "binary files.",
NULL};
const char    **SetupHelp[]=
{
    setup_page1,
    setup_page2,
    setup_page3,
    setup_page4,
    setup_page5,
NULL};

const char    *block_page1[]=
{
    "Press:",
    "   B   to mark begin of block",
    "   E   to mark end of block",
    "   C   to copy block",
    "   M   to move block",
    "   D   to delete block",
    "   W   to write block to a file",
    "   R   to read block from a file",
    "   I   to indent block",
    "   U   to unindent block",
    "   T   to change block type",
    "   L   to convert block or characters",
    "to end of line to lower case",
    "   P   to convert to upper case",
    "   X   to exchange upper/lower cases",
NULL};
const char    *block_page2[]=
{
    "   H   to hide or unhide block",
    "or any other key to leave block",
    "functions",
    "   |   to pipe the block through a",
    "specified UNIX command (such as sed)",
NULL};
const char    **BlockHelp[]=
{
    block_page1,
    block_page2,
NULL};

const char    *calc_page1[]=
{
    "   This is a postfix calculator. For",
    "example, to calculate 2*2 you must",
    "type '2 2 *', when it is done stack",
    "will contain 4. Now to add 2 you may",
    "type '2 +', stack will contain 6, etc.",
    "Here are operations of the calculator:",
    "  + - * /  simplest arithmetics",
    "  %        take remainder of Y/X",
    "  sq       square X",
    "  sqr      take square root of X",
    "  **       rise X to power Y",
    "  ln       take natural logarithm",
    "  lg       take log10",
    "  pi,e     load constants",
    "  exp      take exponent of X",

NULL
};
const char    *calc_page2[]=
{
    "  del      delete number",
    "  clr      clear the stack",
    "  xy       swap X and Y",
    "  neg      take -X",
    "  rev      take 1/X",
    "  cp       copy X",
    "  sin,cos",
NULL
};
const char    **CalcHelp[]=
{
    calc_page1,
    calc_page2,
NULL
};

const char    *frames_page1[]=
{
   "   The editor is in frame-drawing",
   "mode now. You can use the following",
   "control keys:",
   "",
   "Cursor arrows - draw line",
   "Tab           - switch between single,",
   "                double and no line mode",
   "Any other key - leave the mode",
   "",
   "You can select graphic character set",
   "when modifying terminal options.",
   "Note: on some terminals there is no",
   "double lines.",
NULL
};
const char    **FramesHelp[]=
{
    frames_page1,
NULL
};
