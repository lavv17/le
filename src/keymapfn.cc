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

#include <config.h>
#include <stdio.h>
#include <errno.h>
#include "edit.h"
#include "block.h"
#include "keymap.h"

ActionProcRec  EditorActionProcTable[]=
{
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
   {FORMAT_FUNC_BAR,FormatFunc},

// Others
   {CALCULATOR,editcalc},
   {DRAW_FRAMES,DrawFrames},
   {TABS_EXPAND,ExpandAllTabs},
   {TEXT_OPTIMIZE,Optimize},
   {CHOOSE_CHAR,UserChooseChar},
   {UNIX_DOS_TRANSFORM,DOS_UNIX},

// Options
   {EDITOR_OPTIONS,Options},
   {TERMINAL_OPTIONS,TermOpt},
   {FORMAT_OPTIONS,FormatOptions},
   {APPEARENCE_OPTIONS,AppearOpt},
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


/*   char  filename[1024];

   sprintf(filename,"%s/.le",HOME);
   mkdir(filename,0755);

   errno=0;
   sprintf(filename,"%s/.le/keymap-%s",HOME,TERM);
   f=fopen(filename,"w");
   if(f==NULL)
      return;

   fclose(f);*/
