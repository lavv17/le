/*
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
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

#ifndef KEYMAP_H
#define KEYMAP_H

enum  Action
{
// Movement actions
   CHAR_LEFT=1024,
   CHAR_RIGHT,
   WORD_LEFT,
   WORD_RIGHT,
   LINE_BEGIN,
   LINE_END,
   TEXT_BEGIN,
   TEXT_END,
   NEXT_PAGE,
   PREV_PAGE,
   PAGE_TOP,
   PAGE_BOTTOM,
   TO_LINE_NUMBER,
   TO_OFFSET,
   TO_PREVIOUS_LOC,
   LINE_UP,
   LINE_DOWN,

// Movement with block marking
   MARK_CHAR_LEFT,
   MARK_CHAR_RIGHT,
   MARK_WORD_LEFT,
   MARK_WORD_RIGHT,
   MARK_LINE_BEGIN,
   MARK_LINE_END,
   MARK_TEXT_BEGIN,
   MARK_TEXT_END,
   MARK_NEXT_PAGE,
   MARK_PREV_PAGE,
   MARK_PAGE_TOP,
   MARK_PAGE_BOTTOM,
   MARK_LINE_UP,
   MARK_LINE_DOWN,

// Delete actions
   DELETE_CHAR,
   BACKSPACE_CHAR,
   DELETE_WORD,
   BACKWARD_DELETE_WORD,
   FORWARD_DELETE_WORD,
   DELETE_TO_EOL,
   DELETE_LINE,
   UNDELETE,

// Insert actions
   INDENT,
   UNINDENT,
   AUTOINDENT,
   NEWLINE,
   COPY_FROM_UP,
   COPY_FROM_DOWN,

// Undo/redo
   UNDO,
   REDO,
   UNDO_STEP,
   REDO_STEP,

// File ops
   LOAD_FILE,
   SWITCH_FILE,
   REOPEN_FILE_RW,
   SAVE_FILE,
   SAVE_FILE_AS,
   FILE_INFO,

// Block ops
   COPY_BLOCK,
   MOVE_BLOCK,
   DELETE_BLOCK,
   SET_BLOCK_END,
   SET_BLOCK_BEGIN,
   READ_BLOCK,
   WRITE_BLOCK,
   PIPE_BLOCK,
   INDENT_BLOCK,
   UNINDENT_BLOCK,
   INSERT_PREFIX,
   TO_UPPER,
   TO_LOWER,
   EXCHANGE_CASE,
   BLOCK_HIDE,
   BLOCK_TYPE,
   BLOCK_FUNC_BAR,
   MARK_LINE,
   MARK_TO_EOL,
   MARK_ALL,
   START_DRAG_MARK,
   YANK_BLOCK,
   REMEMBER_BLOCK,

// Search
   SEARCH_FORWARD,
   SEARCH_BACKWARD,
   START_REPLACE,
   CONT_SEARCH,
   FIND_MATCH_BRACKET,
   FIND_BLOCK_BEGIN,
   FIND_BLOCK_END,

// Format
   FORMAT_ONE_PARA,
   FORMAT_DOCUMENT,
   CENTER_LINE,
   ADJUST_RIGHT_LINE,
   FORMAT_FUNC_BAR,

// Others
   CALCULATOR,
   DRAW_FRAMES,
   TABS_EXPAND,
   TEXT_OPTIMIZE,
   CHOOSE_CHAR,
   CHOOSE_WCHAR,
   CHOOSE_BYTE,
   UNIX_DOS_TRANSFORM,

// Options
   EDITOR_OPTIONS,
   TERMINAL_OPTIONS,
   FORMAT_OPTIONS,
   APPEARENCE_OPTIONS,
   PROGRAM_OPTIONS,
   UNDO_OPTIONS,
   COLOR_TUNING,
   SAVE_OPTIONS,
   SAVE_OPTIONS_LOCAL,

   ENTER_CONTROL_CHAR,
   ENTER_CHAR_CODE,
   ENTER_WCHAR_CODE,
   ENTER_BYTE_CODE,

   WINDOW_RESIZE,

   EDITOR_HELP,
   CONTEXT_HELP,

   SUSPEND_EDITOR,
   QUIT_EDITOR,
#define  CANCEL   QUIT_EDITOR

   COMPILE_CMD,
   MAKE_CMD,
   RUN_CMD,
   SHELL_CMD,
   ONE_SHELL_CMD,

   COMMENT_LINE,
   REFRESH_SCREEN,

   ENTER_MENU,

   SWITCH_INSERT_MODE,
   SWITCH_HEX_MODE,
   SWITCH_AUTOINDENT_MODE,
   SWITCH_RUSSIAN_MODE,
   SWITCH_TEXT_MODE,
   SWITCH_GRAPH_MODE,

   EDIT_CHARSET,
   SET_CHARSET_8BIT,
   SET_CHARSET_8BIT_NO_CONTROL,
   SAVE_TERMINAL_OPTIONS,
   EDIT_COLORS,
   SAVE_COLORS,
   SAVE_COLORS_FOR_TERM,
   LOAD_COLOR_DEFAULT,
   LOAD_COLOR_DEFBG,
   LOAD_COLOR_BLACK,
   LOAD_COLOR_BLUE,
   LOAD_COLOR_GREEN,
   LOAD_COLOR_WHITE,
   PROGRAMS_OPTIONS,
   ABOUT,
   LOAD_KEYMAP_DEFAULT,
   LOAD_KEYMAP_EMACS,
   SAVE_KEYMAP,
   SAVE_KEYMAP_FOR_TERM,

   SET_BOOKMARK,
   SET_BOOKMARK_0,
   SET_BOOKMARK_1,
   SET_BOOKMARK_2,
   SET_BOOKMARK_3,
   SET_BOOKMARK_4,
   SET_BOOKMARK_5,
   SET_BOOKMARK_6,
   SET_BOOKMARK_7,
   SET_BOOKMARK_8,
   SET_BOOKMARK_9,
   GO_BOOKMARK,
   GO_BOOKMARK_0,
   GO_BOOKMARK_1,
   GO_BOOKMARK_2,
   GO_BOOKMARK_3,
   GO_BOOKMARK_4,
   GO_BOOKMARK_5,
   GO_BOOKMARK_6,
   GO_BOOKMARK_7,
   GO_BOOKMARK_8,
   GO_BOOKMARK_9,

   MOUSE_ACTION,
   NO_ACTION
};

typedef  void  (*ActionProc)();

struct   ActionProcRec
{
   int         action;
   ActionProc  proc;
};

struct   ActionNameRec
{
   int   action;
   const char *name;
};

struct   ActionCodeRec
{
   int   action;
   char  *code;
};

extern unsigned char StringTyped[];
extern int   StringTypedLen;
extern ActionProcRec  EditorActionProcTable[];
extern const ActionCodeRec *ActionCodeTable;
extern const ActionCodeRec DefaultActionCodeTable[];

int   GetNextAction(void);
const char *GetActionString(int action);
void  ReadActionMap(FILE*);
void  WriteActionMap(FILE*);
ActionProc GetActionProc(ActionProcRec*,int action);
void  EditorReadKeymap();
void  RebuildKeyTree();
void  FreeActionCodeTable();

int   FindActionCode(const char *);

const char *ShortcutPrettyPrint(int c);

void LoadKeymapEmacs();
void LoadKeymapDefault();
void SaveKeymap();
void SaveKeymapForTerminal();

#endif /* KEYMAP_H */
