/*
 * Copyright (c) 1993-2004 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*_________________________________________________________________________
**
** File:    keymapdf.cc
**
** Desc:    Default keyboard mapping
**_________________________________________________________________________
*/

#include <config.h>
#include <stdio.h>
#include "keymap.h"

ActionCodeRec  DefaultActionCodeTable[]=
{
   {CHAR_LEFT,"$kcub1"},
   {CHAR_RIGHT,"$kcuf1"},
   {WORD_LEFT,"\033|$kcub1"},
   {WORD_LEFT,"$kLFT"},
   {WORD_RIGHT,"\033|$kcuf1"},
   {WORD_RIGHT,"$kRIT"},
   {LINE_BEGIN,"$kbeg"},
   {LINE_BEGIN,"$khome"},
   {LINE_BEGIN,"$ka1"},
   {LINE_BEGIN,"\033|^"},
   {LINE_BEGIN,"\033|a"},
   {LINE_BEGIN,"\033|A"},
   {LINE_END,"$kend"},
   {LINE_END,"$kc1"},
   {LINE_END,"$kll"},
   {LINE_END,"\033|\\$"},
   {LINE_END,"\033|e"},
   {LINE_END,"\033|E"},
   {TEXT_BEGIN,"$kBEG"},
   {TEXT_BEGIN,"$kHOM"},
   {TEXT_BEGIN,"\033|$kbeg"},
   {TEXT_BEGIN,"\033|$khome"},
   {TEXT_BEGIN,"\007|B"},
   {TEXT_BEGIN,"\007|^B"},
   {TEXT_BEGIN,"\007|b"},
   {TEXT_END,"$kEND"},
   {TEXT_END,"\033|$kend"},
   {TEXT_END,"\007|E"},
   {TEXT_END,"\007|^E"},
   {TEXT_END,"\007|e"},
   {NEXT_PAGE,"$knp"},
   {NEXT_PAGE,"$kc3"},
   {NEXT_PAGE,"$knxt"},
   {NEXT_PAGE,"\033|$kcud1"},
   {PREV_PAGE,"$kpp"},
   {PREV_PAGE,"$ka3"},
   {PREV_PAGE,"$kprv"},
   {PREV_PAGE,"\033|$kcuu1"},
   {PAGE_TOP,"\033|$kpp"},
   {PAGE_TOP,"$kPRV"},
   {PAGE_BOTTOM,"\033|$knp"},
   {PAGE_BOTTOM,"$kNXT"},
   {TO_LINE_NUMBER,"\007|G"},
   {TO_LINE_NUMBER,"\007|\007"},
   {TO_LINE_NUMBER,"\007|g"},
   {TO_LINE_NUMBER,"\007|l"},
   {TO_LINE_NUMBER,"\007|L"},
   {TO_LINE_NUMBER,"$kf8"},
   {TO_LINE_NUMBER,"\033|8"},
   {TO_OFFSET,"\007|O"},
   {TO_OFFSET,"\007|\017"},
   {TO_OFFSET,"\007|o"},
   {TO_PREVIOUS_LOC,"\007|P"},
   {TO_PREVIOUS_LOC,"\007|\020"},
   {TO_PREVIOUS_LOC,"\007|p"},
   {LINE_UP,"$kcuu1"},
   {LINE_DOWN,"$kcud1"},

// Movement with block marking (new xterm codes)
   {MARK_CHAR_LEFT,"\033O2D"},
   {MARK_CHAR_RIGHT,"\033O2C"},
   {MARK_WORD_LEFT,"\033O6D"},
   {MARK_WORD_RIGHT,"\033O6C"},
   {MARK_LINE_BEGIN,"\033O2H"},
   {MARK_LINE_END,"\033O2F"},
   {MARK_TEXT_BEGIN,"\033O6H"},
   {MARK_TEXT_END,"\033O6F"},
   {MARK_NEXT_PAGE,"\033[6;2~"},
   {MARK_PREV_PAGE,"\033[5;2~"},
   {MARK_PAGE_TOP,"\033[5;6~"},
   {MARK_PAGE_BOTTOM,"\033[6;6~"},
   {MARK_LINE_UP,"\033O2A"},
   {MARK_LINE_DOWN,"\033O2B"},

// Movement with block marking (even newer xterm codes)
   {MARK_CHAR_LEFT,"\033[1;2D"},
   {MARK_CHAR_RIGHT,"\033[1;2C"},
   {MARK_WORD_LEFT,"\033[1;6D"},
   {MARK_WORD_RIGHT,"\033[1;6C"},
   {MARK_LINE_BEGIN,"\033[1;2H"},
   {MARK_LINE_END,"\033[1;2F"},
   {MARK_TEXT_BEGIN,"\033[1;6H"},
   {MARK_TEXT_END,"\033[1;6F"},
   {MARK_LINE_UP,"\033[1;2A"},
   {MARK_LINE_DOWN,"\033[1;2B"},
   {WORD_LEFT,"\033[1;5D"},
   {WORD_RIGHT,"\033[1;5C"},

// Delete actions
   {BACKSPACE_CHAR,"^H"},
   {DELETE_CHAR,"^D"},
   {DELETE_CHAR,"\177"},
   {BACKSPACE_CHAR,"$kbs"},
   {DELETE_CHAR,"$kdch1"},
   {FORWARD_DELETE_WORD,"\033|$kdch1"},
   {FORWARD_DELETE_WORD,"$kDC"},
   {DELETE_WORD,"^W"},
   {BACKWARD_DELETE_WORD,"\033|$kbs"},
   {FORWARD_DELETE_WORD,"\033|\177"},
   {BACKWARD_DELETE_WORD,"\033|^H"},
   {DELETE_TO_EOL,"\013"},
   {DELETE_TO_EOL,"$kel"},
   {DELETE_LINE,"$kdl"},
   {DELETE_LINE,"\031"},
   {UNDELETE,"^U"},
   {UNDELETE,"$kund"},

// Insert actions
//   {INDENT,"indent"},
//   {UNINDENT,UserUnindent,"unindent"},
   {NEWLINE,"^J"},
   {NEWLINE,"^M"},
   {COPY_FROM_UP,"^T"},
   {COPY_FROM_DOWN,"^V"},

   {UNDO,"^Z"},
   {REDO,"\033|Z"},
   {REDO,"\033|z"},

// File ops
   {LOAD_FILE,"$kf3"},
   {LOAD_FILE,"\033|3"},
   {SWITCH_FILE,"\033|$kf3"},
   {SWITCH_FILE,"$1kf3"},
   {SAVE_FILE,"$kf2"},
   {SAVE_FILE,"\033|2"},
   {SAVE_FILE_AS,"\033|$kf2"},
   {SAVE_FILE_AS,"$1kf2"},
//   {FILE_INFO,"file-info"},

// Block ops
   {COPY_BLOCK,"$kf11"},
   {COPY_BLOCK,"${kf4}C"},
   {COPY_BLOCK,"${kf4}c"},
   {MOVE_BLOCK,"$kf12"},
   {MOVE_BLOCK,"${kf4}M"},
   {MOVE_BLOCK,"${kf4}m"},
   {DELETE_BLOCK,"${kf4}D"},
   {DELETE_BLOCK,"${kf4}d"},
   {DELETE_BLOCK,"$1kf12"},
   {DELETE_BLOCK,"\033|$kf12"},
   {DELETE_BLOCK,"\033[3;2~"},
   {SET_BLOCK_END,"$kf6"},
   {SET_BLOCK_END,"\033|6"},
   {SET_BLOCK_END,"${kf4}E"},
   {SET_BLOCK_END,"${kf4}e"},
   {SET_BLOCK_BEGIN,"$kf5"},
   {SET_BLOCK_BEGIN,"\033|5"},
   {SET_BLOCK_BEGIN,"${kf4}B"},
   {SET_BLOCK_BEGIN,"${kf4}b"},
   {READ_BLOCK,"${kf4}R"},
   {READ_BLOCK,"${kf4}r"},
   {WRITE_BLOCK,"${kf4}W"},
   {WRITE_BLOCK,"${kf4}w"},
   {PIPE_BLOCK,"\033|\\|"},
   {PIPE_BLOCK,"${kf4}\\|"},
   {INDENT_BLOCK,"${kf4}I"},
   {INDENT_BLOCK,"${kf4}i"},
   {UNINDENT_BLOCK,"${kf4}U"},
   {UNINDENT_BLOCK,"${kf4}u"},
   {INSERT_PREFIX,"${kf4}>"},
   {INSERT_PREFIX,"\033|>"},
   {INSERT_PREFIX,"\033|."},
   {TO_UPPER,"${kf4}P"},
   {TO_UPPER,"${kf4}p"},
   {TO_LOWER,"${kf4}L"},
   {TO_LOWER,"${kf4}l"},
   {EXCHANGE_CASE,"${kf4}X"},
   {EXCHANGE_CASE,"${kf4}x"},
   {BLOCK_HIDE,"${kf4}H"},
   {BLOCK_HIDE,"${kf4}h"},
   {BLOCK_TYPE,"${kf4}T"},
   {BLOCK_TYPE,"${kf4}t"},
   {BLOCK_FUNC_BAR,"$kf4"},
   {BLOCK_FUNC_BAR,"\033|4"},
   {MARK_LINE,"\033|$kf5"},
   {MARK_LINE,"$1kf5"},
   {MARK_TO_EOL,"\033|$kf6"},
   {MARK_TO_EOL,"$1kf6"},
   {YANK_BLOCK,"$1kf11"},
   {YANK_BLOCK,"\033|$kf11"},
   {YANK_BLOCK,"${kf4}Y"},
   {YANK_BLOCK,"${kf4}y"},
   {YANK_BLOCK,"\033[2;2~"},
   {REMEMBER_BLOCK,"$kIC"},
   {START_DRAG_MARK,"${kf4}V"},
   {START_DRAG_MARK,"${kf4}v"},
   {MARK_ALL,"${kf4}A"},
   {MARK_ALL,"${kf4}a"},

// Search
   {SEARCH_FORWARD,"$kf7"},
   {SEARCH_FORWARD,"\033|7"},
   {SEARCH_FORWARD,"^F"},
   {SEARCH_BACKWARD,"$3kf7"},
   {SEARCH_BACKWARD,"^B"},
   {START_REPLACE,"^R"},
   {START_REPLACE,"$2kf7"},
   {CONT_SEARCH,"^C"},
   {CONT_SEARCH,"$1kf7"},
   {FIND_MATCH_BRACKET,"^]"},
   {FIND_BLOCK_BEGIN,"$2kf5"},
   {FIND_BLOCK_END,"$2kf6"},

// Format
//   {FORMAT_ONE_PARA,FormatPara,"format-paragraph"},
//   {FORMAT_DOCUMENT,FormatAll,"format-document"},
//   {CENTER_LINE,CenterLine,"center-line"},
//   {AJUST_RIGHT_LINE,ShiftRightLine,"ajust-right-line"}
   {FORMAT_FUNC_BAR,"\033|$kf4"},
   {FORMAT_FUNC_BAR,"$1kf4"},
   {FORMAT_FUNC_BAR,"\033|f"},
   {FORMAT_FUNC_BAR,"\033|F"},

// Others
//   {CALCULATOR,editcalc,"calculator"},
//   {DRAW_FRAMES,DrawFrames,"draw-frames"},
//   {TABS_EXPAND,ExpandAllTabs,"expand-tabs"},
//   {TEXT_OPTIMIZE,UserOptimizeText,"optimize-text"},
   {CHOOSE_CHAR,"\005"},
//   {UNIX_DOS_TRANSFORM,DOS_UNIX,"change-text-type"},

// Options
   {EDITOR_OPTIONS,"\017"},
//   {TERMINAL_OPTIONS,TermOpt,"terminal-options"},
//   {FORMAT_OPTIONS,FormatOptions,"format-options"},
//   {APPEARENCE_OPTIONS,AppearOpt,"appearence-options"},
//   {SAVE_OPTIONS,UpdtOpt,"save-options"},
//   {SAVE_OPTIONS_LOCAL,SaveOpt,"save-options-local"},

   {ENTER_CONTROL_CHAR,"\020"},
//   {ENTER_CHAR_CODE,UserEnterCharCode},
   {ENTER_WCHAR_CODE,"\033|w"},
   {ENTER_WCHAR_CODE,"\033|W"},

//   WINDOW_RESIZE,

   {EDITOR_HELP,"$kf1"},
   {EDITOR_HELP,"\033|1"},
   {CONTEXT_HELP,"\033|$kf1"},
   {CONTEXT_HELP,"$2kf1"},

//   {SUSPEND_EDITOR,SuspendEditor,"suspend-editor"},
   {QUIT_EDITOR,"\030"},
   {QUIT_EDITOR,"$kext"},
   {QUIT_EDITOR,"$kcan"},
   {QUIT_EDITOR,"\033|\033"},

   {COMPILE_CMD,"$3kf9"},
   {COMPILE_CMD,"\033|$kf9"},
   {MAKE_CMD,"$kf9"},
   {MAKE_CMD,"\033|9"},
   {RUN_CMD,"$2kf9"},
   {SHELL_CMD,"$1kf9"},
   {ONE_SHELL_CMD,"\033!"},

   {COMMENT_LINE,"\033|/"},

   {REFRESH_SCREEN,"\014"},
   {REFRESH_SCREEN,"$krfr"},

   {ENTER_MENU,"$kf10"},
   {ENTER_MENU,"\033|0"},
   {ENTER_MENU,"^N"},

   {SWITCH_INSERT_MODE,"$kich1"},
   {SWITCH_INSERT_MODE,"^A|I"},
   {SWITCH_INSERT_MODE,"^A|^I"},
   {SWITCH_INSERT_MODE,"^A|i"},
   {SWITCH_HEX_MODE,"^A|H"},
   {SWITCH_HEX_MODE,"^A|h"},
   {SWITCH_HEX_MODE,"^A|^H"},
   {SWITCH_AUTOINDENT_MODE,"^A|^A"},
   {SWITCH_AUTOINDENT_MODE,"^A|A"},
   {SWITCH_AUTOINDENT_MODE,"^A|a"},
   {SWITCH_RUSSIAN_MODE,"^A|^R"},
   {SWITCH_RUSSIAN_MODE,"^A|R"},
   {SWITCH_RUSSIAN_MODE,"^A|r"},
   {SWITCH_TEXT_MODE,"^A|^T"},
   {SWITCH_TEXT_MODE,"^A|T"},
   {SWITCH_TEXT_MODE,"^A|t"},
   {SWITCH_GRAPH_MODE,"^A|^G"},
   {SWITCH_GRAPH_MODE,"^A|G"},
   {SWITCH_GRAPH_MODE,"^A|g"},

   {SET_BOOKMARK,"\033|m"},
   {SET_BOOKMARK,"\033|M"},
   {GO_BOOKMARK,"\033|'"},

#ifdef __MSDOS__
   {PAGE_TOP,"\200\204"},
   {PAGE_BOTTOM,"\200v"},
#endif

   {-1,NULL}
};
