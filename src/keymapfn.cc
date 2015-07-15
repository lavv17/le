/*
 * Copyright (c) 1993-2000 by Alexander V. Lukyanov (lav@yars.free.net)
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

#include <config.h>
#include <stdio.h>
#include <errno.h>
#include "edit.h"
#include "block.h"
#include "options.h"
#include "keymap.h"
#include "format.h"
#include "search.h"
#include "colormnu.h"
#include "efopen.h"

ActionProcRec  EditorActionProcTable[]=
{
// Movement
   {CHAR_LEFT,UserCharLeft},
   {CHAR_RIGHT,UserCharRight},
   {WORD_LEFT,UserWordLeft},
   {WORD_RIGHT,UserWordRight},
   {LINE_BEGIN,UserLineBegin},
   {LINE_END,UserLineEnd},
   {TEXT_BEGIN,UserFileBegin},
   {TEXT_END,UserFileEnd},
   {NEXT_PAGE,UserPageDown},
   {PREV_PAGE,UserPageUp},
   {PAGE_TOP,UserPageTop},
   {PAGE_BOTTOM,UserPageBottom},
   {TO_LINE_NUMBER,UserToLineNumber},
   {TO_OFFSET,UserToOffset},
   {TO_PREVIOUS_LOC,UserPreviousEdit},
   {LINE_UP,UserLineUp},
   {LINE_DOWN,UserLineDown},

// Movement with block marking
   {MARK_CHAR_LEFT,UserMarkCharLeft},
   {MARK_CHAR_RIGHT,UserMarkCharRight},
   {MARK_WORD_LEFT,UserMarkWordLeft},
   {MARK_WORD_RIGHT,UserMarkWordRight},
   {MARK_LINE_BEGIN,UserMarkLineBegin},
   {MARK_LINE_END,UserMarkLineEnd},
   {MARK_TEXT_BEGIN,UserMarkFileBegin},
   {MARK_TEXT_END,UserMarkFileEnd},
   {MARK_NEXT_PAGE,UserMarkPageDown},
   {MARK_PREV_PAGE,UserMarkPageUp},
   {MARK_PAGE_TOP,UserMarkPageTop},
   {MARK_PAGE_BOTTOM,UserMarkPageBottom},
   {MARK_LINE_UP,UserMarkLineUp},
   {MARK_LINE_DOWN,UserMarkLineDown},

// Delete actions
   {DELETE_CHAR,UserDeleteChar},
   {BACKSPACE_CHAR,UserBackSpace},
   {DELETE_WORD,UserDeleteWord},
   {FORWARD_DELETE_WORD,UserForwardDeleteWord},
   {BACKWARD_DELETE_WORD,UserBackwardDeleteWord},
   {DELETE_TO_EOL,UserDeleteToEol},
   {DELETE_LINE,UserDeleteLine},
   {UNDELETE,UserUndelete},

// Insert actions
   {INDENT,UserIndent},
   {UNINDENT,UserUnindent},
   {NEWLINE,UserNewLine},
   {COPY_FROM_UP,UserCopyFromUp},
   {COPY_FROM_DOWN,UserCopyFromDown},

// Undo/redo
   {UNDO,UserUndo},
   {REDO,UserRedo},
   {UNDO_STEP,UserUndoStep},
   {REDO_STEP,UserUndoStep},

// File ops
   {LOAD_FILE,UserLoad},
   {SWITCH_FILE,UserSwitch},
   {REOPEN_FILE_RW,(void(*)())ReopenRW},
   {SAVE_FILE,(void(*)())UserSave},
   {SAVE_FILE_AS,(void(*)())UserSaveAs},
   {FILE_INFO,UserInfo},

// Block ops
   {COPY_BLOCK,UserCopyBlock},
   {MOVE_BLOCK,UserMoveBlock},
   {DELETE_BLOCK,UserDeleteBlock},
   {SET_BLOCK_END,UserSetBlockEnd},
   {SET_BLOCK_BEGIN,UserSetBlockBegin},
   {READ_BLOCK,Read},
   {WRITE_BLOCK,Write},
   {PIPE_BLOCK,UserPipeBlock},
   {INDENT_BLOCK,Indent},
   {UNINDENT_BLOCK,Unindent},
   {INSERT_PREFIX,UserBlockPrefixIndent},
   {TO_UPPER,ConvertToUpper},
   {TO_LOWER,ConvertToLower},
   {EXCHANGE_CASE,ExchangeCases},
   {BLOCK_HIDE,HideDisplay},
   {BLOCK_TYPE,BlockType},
   {BLOCK_FUNC_BAR,BlockFunc},
   {MARK_LINE,UserMarkLine},
   {MARK_TO_EOL,UserMarkToEol},
   {MARK_ALL,UserMarkAll},
   {START_DRAG_MARK,UserStartDragMark},
   {YANK_BLOCK,UserYankBlock},
   {REMEMBER_BLOCK,UserRememberBlock},

// Search
   {SEARCH_FORWARD,StartSearch},
   {SEARCH_BACKWARD,StartSearchBackward},
   {START_REPLACE,StartReplace},
   {CONT_SEARCH,ContSearch},
   {FIND_MATCH_BRACKET,FindMatch},
   {FIND_BLOCK_BEGIN,UserFindBlockBegin},
   {FIND_BLOCK_END,UserFindBlockEnd},

// Format
   {FORMAT_ONE_PARA,FormatPara},
   {FORMAT_DOCUMENT,FormatAll},
   {CENTER_LINE,CenterLine},
   {ADJUST_RIGHT_LINE,ShiftRightLine},
   {FORMAT_FUNC_BAR,FormatFunc},

// Others
   {CALCULATOR,editcalc},
   {DRAW_FRAMES,DrawFrames},
   {TABS_EXPAND,ExpandAllTabs},
   {SPAN_TABS_EXPAND,ExpandSpanTabs},
   {TEXT_OPTIMIZE,UserOptimizeText},
   {CHOOSE_CHAR,UserChooseChar},
   {CHOOSE_WCHAR,UserChooseWChar},
   {CHOOSE_BYTE,UserChooseByte},
   {UNIX_DOS_TRANSFORM,DOS_UNIX},

// Options
   {EDITOR_OPTIONS,Options},
   {TERMINAL_OPTIONS,TermOpt},
   {FORMAT_OPTIONS,FormatOptions},
   {APPEARANCE_OPTIONS,AppearOpt},
   {PROGRAM_OPTIONS,ProgOpt},
   {UNDO_OPTIONS,UndoOpt},
   {COLOR_TUNING,ColorsOpt},
   {SAVE_OPTIONS,UpdtOpt},
   {SAVE_OPTIONS_LOCAL,SaveOpt},

   {ENTER_CONTROL_CHAR,UserEnterControlChar},
   {ENTER_CHAR_CODE,UserInsertCharCode},
   {ENTER_WCHAR_CODE,UserInsertWCharCode},
   {ENTER_BYTE_CODE,UserInsertByteCode},

//   WINDOW_RESIZE,

   {EDITOR_HELP,UserKeysHelp},
   {CONTEXT_HELP,UserWordHelp},

   {SUSPEND_EDITOR,SuspendEditor},
   {QUIT_EDITOR,Quit},

   {COMPILE_CMD,DoCompile},
   {MAKE_CMD,DoMake},
   {RUN_CMD,DoRun},
   {SHELL_CMD,DoShell},
   {ONE_SHELL_CMD,UserShellCommand},

   {COMMENT_LINE,UserCommentLine},

   {ENTER_MENU,UserMenu},

   {REFRESH_SCREEN,UserRefreshScreen},

   {SWITCH_INSERT_MODE,UserSwitchInsertMode},
   {SWITCH_HEX_MODE,UserSwitchHexMode},
   {SWITCH_AUTOINDENT_MODE,UserSwitchAutoindentMode},
   {SWITCH_RUSSIAN_MODE,UserSwitchRussianMode},
   {SWITCH_TEXT_MODE,UserSwitchTextMode},
   {SWITCH_GRAPH_MODE,UserSwitchGraphMode},

   {EDIT_CHARSET,edit_chset},
   {SET_CHARSET_8BIT,set_chset_8bit},
   {SET_CHARSET_8BIT_NO_CONTROL,set_chset_8bit_noctrl},
   {SAVE_TERMINAL_OPTIONS,SaveTermOpt},
   {EDIT_COLORS,ColorsOpt},
   {SAVE_COLORS,ColorsSave},
   {SAVE_COLORS_FOR_TERM,ColorsSaveForTerminal},
   {LOAD_COLOR_DEFAULT,LoadColorDefault},
   {LOAD_COLOR_DEFBG,LoadColorDefaultBG},
   {LOAD_COLOR_BLACK,LoadColorBlack},
   {LOAD_COLOR_BLUE,LoadColorBlue},
   {LOAD_COLOR_GREEN,LoadColorGreen},
   {LOAD_COLOR_WHITE,LoadColorWhite},
   {PROGRAMS_OPTIONS,ProgOpt},
   {ABOUT,UserAbout},
   {LOAD_KEYMAP_DEFAULT,LoadKeymapDefault},
   {LOAD_KEYMAP_EMACS,  LoadKeymapEmacs},
   {SAVE_KEYMAP,SaveKeymap},
   {SAVE_KEYMAP_FOR_TERM,SaveKeymapForTerminal},

   {SET_BOOKMARK,UserSetBookmark},
   {SET_BOOKMARK_0,UserSetBookmark0},
   {SET_BOOKMARK_1,UserSetBookmark1},
   {SET_BOOKMARK_2,UserSetBookmark2},
   {SET_BOOKMARK_3,UserSetBookmark3},
   {SET_BOOKMARK_4,UserSetBookmark4},
   {SET_BOOKMARK_5,UserSetBookmark5},
   {SET_BOOKMARK_6,UserSetBookmark6},
   {SET_BOOKMARK_7,UserSetBookmark7},
   {SET_BOOKMARK_8,UserSetBookmark8},
   {SET_BOOKMARK_9,UserSetBookmark9},
   {GO_BOOKMARK,UserGoBookmark},
   {GO_BOOKMARK_0,UserGoBookmark0},
   {GO_BOOKMARK_1,UserGoBookmark1},
   {GO_BOOKMARK_2,UserGoBookmark2},
   {GO_BOOKMARK_3,UserGoBookmark3},
   {GO_BOOKMARK_4,UserGoBookmark4},
   {GO_BOOKMARK_5,UserGoBookmark5},
   {GO_BOOKMARK_6,UserGoBookmark6},
   {GO_BOOKMARK_7,UserGoBookmark7},
   {GO_BOOKMARK_8,UserGoBookmark8},
   {GO_BOOKMARK_9,UserGoBookmark9},

   {-1,NULL}
};

void  EditorReadKeymap()
{
#ifdef EMBED_DATADIR
   char fn[strlen(HOME)+1+3+1+7+strlen(TERM)+1];
   FILE *f;
   sprintf(fn,"%s/.le/keymap-%s",HOME,TERM); f=fopen(fn,"r");
   if(!f) { sprintf(fn,"keymap-%s",TERM); f=efopen(fn,"r"); }
   if(!f) { sprintf(fn,"%s/.le/keymap",HOME); f=fopen(fn,"r"); }
   if(!f) { strcpy(fn,"keymap"); f=efopen(fn,"r"); }
#else
   char fn[strlen(PKGDATADIR)+strlen(HOME)+1+3+1+7+strlen(TERM)+1];
   FILE *f;
   sprintf(fn,"%s/.le/keymap-%s",HOME,TERM); f=fopen(fn,"r");
   if(!f) { sprintf(fn,"%s/keymap-%s",PKGDATADIR,TERM); f=fopen(fn,"r"); }
   if(!f) { sprintf(fn,"%s/.le/keymap",HOME); f=fopen(fn,"r"); }
   if(!f) { sprintf(fn,"%s/keymap",PKGDATADIR); f=fopen(fn,"r"); }
#endif
   if(!f)
      return;
   
   errno=0;
   ReadActionMap(f);
   if(errno)
   {
      FError(fn);
   }
   fclose(f);
}

void LoadKeymap(const char *name)
{
#ifdef EMBED_DATADIR
   char fn[strlen(name)+1];
   strcpy(fn,name);
   FILE *f=efopen(fn,"r");
#else
   char fn[strlen(PKGDATADIR)+1+strlen(name)+1];
   sprintf(fn,"%s/%s",PKGDATADIR,name);
   FILE *f=fopen(fn,"r");
#endif
   if(!f)
   {
      FError(fn);
      return;
   }
   ReadActionMap(f);
   fclose(f);
   RebuildKeyTree();
   LoadMainMenu();
}

void LoadKeymapEmacs()
{
   LoadKeymap("keymap-emacs");
}
void LoadKeymapDefault()
{
   FreeActionCodeTable();
   ActionCodeTable=DefaultActionCodeTable;
   RebuildKeyTree();
   LoadMainMenu();
}
void SaveKeymap()
{
   char  filename[strlen(HOME)+1+3+1+6+1];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap",HOME);
   f=fopen(filename,"w");
   if(!f)
   {
      FError(filename);
      return;
   }
   WriteActionMap(f);
   fclose(f);
}
void SaveKeymapForTerminal()
{
   char  filename[strlen(HOME)+1+3+1+7+strlen(TERM)+1];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap-%s",HOME,TERM);
   f=fopen(filename,"w");
   if(!f)
   {
      FError(filename);
      return;
   }
   WriteActionMap(f);
   fclose(f);
}
