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
#include <signal.h>
#include "edit.h"
#include "menu1.h"

Menu1 MainMenu[]={
{" &File ",SUBM},
   {" &Load         F3 ",FUNC+HIDE,UserLoad           },
   {" &Save         F2 ",FUNC+HIDE,(void(*)())UserSave,    &View },
   {" save &As...  ~F2 ",FUNC+HIDE,(void(*)())UserSaveAs         },
   {" s&Witch      ~F3 ",FUNC+HIDE,UserSwitch         },
   {" re&Open R/W      ",FUNC+HIDE,(void(*)())ReopenRW	      },
   {" &Info            ",FUNC+HIDE,UserInfo           },
   {"---"},
   {" &Make         F9 ",FUNC+HIDE,DoMake,         &View },
   {" &Compile    ~^F9 ",FUNC+HIDE,DoCompile,      &View },
   {" &Run         ^F9 ",FUNC+HIDE,DoRun,       &View },
   {"---"},
   {" S&hell       ~F9 ",FUNC+HIDE,DoShell               },
#ifndef MSDOS
   {" S&uspend         ",FUNC+HIDE,SuspendEditor           },
#endif
   {" &Quit         ^X ",FUNC+HIDE,Quit               },
   {NULL},
{" &Block ",SUBM},
   {" set &Begin        F5 ",FUNC+HIDE,UserSetBlockBegin       },
   {" set &End          F6 ",FUNC+HIDE,UserSetBlockEnd         },
   {" &Copy            F11 ",FUNC+HIDE,Copy,       &View,&hide },
   {" &Move            F12 ",FUNC+HIDE,Move,       &View,&hide },
   {" &Delete         F4 D ",FUNC+HIDE,Delete,        &View,&hide },
   {" &Read           F4 R ",FUNC+HIDE,Read,       &View       },
   {" &Write          F4 W ",FUNC+HIDE,Write,         &hide    },
#ifndef __MSDOS__
   {" Pipe           F4 &| ",FUNC+HIDE,UserPipeBlock,&hide,&View },
#endif
   {"---"},
   {" &Indent         F4 I ",FUNC+HIDE,Indent,        &View,&hide },
   {" &Unindent       F4 U ",FUNC+HIDE,Unindent,      &View,&hide },
   {" Insert prefix  F4 &> ",FUNC+HIDE,UserBlockPrefixIndent,&View,&hide },
   {"---"},
   {" to &Lower case  F4 L ",FUNC+HIDE,ConvertToLower,   &View },
   {" to u&Pper case  F4 P ",FUNC+HIDE,ConvertToUpper,   &View },
   {" e&Xchange cases F4 X ",FUNC+HIDE,ExchangeCases, &View },
   {"---"},
   {" &Hide/display   F4 H ",FUNC+HIDE,HideDisplay          },
   {" block &Type     F4 T ",FUNC+HIDE,BlockType            },
   {NULL},
{" &Search ",SUBM},
   {" &Search forwards          ^F | F7 ",FUNC+HIDE,StartSearch  },
   {" search &Backwards       ^B | ~^F7 ",FUNC+HIDE,StartSearchBackward},
   {" start &Replace           ^R | ^F7 ",FUNC+HIDE,StartReplace,&View},
   {" &Continue search/replace ^C | ~F7 ",FUNC+HIDE,ContSearch      },
   {" find &Matching bracket         ^] ",FUNC+HIDE,FindMatch    },
   {" find block begin             ^F5 ",FUNC+HIDE,UserFindBlockBegin,&hide},
   {" find block end               ^F6 ",FUNC+HIDE,UserFindBlockEnd,&hide},
   {NULL},
{" &Move ",SUBM},
   {" &Line number          ^G G ",FUNC+HIDE,UserToLineNumber },
   {" &Begin of the file    ^G B ",FUNC+HIDE,UserFileBegin  },
   {" &End of the file      ^G E ",FUNC+HIDE,UserFileEnd    },
   {" &Offset               ^G O ",FUNC+HIDE,UserToOffset     },
   {" &Previous edit        ^G P ",FUNC+HIDE,UserPreviousEdit },
   {NULL},
{" Fo&rmat ",SUBM,NULL,&View},
   {" &Format one paragraph   ",FUNC+HIDE,FormatPara,&View  },
   {" &Center line            ",FUNC+HIDE,CenterLine,&View  },
   {" format &All             ",FUNC+HIDE,FormatAll,  &View },
   {NULL},
{" O&thers ",SUBM},
   {" &Calculator         ",FUNC+HIDE,editcalc             },
   {" &Draw tables        ",FUNC+HIDE,DrawFrames,    &View },
   {" &Format functions   ",FUNC+HIDE,FormatFunc,    &View },
   {" &Expand all tabs    ",FUNC+HIDE,ExpandAllTabs, &View },
   {" &Optimize text      ",FUNC+HIDE,Optimize,      &View },
   {" character &Set      ",FUNC+HIDE,UserChooseChar,&View },
   {" &Unix<->Dos         ",FUNC+HIDE,DOS_UNIX,      &View },
   {NULL},
{" &Options ",SUBM},
   {" &Editor                      ",FUNC+HIDE,Options},
   {" &Format                      ",FUNC+HIDE,FormatOptions},
   {" &Terminal                    ",SUBM},
      {" coding, &Graphics, etc...    ",FUNC,TermOpt},
      {" &Character set visualization ",FUNC,edit_chset},
      {" Full 8-bit                  ", FUNC+HIDE,set_chset_8bit},
      {" Full 8-bit, no ctrl chars   ", FUNC+HIDE,set_chset_8bit_noctrl},
      {" &Save terminal options       ",FUNC+HIDE,SaveTermOpt},
      {NULL},
    {" &Appearence                  ",FUNC,AppearOpt},
   {" &Save to current directory   ",FUNC+HIDE,SaveOpt},
   {" &Update current options file ",FUNC+HIDE,UpdtOpt},
   {NULL},
{" &Help ",SUBM},
   {" &Help on keys    F1 ",FUNC+HIDE,UserKeysHelp},
   {" &Help on word   ^F1 ",FUNC+HIDE,UserWordHelp},
   {" &About              ",FUNC+HIDE,UserAbout},
   {NULL},
{NULL}};

