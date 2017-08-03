/*
 * Copyright (c) 1993-2006 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <stdlib.h>
#include "edit.h"
#include "keynames.h"
#include <term.h>

static const struct CodeName
{
   int code;
   const char *name;
}
   CodeNameTable[]={
#if 0
{ KEY_A1         , "a1"         },
{ KEY_A3         , "a3"         },
{ KEY_B2         , "b2"         },
{ KEY_BACKSPACE  , "backspace"  },
{ KEY_BEG        , "beg"        },
{ KEY_BTAB       , "btab"       },
{ KEY_C1         , "c1"         },
{ KEY_C3         , "c3"         },
{ KEY_CANCEL     , "cancel"     },
{ KEY_CATAB      , "catab"      },
{ KEY_CLEAR      , "clear"      },
{ KEY_CLOSE      , "close"      },
{ KEY_COMMAND    , "command"    },
{ KEY_COPY       , "copy"       },
{ KEY_CREATE     , "create"     },
{ KEY_CTAB       , "ctab"       },
{ KEY_DC         , "dc"         },
{ KEY_DL         , "dl"         },
{ KEY_DOWN       , "down"       },
{ KEY_EIC        , "eic"        },
{ KEY_END        , "end"        },
{ KEY_ENTER      , "enter"      },
{ KEY_EOL        , "eol"        },
{ KEY_EOS        , "eos"        },
{ KEY_EXIT       , "exit"       },
{ KEY_F(0)       , "f0"         },
{ KEY_F(1)       , "f1"         },
{ KEY_F(10)      , "f10"        },
{ KEY_F(11)      , "f11"        },
{ KEY_F(12)      , "f12"        },
{ KEY_F(13)      , "f13"        },
{ KEY_F(14)      , "f14"        },
{ KEY_F(15)      , "f15"        },
{ KEY_F(16)      , "f16"        },
{ KEY_F(17)      , "f17"        },
{ KEY_F(18)      , "f18"        },
{ KEY_F(19)      , "f19"        },
{ KEY_F(2)       , "f2"         },
{ KEY_F(20)      , "f20"        },
{ KEY_F(21)      , "f21"        },
{ KEY_F(22)      , "f22"        },
{ KEY_F(23)      , "f23"        },
{ KEY_F(24)      , "f24"        },
{ KEY_F(25)      , "f25"        },
{ KEY_F(26)      , "f26"        },
{ KEY_F(27)      , "f27"        },
{ KEY_F(28)      , "f28"        },
{ KEY_F(29)      , "f29"        },
{ KEY_F(3)       , "f3"         },
{ KEY_F(30)      , "f30"        },
{ KEY_F(31)      , "f31"        },
{ KEY_F(32)      , "f32"        },
{ KEY_F(33)      , "f33"        },
{ KEY_F(34)      , "f34"        },
{ KEY_F(35)      , "f35"        },
{ KEY_F(36)      , "f36"        },
{ KEY_F(37)      , "f37"        },
{ KEY_F(38)      , "f38"        },
{ KEY_F(39)      , "f39"        },
{ KEY_F(4)       , "f4"         },
{ KEY_F(40)      , "f40"        },
{ KEY_F(41)      , "f41"        },
{ KEY_F(42)      , "f42"        },
{ KEY_F(43)      , "f43"        },
{ KEY_F(44)      , "f44"        },
{ KEY_F(45)      , "f45"        },
{ KEY_F(46)      , "f46"        },
{ KEY_F(47)      , "f47"        },
{ KEY_F(48)      , "f48"        },
{ KEY_F(49)      , "f49"        },
{ KEY_F(5)       , "f5"         },
{ KEY_F(50)      , "f50"        },
{ KEY_F(51)      , "f51"        },
{ KEY_F(52)      , "f52"        },
{ KEY_F(53)      , "f53"        },
{ KEY_F(54)      , "f54"        },
{ KEY_F(55)      , "f55"        },
{ KEY_F(56)      , "f56"        },
{ KEY_F(57)      , "f57"        },
{ KEY_F(58)      , "f58"        },
{ KEY_F(59)      , "f59"        },
{ KEY_F(6)       , "f6"         },
{ KEY_F(60)      , "f60"        },
{ KEY_F(61)      , "f61"        },
{ KEY_F(62)      , "f62"        },
{ KEY_F(63)      , "f63"        },
{ KEY_F(7)       , "f7"         },
{ KEY_F(8)       , "f8"         },
{ KEY_F(9)       , "f9"         },
{ KEY_FIND       , "find"       },
{ KEY_HELP       , "help"       },
{ KEY_HOME       , "home"       },
{ KEY_IC         , "ic"         },
{ KEY_IL         , "il"         },
#endif //0
{ KEY_SBEG       , "kBEG"       },
{ KEY_SCANCEL    , "kCAN"       },
{ KEY_SCOMMAND   , "kCMD"       },
{ KEY_SCOPY      , "kCPY"       },
{ KEY_SCREATE    , "kCRT"       },
{ KEY_SDC        , "kDC"        },
{ KEY_SDL        , "kDL"        },
{ KEY_SEND       , "kEND"       },
{ KEY_SEOL       , "kEOL"       },
{ KEY_SEXIT      , "kEXT"       },
{ KEY_SFIND      , "kFND"       },
{ KEY_SHELP      , "kHLP"       },
{ KEY_SHOME      , "kHOM"       },
{ KEY_SIC        , "kIC"        },
{ KEY_SLEFT      , "kLFT"       },
{ KEY_SMOVE      , "kMOV"       },
{ KEY_SMESSAGE   , "kMSG"       },
{ KEY_SNEXT      , "kNXT"       },
{ KEY_SOPTIONS   , "kOPT"       },
{ KEY_SPRINT     , "kPRT"       },
{ KEY_SPREVIOUS  , "kPRV"       },
{ KEY_SREDO      , "kRDO"       },
{ KEY_SRSUME     , "kRES"       },
{ KEY_SRIGHT     , "kRIT"       },
{ KEY_SREPLACE   , "kRPL"       },
{ KEY_SSAVE      , "kSAV"       },
{ KEY_SSUSPEND   , "kSPD"       },
{ KEY_SUNDO      , "kUND"       },
{ KEY_A1         , "ka1"        },
{ KEY_A3         , "ka3"        },
{ KEY_B2         , "kb2"        },
{ KEY_BEG        , "kbeg"       },
{ KEY_BACKSPACE  , "kbs"        },
{ KEY_C1         , "kc1"        },
{ KEY_C3         , "kc3"        },
{ KEY_CANCEL     , "kcan"       },
{ KEY_BTAB       , "kcbt"       },
{ KEY_CLOSE      , "kclo"       },
{ KEY_CLEAR      , "kclr"       },
{ KEY_COMMAND    , "kcmd"       },
{ KEY_COPY       , "kcpy"       },
{ KEY_CREATE     , "kcrt"       },
{ KEY_CTAB       , "kctab"      },
{ KEY_LEFT       , "kcub1"      },
{ KEY_DOWN       , "kcud1"      },
{ KEY_RIGHT      , "kcuf1"      },
{ KEY_UP         , "kcuu1"      },
{ KEY_DC         , "kdch1"      },
{ KEY_DL         , "kdl1"       },
{ KEY_EOS        , "ked"        },
{ KEY_EOL        , "kel"        },
{ KEY_END        , "kend"       },
{ KEY_ENTER      , "kent"       },
{ KEY_EXIT       , "kext"       },
{ KEY_F(0)       , "kf0"        },
{ KEY_F(1)       , "kf1"        },
{ KEY_F(10)      , "kf10"       },
{ KEY_F(11)      , "kf11"       },
{ KEY_F(12)      , "kf12"       },
{ KEY_F(13)      , "kf13"       },
{ KEY_F(14)      , "kf14"       },
{ KEY_F(15)      , "kf15"       },
{ KEY_F(16)      , "kf16"       },
{ KEY_F(17)      , "kf17"       },
{ KEY_F(18)      , "kf18"       },
{ KEY_F(19)      , "kf19"       },
{ KEY_F(2)       , "kf2"        },
{ KEY_F(20)      , "kf20"       },
{ KEY_F(21)      , "kf21"       },
{ KEY_F(22)      , "kf22"       },
{ KEY_F(23)      , "kf23"       },
{ KEY_F(24)      , "kf24"       },
{ KEY_F(25)      , "kf25"       },
{ KEY_F(26)      , "kf26"       },
{ KEY_F(27)      , "kf27"       },
{ KEY_F(28)      , "kf28"       },
{ KEY_F(29)      , "kf29"       },
{ KEY_F(3)       , "kf3"        },
{ KEY_F(30)      , "kf30"       },
{ KEY_F(31)      , "kf31"       },
{ KEY_F(32)      , "kf32"       },
{ KEY_F(33)      , "kf33"       },
{ KEY_F(34)      , "kf34"       },
{ KEY_F(35)      , "kf35"       },
{ KEY_F(36)      , "kf36"       },
{ KEY_F(37)      , "kf37"       },
{ KEY_F(38)      , "kf38"       },
{ KEY_F(39)      , "kf39"       },
{ KEY_F(4)       , "kf4"        },
{ KEY_F(40)      , "kf40"       },
{ KEY_F(41)      , "kf41"       },
{ KEY_F(42)      , "kf42"       },
{ KEY_F(43)      , "kf43"       },
{ KEY_F(44)      , "kf44"       },
{ KEY_F(45)      , "kf45"       },
{ KEY_F(46)      , "kf46"       },
{ KEY_F(47)      , "kf47"       },
{ KEY_F(48)      , "kf48"       },
{ KEY_F(49)      , "kf49"       },
{ KEY_F(5)       , "kf5"        },
{ KEY_F(50)      , "kf50"       },
{ KEY_F(51)      , "kf51"       },
{ KEY_F(52)      , "kf52"       },
{ KEY_F(53)      , "kf53"       },
{ KEY_F(54)      , "kf54"       },
{ KEY_F(55)      , "kf55"       },
{ KEY_F(56)      , "kf56"       },
{ KEY_F(57)      , "kf57"       },
{ KEY_F(58)      , "kf58"       },
{ KEY_F(59)      , "kf59"       },
{ KEY_F(6)       , "kf6"        },
{ KEY_F(60)      , "kf60"       },
{ KEY_F(61)      , "kf61"       },
{ KEY_F(62)      , "kf62"       },
{ KEY_F(63)      , "kf63"       },
{ KEY_F(7)       , "kf7"        },
{ KEY_F(8)       , "kf8"        },
{ KEY_F(9)       , "kf9"        },
{ KEY_FIND       , "kfnd"       },
{ KEY_HELP       , "khlp"       },
{ KEY_HOME       , "khome"      },
{ KEY_STAB       , "khts"       },
{ KEY_IC         , "kich1"      },
{ KEY_IL         , "kil1"       },
{ KEY_SF         , "kind"       },
{ KEY_LL         , "kll"        },
{ KEY_MOVE       , "kmov"       },
{ KEY_MARK       , "kmrk"       },
{ KEY_MESSAGE    , "kmsg"       },
{ KEY_NPAGE      , "knp"        },
{ KEY_NEXT       , "knxt"       },
{ KEY_OPEN       , "kopn"       },
{ KEY_OPTIONS    , "kopt"       },
{ KEY_PPAGE      , "kpp"        },
{ KEY_PRINT      , "kprt"       },
{ KEY_PREVIOUS   , "kprv"       },
{ KEY_REDO       , "krdo"       },
{ KEY_REFERENCE  , "kref"       },
{ KEY_RESUME     , "kres"       },
{ KEY_REFRESH    , "krfr"       },
{ KEY_SR         , "kri"        },
{ KEY_EIC        , "krmir"      },
{ KEY_REPLACE    , "krpl"       },
{ KEY_RESTART    , "krst"       },
{ KEY_SAVE       , "ksav"       },
{ KEY_SELECT     , "kslt"       },
{ KEY_SUSPEND    , "kspd"       },
{ KEY_CATAB      , "ktbc"       },
{ KEY_UNDO       , "kund"       },
#if 0
{ KEY_LEFT       , "left"       },
{ KEY_LL         , "ll"         },
{ KEY_MARK       , "mark"       },
{ KEY_MESSAGE    , "message"    },
{ KEY_MOVE       , "move"       },
{ KEY_NEXT       , "next"       },
{ KEY_NPAGE      , "npage"      },
{ KEY_OPEN       , "open"       },
{ KEY_OPTIONS    , "options"    },
{ KEY_PPAGE      , "ppage"      },
{ KEY_PREVIOUS   , "previous"   },
{ KEY_PRINT      , "print"      },
{ KEY_REDO       , "redo"       },
{ KEY_REFERENCE  , "reference"  },
{ KEY_REFRESH    , "refresh"    },
{ KEY_REPLACE    , "replace"    },
{ KEY_RESTART    , "restart"    },
{ KEY_RESUME     , "resume"     },
{ KEY_RIGHT      , "right"      },
{ KEY_SAVE       , "save"       },
{ KEY_SBEG       , "sbeg"       },
{ KEY_SCANCEL    , "scancel"    },
{ KEY_SCOMMAND   , "scommand"   },
{ KEY_SCOPY      , "scopy"      },
{ KEY_SCREATE    , "screate"    },
{ KEY_SDC        , "sdc"        },
{ KEY_SDL        , "sdl"        },
{ KEY_SELECT     , "select"     },
{ KEY_SEND       , "send"       },
{ KEY_SEOL       , "seol"       },
{ KEY_SEXIT      , "sexit"      },
{ KEY_SF         , "sf"         },
{ KEY_SFIND      , "sfind"      },
{ KEY_SHELP      , "shelp"      },
{ KEY_SHOME      , "shome"      },
{ KEY_SIC        , "sic"        },
{ KEY_SLEFT      , "sleft"      },
{ KEY_SMESSAGE   , "smessage"   },
{ KEY_SMOVE      , "smove"      },
{ KEY_SNEXT      , "snext"      },
{ KEY_SOPTIONS   , "soptions"   },
{ KEY_SPREVIOUS  , "sprevious"  },
{ KEY_SPRINT     , "sprint"     },
{ KEY_SR         , "sr"         },
{ KEY_SREDO      , "sredo"      },
{ KEY_SREPLACE   , "sreplace"   },
{ KEY_SRIGHT     , "sright"     },
{ KEY_SRSUME     , "srsume"     },
{ KEY_SSAVE      , "ssave"      },
{ KEY_SSUSPEND   , "ssuspend"   },
{ KEY_STAB       , "stab"       },
{ KEY_SUNDO      , "sundo"      },
{ KEY_SUSPEND    , "suspend"    },
{ KEY_UNDO       , "undo"       },
{ KEY_UP         , "up"         },
#endif //0
};
const int CodeNameTableSize=sizeof(CodeNameTable)/sizeof(CodeNameTable[0]);

static struct CodeName *CodeNameTableExt;
static int CodeNameTableExtSize;

static int node_compare(const void *node1, const void *node2)
{
   return(strcmp(((const CodeName*)node1)->name,
		 ((const CodeName*)node2)->name));
}

static void MakeCodeNameTableExt()
{
#if NCURSES_XNAMES

/* These are taken from ncurses internals */
#define NUM_STRINGS(tp)  ((tp)->num_Strings)
#define EXT_NAMES(tp,i,limit,index,table) ((i >= limit) ? tp->ext_Names[index] : table[i])
#define ExtStrname(tp,i,names)  EXT_NAMES(tp, i, STRCOUNT, (i - (tp->num_Strings - tp->ext_Strings)) + (tp->ext_Numbers + tp->ext_Booleans), names)

   if(!cur_term)
      return;
   TERMTYPE *tp = (TERMTYPE *)(cur_term);
   if(!tp)
      return;
   if(NUM_STRINGS(tp)<=STRCOUNT)
      return;
   CodeNameTableExt=(CodeName*)malloc(sizeof(CodeName)*(NUM_STRINGS(tp)-STRCOUNT));
   if(!CodeNameTableExt)
      return;
   CodeNameTableExtSize=0;
   for(int n=STRCOUNT; n<NUM_STRINGS(tp); n++)
   {
      const char *name = ExtStrname(tp, n, strnames);
      const char *value = tp->Strings[n];
      if (name && *name=='k' && value)
      {
	 CodeNameTableExt[CodeNameTableExtSize].code=n-STRCOUNT+KEY_MAX;
	 CodeNameTableExt[CodeNameTableExtSize].name=strdup(name);
	 if(!CodeNameTableExt[CodeNameTableExtSize].name)
	    return;
	 CodeNameTableExtSize++;
      }
   }
   if(CodeNameTableExtSize)
      qsort(CodeNameTableExt,CodeNameTableExtSize,sizeof(CodeName),node_compare);
#endif
}

int FindKeyCode(const char *name)
{
   CodeName node={-1,name};

   void *addr=bsearch(&node,CodeNameTable,CodeNameTableSize,
		      sizeof(CodeNameTable[0]),node_compare);
   if(!addr)
   {
      if(!CodeNameTableExt)
	 MakeCodeNameTableExt();
      if(!CodeNameTableExt)
	 return -1;
      addr=bsearch(&node,CodeNameTableExt,CodeNameTableExtSize,
		   sizeof(CodeNameTableExt[0]),node_compare);
      if(!addr)
	 return -1;
   }
   return ((CodeName*)addr)->code;
}
