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

#include <config.h>
#include "edit.h"
#include "menu1.h"
#include "block.h"
#include "options.h"
#include "clipbrd.h"
#include "format.h"
#include "colormnu.h"
#include "about.h"
#include "search.h"

static bool RW() { return !View; }
static bool BLK() { return !hide; }
static bool noMM() { return !buffer_mmapped; }
static bool RW_BLK() { return RW() && BLK(); }
static bool RW_noMM() { return RW() && noMM(); }
static bool RW_BLK_noMM() { return RW_BLK() && noMM(); }
static bool RW_CLIP() { return RW() && !MainClipBoard.IsEmpty(); }

typedef void (*f)();

Menu1 MainMenu[]={
{" &File ",SUBM},
   {" &Load         F3 ",FUNC+HIDE,UserLoad		    },
   {" &Save         F2 ",FUNC+HIDE,(f)UserSave,		 RW },
   {" save &As...  ~F2 ",FUNC+HIDE,(f)UserSaveAs	    },
   {" s&Witch      ~F3 ",FUNC+HIDE,UserSwitch		    },
   {" re&Open R/W      ",FUNC+HIDE,(f)ReopenRW		    },
   {" &Info            ",FUNC+HIDE,UserInfo		    },
   {"---"},
   {" &Make         F9 ",FUNC+HIDE,DoMake,		    },
   {" &Compile    ~^F9 ",FUNC+HIDE,DoCompile,		    },
   {" &Run         ^F9 ",FUNC+HIDE,DoRun,		    },
   {"---"},
   {" S&hell       ~F9 ",FUNC+HIDE,DoShell		    },
#ifndef MSDOS
   {" S&uspend         ",FUNC+HIDE,SuspendEditor	    },
#endif
   {" &Quit         ^X ",FUNC+HIDE,Quit			    },
   {NULL},
{" &Block ",SUBM},
   {" set &Begin        F5 ",FUNC+HIDE,UserSetBlockBegin    },
   {" set &End          F6 ",FUNC+HIDE,UserSetBlockEnd      },
   {" &Copy            F11 ",FUNC+HIDE,Copy,	   RW_BLK   },
   {" &Move            F12 ",FUNC+HIDE,Move,	   RW_BLK   },
   {" &Delete         F4 D ",FUNC+HIDE,(f)Delete,  RW_BLK_noMM },
   {" &Read           F4 R ",FUNC+HIDE,Read,       RW       },
   {" &Write          F4 W ",FUNC+HIDE,Write,      BLK	    },
#ifndef __MSDOS__
   {" Pipe           F4 &| ",FUNC+HIDE,UserPipeBlock, RW_BLK_noMM },
#endif
   {"---"},
   {" &Indent         F4 I ",FUNC+HIDE,Indent,     RW_BLK_noMM },
   {" &Unindent       F4 U ",FUNC+HIDE,Unindent,   RW_BLK_noMM },
   {" Insert prefix  F4 &> ",FUNC+HIDE,UserBlockPrefixIndent,RW_BLK_noMM },
   {" &Yank old       F4 Y ",FUNC+HIDE,UserYankBlock,RW_CLIP },
   {"---"},
   {" to &Lower case  F4 L ",FUNC+HIDE,ConvertToLower,   RW },
   {" to u&Pper case  F4 P ",FUNC+HIDE,ConvertToUpper,   RW },
   {" e&Xchange cases F4 X ",FUNC+HIDE,ExchangeCases,	 RW },
   {"---"},
   {" &Hide/display   F4 H ",FUNC+HIDE,HideDisplay          },
   {" Drag mark      F4 &V ",FUNC+HIDE,UserStartDragMark    },
   {" block &Type     F4 T ",FUNC+HIDE,BlockType,  	 noMM },
   {NULL},
{" &Search ",SUBM},
   {" &Search forwards          ^F | F7 ",FUNC+HIDE,StartSearch	     },
   {" search &Backwards       ^B | ~^F7 ",FUNC+HIDE,StartSearchBackward},
   {" start &Replace           ^R | ^F7 ",FUNC+HIDE,StartReplace,RW  },
   {" &Continue search/replace ^C | ~F7 ",FUNC+HIDE,ContSearch	     },
   {" find &Matching bracket         ^] ",FUNC+HIDE,FindMatch	     },
   {NULL},
{" &Move ",SUBM},
   {" &Line number          ^G G ",FUNC+HIDE,UserToLineNumber  },
   {" &Begin of the file    ^G B ",FUNC+HIDE,UserFileBegin     },
   {" &End of the file      ^G E ",FUNC+HIDE,UserFileEnd       },
   {" &Offset               ^G O ",FUNC+HIDE,UserToOffset      },
   {" &Previous edit        ^G P ",FUNC+HIDE,UserPreviousEdit  },
   {" Block begin           ^F5 ",FUNC+HIDE,UserFindBlockBegin,	BLK},
   {" Block end             ^F6 ",FUNC+HIDE,UserFindBlockEnd,	BLK},
   {NULL},
{" Fo&rmat ",SUBM,NULL, RW_noMM},
   {" enter &Format mode      ",FUNC+HIDE,FormatFunc,    RW_noMM  },
   {" format one &Paragraph   ",FUNC+HIDE,FormatPara,	 RW_noMM  },
   {" &Center line            ",FUNC+HIDE,CenterLine,	 RW_noMM  },
   {" ajust &Right line       ",FUNC+HIDE,ShiftRightLine,RW_noMM  },
   {" format &All             ",FUNC+HIDE,FormatAll,	 RW_noMM  },
   {NULL},
{" O&thers ",SUBM},
   {" &Calculator         ",FUNC+HIDE,editcalc		       },
   {" &Draw tables        ",FUNC+HIDE,DrawFrames,     RW_noMM  },
   {" &Format functions   ",FUNC+HIDE,FormatFunc,     RW_noMM  },
   {" &Expand all tabs    ",FUNC+HIDE,ExpandAllTabs,  RW_noMM  },
   {" &Optimize text      ",FUNC+HIDE,Optimize,	      RW_noMM  },
   {" character &Set      ",FUNC+HIDE,UserChooseChar	       },
   {" &Unix<->Dos         ",FUNC+HIDE,DOS_UNIX,       RW_noMM  },
   {NULL},
{" &Options ",SUBM},
   {" &Editor                      ",FUNC,Options},
   {" &Format                      ",FUNC,FormatOptions},
   {" &Terminal                    ",SUBM},
      {" coding, &Graphics, etc...    ",FUNC,TermOpt},
      {" &Character set visualization ",FUNC,edit_chset},
      {" Full 8-bit                  ", FUNC+HIDE,set_chset_8bit},
      {" Full 8-bit, no ctrl chars   ", FUNC+HIDE,set_chset_8bit_noctrl},
      {" &Save terminal options       ",FUNC+HIDE,SaveTermOpt},
      {NULL},
   {" &Appearence                  ",FUNC,AppearOpt},
   {" &Colors                      ",SUBM},
      {" &Edit                       ",FUNC+HIDE,ColorsOpt},
      {" &Save                       ",FUNC+HIDE,ColorsSave},
      {" Save as &terminal specific  ",FUNC+HIDE,ColorsSaveForTerminal},
      {"---"},
      {" &Default                    ",FUNC+HIDE,LoadColorDefault},
      {" Load default-background    ",FUNC+HIDE,LoadColorDefaultBG},
      {" Load black                 ",FUNC+HIDE,LoadColorBlack},
      {" Load blue                  ",FUNC+HIDE,LoadColorBlue},
      {" Load green                 ",FUNC+HIDE,LoadColorGreen},
      {" Load white                 ",FUNC+HIDE,LoadColorWhite},
      {NULL},
   {" &Programs                    ",FUNC,ProgOpt},
   {" &Save to current directory   ",FUNC+HIDE,SaveOpt},
   {" &Update current options file ",FUNC+HIDE,UpdtOpt},
   {NULL},
{" &Help ",SUBM},
   {" &Help on keys    F1 ",FUNC+HIDE,UserKeysHelp},
   {" &Help on word   ^F1 ",FUNC+HIDE,UserWordHelp},
   {" &About              ",FUNC+HIDE,UserAbout},
   {NULL},
{NULL}};
