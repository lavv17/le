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

// File ops
   {LOAD_FILE,UserLoad},
   {SWITCH_FILE,UserSwitch},
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
   {START_DRAG_MARK,UserStartDragMark},
   {YANK_BLOCK,UserYankBlock},

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
   {AJUST_RIGHT_LINE,ShiftRightLine},
   {FORMAT_FUNC_BAR,FormatFunc},

// Others
   {CALCULATOR,editcalc},
   {DRAW_FRAMES,DrawFrames},
   {TABS_EXPAND,ExpandAllTabs},
   {TEXT_OPTIMIZE,UserOptimizeText},
   {CHOOSE_CHAR,UserChooseChar},
   {UNIX_DOS_TRANSFORM,DOS_UNIX},

// Options
   {EDITOR_OPTIONS,Options},
   {TERMINAL_OPTIONS,TermOpt},
   {FORMAT_OPTIONS,FormatOptions},
   {APPEARENCE_OPTIONS,AppearOpt},
   {PROGRAM_OPTIONS,ProgOpt},
   {COLOR_TUNING,ColorsOpt},
   {SAVE_OPTIONS,UpdtOpt},
   {SAVE_OPTIONS_LOCAL,SaveOpt},

   {ENTER_CONTROL_CHAR,UserEnterControlChar},
//   {ENTER_CHAR_CODE,UserEnterCharCode},

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

   {-1,NULL}
};

void  EditorReadKeymap()
{
   char  filename[1024];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap-%s",HOME,TERM);
   f=fopen(filename,"r");
   if(f==NULL)
   {
      sprintf(filename,"%s/keymap-%s",PKGDATADIR,TERM);
      f=fopen(filename,"r");
      if(f==NULL)
      {
         sprintf(filename,"%s/.le/keymap",HOME);
         f=fopen(filename,"r");
         if(f==NULL)
         {
            sprintf(filename,"%s/keymap",PKGDATADIR);
            f=fopen(filename,"r");
            if(f==NULL)
               return;
         }
      }
   }

   errno=0;
   ReadActionMap(f);
   if(errno)
   {
      FError(filename);
      ActionCodeTable=DefaultActionCodeTable;
   }

   fclose(f);
}
