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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "edit.h"
#include "rus.h"
#include "keymap.h"
#include "options.h"
#include "highli.h"

extern  LineLen;
extern  LeftMargin;
extern  FirstLineMargin;
extern  char   ContextHelpNames[];
extern  RightAdj,LeftAdj;

int useidl=0;

extern  grsetno;

#define STR    0
#define ONE    1
#define MANY   2
#define BUTTON 3
#define NUM    4

int fx,fy;

static   OldTabSize;

#define _StrPos 16
#define _StrLen 49

#define _WinWidth   (_StrPos+_StrLen+3)
#define _WinHeight  20

struct  opt
{
   char  *name;
   int    type;
   void  *var;
   int    x,y;
   int   len;
   int   maxlen;
   int   flags;
   char  *old;
} opt[]=
{
{"&Insert",          ONE,  (void*)&insert,    3,2},
{"&Autoindent",      ONE,  (void*)&autoindent, 3,3},

{"Save history",     ONE,  (void*)&SaveHst,  22,2},
{"Save positions",   ONE,  (void*)&SavePos,  22,3},

// {"rectangle &Blocks",ONE,  (void*)&rblock,  45,2},
{"Make backup",      ONE,  (void*)&makebak,  45,2},
{"No regular expr.", ONE,  (void*)&noreg,   45,3},
{"Use colors",       ONE,  (void*)&UseColor,   45,4},
{"Syntax highlight", ONE,  (void*)&hl_option,	45,5},
{"Use tabs",         ONE,  (void*)&UseTabs,  45,6},
{"BackSp unindents", ONE,  (void*)&BackspaceUnindents,45,7},

{"&Latin",           MANY, (void*)&inputmode,  3,5},
{"r&Ussian",         MANY, (void*)&inputmode,  3,6},
{"&Graphic",         MANY, (void*)&inputmode,  3,7},

{"&Exact",           MANY, (void*)&editmode,   22,5},
{"te&Xt",            MANY, (void*)&editmode,   22,6},
{"&Hex",             MANY, (void*)&editmode,   22,7},

{"&Tab size",        NUM,  (void*)&TabSize,  3,9},
{"Indent size",      NUM,  (void*)&IndentSize, 3,10},

{"Vertical scroll",  NUM,  (void*)&Scroll,  22,9},
{"Horizontal scroll",NUM,  (void*)&hscroll,   22,10},

{"Backup suffix",    STR,  bak,  45,9,4,sizeof(bak),FRIGHT   },

{"&Compile",         STR,  (void*)Compile,   _StrPos,12,_StrLen,sizeof(Compile)  },
{"&Make",            STR,  (void*)Make,      _StrPos,13,_StrLen,sizeof(Make)     },
{"&Run",             STR,  (void*)Run,       _StrPos,14,_StrLen,sizeof(Run)      },
{"&Shell",           STR,  (void*)Shell,     _StrPos,15,_StrLen,sizeof(Shell)    },
{"Help command",     STR,  (void*)HelpCmd,   _StrPos,16,_StrLen,sizeof(HelpCmd)  },
{"backup &Path",     STR,  (void*)BakPath,   _StrPos,17,_StrLen,sizeof(BakPath)  },

/*
{"  Save  ",   BUTTON, NULL,   MIDDLE-15,FDOWN-2},
{" Update ",   BUTTON, NULL,   MIDDLE-5,FDOWN-2},
{"  Use   ",   BUTTON, NULL,   MIDDLE+5,FDOWN-2},
{" Cancel ",   BUTTON, NULL,   MIDDLE+15,FDOWN-2},*/
{NULL}
},
   TOpt[]=
{
{"No cyrillic",      MANY,   (void*)&coding, 3,2},
{"Alternative",      MANY,   (void*)&coding, 3,3},
{"Alternative with 'jo'",MANY,   (void*)&coding, 3,4},
{"KOI-8",            MANY,   (void*)&coding, 3,5},
{"KOI-8-BESTA",      MANY,   (void*)&coding, 3,6},
{"Main",             MANY,   (void*)&coding, 3,7},

{"IBM coding      ³Å¿ÚÂÃ´ÀÙÁÄ",MANY, (void*)&grsetno,32,2},
{"KOI-8 coding 1  ‹—²š›Œ˜±™œ",MANY, (void*)&grsetno,32,3},
{"KOI-8 coding 2  ƒ•ª’“„©‘”",MANY, (void*)&grsetno,32,4},
{"No graphics     |+++++++++-",MANY, (void*)&grsetno,32,5},

{"Use insert/delete line cap.",ONE, &useidl,32,7},
{"Number of functional keys",NUM,   (void*)&FuncKeysNum,32,8},
{NULL}
},
   FormatOpt[]=
{
{"Left Margin",      NUM, (void*)&LeftMargin,     3,2},
{"Line Length",      NUM, (void*)&LineLen,        3,3},
{"First line margin",NUM, (void*)&FirstLineMargin,3,4},
{"Left adjustment",  ONE, &LeftAdj,               3,6},
{"Right adjustment", ONE, &RightAdj,              3,7},
{NULL}
},
   AppearOptTbl[]=
{
{"Right scroll bar", MANY,   &ShowScrollBar,3,2},
{"No scroll bar",    MANY,   &ShowScrollBar,3,3},
{"Left scroll bar",  MANY,   &ShowScrollBar,3,4},

{"Bottom status line",MANY,   &ShowStatusLine,3,6},
{"No status line",   MANY,   &ShowStatusLine,3,7},
{"Top status line",  MANY,   &ShowStatusLine,3,8},
{NULL},
};

struct init
   init[]=
{
   { "tabsize",      NUM,  (void*)&TabSize            },
   { "indentsize",   NUM,  (void*)&IndentSize         },
   { "autoindent",   NUM,  (void*)&autoindent         },
   { "bsunindents",  NUM,  (void*)&BackspaceUnindents },
   { "insert",       NUM,  (void*)&insert             },
   { "inputmode",    NUM,  (void*)&inputmode          },
   { "editmode",     NUM,  (void*)&editmode           },
   { "makebak",      NUM,  (void*)&makebak            },
   { "bakpath",      STR,  (void*)BakPath             },
   { "make",         STR,  (void*)Make                },
   { "shell",        STR,  (void*)Shell               },
   { "run",          STR,  (void*)Run                 },
   { "compile",      STR,  (void*)Compile             },
   { "scroll",       NUM,  (void*)&Scroll             },
   { "hscroll",      NUM,  (void*)&hscroll            },
   { "rblock",       NUM,  (void*)&rblock             },
   { "savepos",      NUM,  (void*)&SavePos            },
   { "savehst",      NUM,  (void*)&SaveHst            },
   { "noreg",        NUM,  (void*)&noreg              },
   { "linelen",      NUM,  (void*)&LineLen            },
   { "leftmrg",      NUM,  (void*)&LeftMargin         },
   { "flnmarg",      NUM,  (void*)&FirstLineMargin    },
   { "leftadj",      NUM,  (void*)&LeftAdj            },
   { "rightadj",     NUM,  (void*)&RightAdj           },
   { "helpcmd",      STR,  (void*)HelpCmd             },
   { "usecolor",     NUM,  (void*)&UseColor           },
   { "usetabs",      NUM,  (void*)&UseTabs            },
   { "scrollbar",    NUM,  &ShowScrollBar             },
   { "statusline",   NUM,  &ShowStatusLine            },
   { "backupext",    STR,  bak                        },
   { NULL }
},
   term[]=
{
   { "coding",       NUM,  (void*)&coding	      },
   { "grsetno",      NUM,  (void*)&grsetno	      },
   { "chset",        STR,  (void*)&chset	      },
   { "fknum",        NUM,  (void*)&FuncKeysNum	      },
   { "useidl",       NUM,  &useidl      	      },
   { NULL }
},
   colors[]=
{
   { "status_line",	STR,  color_descriptions[STATUS_LINE]		   },
   { "status_line_bw",	STR,  color_descriptions[STATUS_LINE+MAX_COLOR_NO] },
   { "normal_text",	STR,  color_descriptions[NORMAL_TEXT]		   },
   { "normal_text_bw",	STR,  color_descriptions[NORMAL_TEXT+MAX_COLOR_NO] },
   { "block_text",	STR,  color_descriptions[BLOCK_TEXT]		   },
   { "block_text_bw",	STR,  color_descriptions[BLOCK_TEXT+MAX_COLOR_NO]  },
   { "error_win",	STR,  color_descriptions[ERROR_WIN]		   },
   { "error_win_bw",	STR,  color_descriptions[ERROR_WIN+MAX_COLOR_NO]   },
   { "verify_win",	STR,  color_descriptions[VERIFY_WIN]		   },
   { "verify_win_bw",	STR,  color_descriptions[VERIFY_WIN+MAX_COLOR_NO]  },
   { "curr_button",	STR,  color_descriptions[CURR_BUTTON]              },
   { "curr_button_bw",	STR,  color_descriptions[CURR_BUTTON+MAX_COLOR_NO] },
   { "help_win",	STR,  color_descriptions[HELP_WIN]  		   },
   { "help_win_bw",	STR,  color_descriptions[HELP_WIN+MAX_COLOR_NO]    },
   { "dialogue_win",	STR,  color_descriptions[DIALOGUE_WIN]             },
   { "dialogue_win_bw",	STR,  color_descriptions[DIALOGUE_WIN+MAX_COLOR_NO]},
   { "menu_win",	STR,  color_descriptions[MENU_WIN]  		   },
   { "menu_win_bw",	STR,  color_descriptions[MENU_WIN+MAX_COLOR_NO]    },
   { "disabled_item",	STR,  color_descriptions[DISABLED_ITEM]            },
   { "disabled_item_bw",STR,  color_descriptions[DISABLED_ITEM+MAX_COLOR_NO]},
   { "scroll_bar",	STR,  color_descriptions[SCROLL_BAR]		   },
   { "scroll_bar_bw",	STR,  color_descriptions[SCROLL_BAR+MAX_COLOR_NO]  },
   { "shadowed",	STR,  color_descriptions[SHADOWED]  		   },
   { "shadowed_bw",	STR,  color_descriptions[SHADOWED+MAX_COLOR_NO]    },
   { "syntax1",		STR,  color_descriptions[SYNTAX1]		   },
   { "syntax1_bw",	STR,  color_descriptions[SYNTAX1+MAX_COLOR_NO]	   },
   { "syntax2",		STR,  color_descriptions[SYNTAX2]		   },
   { "syntax2_bw",	STR,  color_descriptions[SYNTAX2+MAX_COLOR_NO]	   },
   { "syntax3",		STR,  color_descriptions[SYNTAX3]		   },
   { "syntax3_bw",	STR,  color_descriptions[SYNTAX3+MAX_COLOR_NO]	   },
   { NULL }
};

void  SaveConfToOpenFile(FILE *f,struct init *init)
{
   struct  init   *p;

   for(p=init; p->name; p++)
   {
      fprintf(f,"%s=",p->name);
      if(p->format==NUM)
         fprintf(f,"%d",*(int*)(p->var));
      else if(p->format==STR)
         fputs((char*)(p->var),f);
      fputc('\n',f);
   }
}

void  SaveConfToFile(char *f,struct init *init)
{
   FILE  *conf;

   conf=fopen(f,"w");
   if(conf==NULL)
   {
      FError(f);
      return;
   }
   SaveConfToOpenFile(conf,init);
   if(ferror(conf))
   {
      fclose(conf);
      FError(f);
      return;
   }
   fclose(conf);
}

void  SaveConf(char *f)
{
   Message("Saving the editor options...");
   SaveConfToFile(f,init);
   strcpy(InitName,f);
}

void  SaveTermOpt()
{
   char  t[256];
   Message("Saving the terminal options...");
#ifndef MSDOS
   sprintf(t,"%s/.le/term-%s",HOME,TERM);
#else
   sprintf(t,"%s/le-%s",HOME,TERM);
#endif
   SaveConfToFile(t,term);
}

void  fskip(FILE *f)
{
   int i;
   while((i=getc(f))!=EOF && i!='\n');
}

void  ReadConfFromOpenFile(FILE *f,struct init *init)
{
   struct init *ptr;
   int    i;
   char  str[256];
   char  *s;

   while(fscanf(f,"%[^=]=",str)==1)
   {
      for(i=strlen(str); i>0; i--)
         str[i-1]=tolower(str[i-1]);
      for(ptr=init; ptr->name; ptr++)
      {
         if(!strcmp(ptr->name,str))
         {
            if(ptr->format==NUM)
            {
               if(fscanf(f,"%d\n",(int*)(ptr->var))<1)
                  fskip(f);
               break;
            }
            else if(ptr->format==STR)
            {
               s=(char*)(ptr->var);
               do
               {
                  if((i=getc(f))==EOF || i=='\n')
                     break;
                  *s++=i;
                  if(s-(char*)(ptr->var)>=255)
                  {
                     fskip(f);
                     break;
                  }
               }
               while(1);
               *s='\0';
            }
            break;
         }
      }
      if(!ptr->name)
         fskip(f);
   }
}

void  ReadConfFromFile(char *ini,struct init *init)
{
   FILE  *f;
   f=fopen(ini,"r");
   if(f==NULL)
      return;
   ReadConfFromOpenFile(f,init);
   fclose(f);
}

void  ReadConf()
{
   char  t[256];

#ifndef __MSDOS__
   sprintf(t,"%s/.le/term-%s",HOME,TERM);
   if(access(t,R_OK)==-1)
   {
      sprintf(t,"%s/term-%s",PKGDATADIR,TERM);
      if(access(t,R_OK)==-1)
      {
	 sprintf(t,"%s/.le/term",HOME);
	 if(access(t,R_OK)==-1)
            sprintf(t,"%s/term",PKGDATADIR);
      }
   }
   ReadConfFromFile(t,term);

   if(chset[0]=='7') // workaround for older version
      init_chset();

   sprintf(t,"%s/.le/colors-%s",HOME,TERM);
   if(access(t,R_OK)==-1)
   {
      sprintf(t,"%s/colors-%s",PKGDATADIR,TERM);
      if(access(t,R_OK)==-1)
      {
	 sprintf(t,"%s/.le/colors",HOME);
	 if(access(t,R_OK)==-1)
	    sprintf(t,"%s/colors",PKGDATADIR);
      }
   }
   ReadConfFromFile(t,colors);
   ParseColors();

   strcpy(InitName,".le.ini");
   if(access(InitName,R_OK)==-1)
   {
      sprintf(InitName,"%s/.le/le.ini",HOME);
      if(access(InitName,R_OK)==-1)
      {
	 sprintf(t,"%s/le.ini",PKGDATADIR);
	 ReadConfFromFile(t,init);
      }
      else
	 ReadConfFromFile(InitName,init);
   }
   else
      ReadConfFromFile(InitName,init);

#else
   sprintf(t,"%s/le-%s",HOME,TERM);
   ReadConfFromFile(t,term);
   strcpy(InitName,"le.ini");
   if(access(InitName,R_OK)==-1)
     sprintf(InitName,"%s/le.ini",HOME);
   ReadConfFromFile(InitName,init);
#endif

   CorrectParameters();
}

void  CorrectParameters()
{
   if(TabSize>40)
     TabSize=40;
   if(TabSize<2)
     TabSize=2;
   if(hscroll>40)
     hscroll=40;
   if(hscroll<1)
     hscroll=1;
   if(Scroll>LINES/2)
     Scroll=LINES/2;
   if(Scroll<1)
     Scroll=1;
   if(LineLen<10)
     LineLen=10;
   if(LineLen>99)
     LineLen=99;
   if(LeftMargin<0)
     LeftMargin=0;
   if(FirstLineMargin<0)
     FirstLineMargin=0;
   if(FirstLineMargin>LineLen/2)
     FirstLineMargin=LineLen/2;

   switch(ShowScrollBar)
   {
   case(SHOW_LEFT):
     TextWinX=1;
     TextWinWidth=COLS-1;
     ScrollBarX=0;
     break;
   default:
     ShowScrollBar=SHOW_RIGHT;
     TextWinX=0;
     TextWinWidth=COLS-1;
     ScrollBarX=COLS-1;
     break;
   case(SHOW_NONE):
     TextWinX=0;
     TextWinWidth=COLS;
     ScrollBarX=-1;
     break;
   }
   switch(ShowStatusLine)
   {
   case(SHOW_TOP):
     TextWinY=1;
     TextWinHeight=LINES-1;
     StatusLineY=0;
     break;
   default:
     ShowStatusLine=SHOW_BOTTOM;
     TextWinY=0;
     TextWinHeight=LINES-1;
     StatusLineY=LINES-1;
     break;
   case(SHOW_NONE):
     TextWinY=0;
     TextWinHeight=LINES;
     StatusLineY=-1;
     break;
   }
#if 0
   if(text_w)
      delwin(text_w);
   text_w=subwin(stdscr,TextWinHeight,TextWinWidth,TextWinY,TextWinX);
   assert(text_w!=NULL);
   if(status_w)
      delwin(status_w);
   status_w=subwin(stdscr,1,0,StatusLineY,0);
   assert(status_w!=NULL);
#endif

   idlok(stdscr,useidl);

   if(buffer)
   {
     if(hex)
       ScreenTop=ScreenTop&~15;
     if(TabSize!=OldTabSize)
       TextPoint::OrFlags(COLUNDEFINED);
   }

   init_attrs();
}

int GetNo(struct opt *p,struct opt *p1)
{
   int i;
   for(i=0; p1!=p; p1++)
   {
      if(p1->var==p->var)
         i++;
   }
   return(i);
}

int GetDist(struct opt *to,int action)
{
   int d=30000;
   int tx=to->x,ty=to->y;

   Absolute(&tx,1,Upper->w);
   Absolute(&ty,1,Upper->h);
   Absolute(&fx,1,Upper->w);
   Absolute(&fy,1,Upper->h);

   switch(action)
   {
   case(LINE_UP):
      d=fy-ty;
      if(d<=0)
         d+=Upper->h;
      d*=256;
      d+=abs(fx-tx);
      break;
   case(LINE_DOWN):
      d=ty-fy;
      if(d<=0)
         d+=Upper->h;
      d*=256;
      d+=abs(fx-tx);
      break;
   case(CHAR_LEFT):
      d=fx-tx;
      if(d<=0)
         d+=Upper->w;
/*   d*=256;*/
      d+=abs(fy-ty)*20;
      break;
   case(CHAR_RIGHT):
      d=tx-fx;
      if(d<=0)
         d+=Upper->w;
/*   d*=256;*/
      d+=abs(fy-ty)*20;
      break;
   }
   return(d);
}


void  W_Dialogue(struct opt *opt,
             char **SetupHelp[],char *SetupTitle,
             int (*EatKey)(int),int (*HandleButton)(char *))
{
   int newitem=0;
   int first=1;
   char  s[512];
   struct opt *p,*p1,*n,*curr=opt;
   int shift=0,pos=0,i,key=0,d,dist;
   int size;
   int   action;
   int   OldShowStatusLine=ShowStatusLine;
   attr  *a=Upper->a;

   OldTabSize=TabSize;

   fx=curr->x;
   fy=curr->y;

   size=0;
   for(p=opt; p->name; p++)
   {
      if(p->type==STR)
         size+=p->maxlen;
      else if(p->type==NUM || p->type==ONE || p->type==MANY)
         size+=sizeof(int);
   }
   if(!(opt->old=(char*)malloc(size)))
   {
      NotMemory();
      return;
   }
   size=0;
   for(p=opt; p->name; p++)
   {
      p->old=opt->old+size;
      if(p->type==STR)
      {
         size+=p->maxlen;
         strcpy((char*)(p->old),(char*)(p->var));
      }
      else if(p->type==NUM || p->type==ONE || p->type==MANY)
      {
         size+=sizeof(int);
         *(int*)(p->old) = *(int*)(p->var);
      }
   }

   for(p=opt; p->name; p++)
   {
      switch(p->type)
      {
      case(ONE):
         PutStr(p->x-1,p->y,"[ ]");
         DisplayItem(p->x+3,p->y,p->name,DIALOGUE_WIN_ATTR);
         break;
      case(MANY):
         PutStr(p->x-1,p->y,"( )");
         DisplayItem(p->x+3,p->y,p->name,DIALOGUE_WIN_ATTR);
         break;
      case(STR):
         if(p->flags&FRIGHT)
            DisplayItem(p->x+p->len+2,p->y,p->name,DIALOGUE_WIN_ATTR);
         else
            DisplayItem(p->x-ItemLen(p->name)-2,p->y,p->name,DIALOGUE_WIN_ATTR);
         PutStr(p->x-1,p->y,"[");
         PutStr(p->x+p->len,p->y,"]");
         break;
      case(NUM):
         PutStr(p->x-1,p->y,"[  ]");
         DisplayItem(p->x+4,p->y,p->name,DIALOGUE_WIN_ATTR);
         break;
      }
   }
   for(;;)
   {
      for(p=opt; p->name; p++)
      {
         SetAttr(a);
         switch(p->type)
         {
         case(ONE):
            if(*(int*)(p->var))
               PutCh(p->x,p->y,'X');
            else
               PutCh(p->x,p->y,' ');
            break;
         case(MANY):
            if(*(int*)(p->var)==GetNo(p,opt))
               PutCh(p->x,p->y,'*');
            else
               PutCh(p->x,p->y,' ');
            break;
         case(STR):
            if(p==curr)
               sprintf(s,"%-*.*s",p->len,p->len,(char*)(p->var)+shift);
            else
               sprintf(s,"%-*.*s",p->len,p->len,(char*)(p->var));
            if(curr==p)
               SetAttr(CURR_BUTTON_ATTR);
            PutStr(p->x,p->y,s);
            break;
         case(BUTTON):
            DisplayItem(p->x,p->y,p->name,curr==p?CURR_BUTTON_ATTR:DIALOGUE_WIN_ATTR);
            break;
         case(NUM):
            sprintf(s,"%-2d",*(int*)(p->var));
            if(curr==p)
               SetAttr(CURR_BUTTON_ATTR);
            PutStr(p->x,p->y,s);
            break;
         }
      }
      if(OldShowStatusLine==SHOW_BOTTOM && ShowStatusLine==SHOW_BOTTOM)
         StatusLine();
      switch(curr->type)
      {
         case(BUTTON):
            move(LINES-1,COLS-1);
            curs_set(0);
            break;
         case(ONE):
         case(MANY):
            curs_set(1);
            GotoXY(curr->x,curr->y);
            break;
         case(STR):
            curs_set(1);
            GotoXY(curr->x+pos-shift,curr->y);
            break;
         case(NUM):
            curs_set(1);
            GotoXY(curr->x+pos,curr->y);
            break;
      }
      action=GetNextAction();
use_key:
      if(action==-1)
         continue;
      switch(action)
      {
      case(NEWLINE):
      case(CANCEL):
         goto leave_cycle;
      case(EDITOR_HELP):
         if(SetupHelp)
            Help(SetupHelp,SetupTitle);
         break;
      case(CHAR_LEFT):
      case(CHAR_RIGHT):
         if(curr->type==STR)
            break;
      case(LINE_UP):
      case(LINE_DOWN):
         if(action==LINE_UP || action==LINE_DOWN)
            fy=curr->y;
         else
            fx=curr->x;
         dist=30000; /* a large value */
         n=NULL;
         for(p1=opt; p1->name; p1++)
         {
            if(p1!=curr)
            {
               d=GetDist(p1,action);
               if(d<dist)
                 dist=d,n=p1;
            }
         }
         if(n)
         {
            curr=n,newitem=1;
            if(action==LINE_UP || action==LINE_DOWN)
               fy=curr->y;
            else
               fx=curr->x;
         }
         break;
      default:
         if(StringTypedLen!=1)
         {
            if(curr->type==STR)
               break;
            action=EatKey(action);
            goto use_key;
            break;
         }
         key=StringTyped[0];
         if(key==9)
         {
            curr++;
            if(curr->name==NULL)
               curr=opt;
            newitem=1;
            fx=curr->x;
            fy=curr->y;
            break;
         }
         if(curr->type==STR)
            break;
         if(curr->type==NUM && isdigit(key))
            break;
         if(key==' ')
         {
            switch(curr->type)
            {
            case(ONE):
               *(int*)(curr->var)=!*(int*)(curr->var);
               break;
            case(MANY):
               *(int*)(curr->var)=GetNo(curr,opt);
               break;
            }
            break;
         }
         key=toupper(key);
         for(p1=opt; p1->name; p1++)
         {
            if(ItemChar(p1->name)==key)
            {
               curr=p1;
               newitem=1;
               fx=curr->x;
               fy=curr->y;
               if(p1->type==BUTTON)
               {
                  action=NEWLINE;
                  strcpy(StringTyped,"\n");
               }
               else if(p1->type==ONE || p1->type==MANY)
               {
                  action=NO_ACTION;
                  strcpy(StringTyped," ");
               }
               else
                  break;
               goto use_key;
            }
         }
      }
      if(newitem)
      {
         newitem=0;
         shift=0;
         pos=0;
         first=1;
         continue;
      }
      switch(curr->type)
      {
      case(NUM):
         if(StringTypedLen==1 && isdigit(key=StringTyped[0]))
         {
            if(pos==0)
            {
               pos=1;
               *(int*)(curr->var)=key-'0';
            }
            else
            {
               *(int*)(curr->var) = *(int*)(curr->var)*10+key-'0';
               pos=0;
               CorrectParameters();
            }
         }
         break;
      case(STR):
         switch(action)
         {
         case(CHAR_LEFT):
            if(pos>0)
               pos--;
            break;
         case(CHAR_RIGHT):
            if(pos<(int)strlen((char*)(curr->var)))
               pos++;
            break;
         case(LINE_BEGIN):
            pos=0;
            break;
         case(LINE_END):
            pos=strlen((char*)(curr->var));
            break;
         case(BACKSPACE_CHAR):
            if(pos==0)
               break;
            if(shift>0)
               shift--;
            pos--;
         case(DELETE_CHAR):
            for(i=pos; ((char*)(curr->var))[i]; i++)
               ((char*)(curr->var))[i]=((char*)(curr->var))[i+1];
            break;
         case(DELETE_TO_EOL):
            ((char*)(curr->var))[pos]=0;
            break;
         case(ENTER_CHAR_CODE):
            key=getcode();
            goto do_insert;
         case(CHOOSE_CHAR):
            key=choose_ch();
            goto do_insert;
         case(ENTER_CONTROL_CHAR):
            key=GetRawKey();
            goto do_insert;
         default:
            if(StringTypedLen!=1)
            {
               action=EatKey(action);
               goto use_key;
            }
            key=StringTyped[0];
            if(key>=0 && key<' ')
               break;
         do_insert:
            if(key<=0 || key=='\n')
               break;
            if(first)
               ((char*)(curr->var))[0]=0;
            if((int)strlen((char*)(curr->var))>=curr->maxlen-1)
               break;
            for(i=strlen((char*)(curr->var)); i>=pos; i--)
               ((char*)(curr->var))[i+1]=((char*)(curr->var))[i];
            ((char*)(curr->var))[pos++]=key;
         }
         first=0;
         if(pos-shift>=curr->len)
            shift=pos-curr->len+1;
         if(pos-shift<0)
            shift=pos;
      }
   }
leave_cycle:
   if(action==CANCEL)
   {
esc:  for(p=opt; p->name; p++)
      {
         if(p->type==STR)
            strcpy((char*)(p->var),(char*)(p->old));
         else if(p->type==NUM || p->type==ONE || p->type==MANY)
            *(int*)(p->var) = *(int*)(p->old);
      }
   }
   else
   {
      flag=1;
      stdcol=GetCol();
      if(curr->type==BUTTON)
      {
         if(HandleButton(curr->name)==CANCEL)
            goto esc;
      }
   }
   free(opt->old);
   CorrectParameters();
   curs_set(0);
   idlok(stdscr,useidl);
}

void  Dialogue(struct opt *opt,int WinWidth,int WinHeight,char *WinTitle,
             char **SetupHelp[],char *SetupTitle,
             int (*EatKey)(int),int (*HandleButton)(char *))
{
   WIN *optw;
   optw=CreateWin(MIDDLE,MIDDLE,WinWidth,WinHeight,DIALOGUE_WIN_ATTR,WinTitle,0);
   DisplayWin(optw);

   W_Dialogue(opt,SetupHelp,SetupTitle,EatKey,HandleButton);

   CloseWin();
   DestroyWin(optw);
}

int    OptEatKey(int k)
{
   if(k==SAVE_FILE)
   {
      SaveConf(InitName);
      return(NEWLINE);
   }
   if(k==SAVE_FILE_AS)
   {
      SaveOpt();
      return(NEWLINE);
   }
   return(-1);
}

int    OptHandleBut(char *n)
{
   (void)n;
   return(0);
}

void  Options()
{
   extern  char   **SetupHelp[];
   Dialogue(opt,_WinWidth,_WinHeight," Options ",SetupHelp," Setup Help ",OptEatKey,OptHandleBut);
}

void  SaveOpt()
{
#ifndef __MSDOS__
   SaveConf(".le.ini");
#else
   SaveConf("le.ini");
#endif
}
void  UpdtOpt()
{
   SaveConf(InitName);
}
int   TOEatKey(int k)
{
   (void)k;
   return(-1);
}
int   TOHandleBut(char *button_name)
{
   (void)button_name;
   return(0);
}
void  TermOpt(void)
{
   Dialogue(TOpt,70,11," Terminal Options ",NULL,NULL,TOEatKey,TOHandleBut);
}
void  FormatOptions(void)
{
   Dialogue(FormatOpt,30,10," Format Options ",NULL,NULL,TOEatKey,TOHandleBut);
}
void  AppearOpt(void)
{
   Dialogue(AppearOptTbl,26,11," Appearence Options ",NULL,NULL,TOEatKey,TOHandleBut);
}

static bg,fg,c_bold,c_rev,c_ul,c_dim,b_bold,b_rev,b_ul,b_dim;

void  EditColor(color *cp,color *bp)
{
   struct opt color_opt[]=
   {
      {"Black",	  MANY,&fg,4,3},
      {"Green",	  MANY,&fg,4,4},
      {"Red",	  MANY,&fg,4,5},
      {"Yellow",  MANY,&fg,4,6},
      {"Blue",	  MANY,&fg,4,7},
      {"Cyan",	  MANY,&fg,4,8},
      {"Magenta", MANY,&fg,4,9},
      {"White",	  MANY,&fg,4,10},

      {"Black",	  MANY,&bg,19,3},
      {"Green",	  MANY,&bg,19,4},
      {"Red",	  MANY,&bg,19,5},
      {"Yellow",  MANY,&bg,19,6},
      {"Blue",	  MANY,&bg,19,7},
      {"Cyan",	  MANY,&bg,19,8},
      {"Magenta", MANY,&bg,19,9},
      {"White",	  MANY,&bg,19,10},

      {"Bold",	  ONE,&c_bold,	 34,3},
      {"Reverse", ONE,&c_rev, 	 34,4},
      {"Underline",ONE,&c_ul,	 34,5},
      {"Dim",	  ONE,&c_dim,	 34,6},

      {"Bold",	  ONE,&b_bold,	 49,3},
      {"Reverse", ONE,&b_rev, 	 49,4},
      {"Underline",ONE,&b_ul,	 49,5},
      {"Dim",	  ONE,&b_dim,	 49,6},

      {NULL}
   };

   static int color_xlat[]={COLOR_BLACK,COLOR_GREEN,COLOR_RED,COLOR_YELLOW,
			    COLOR_BLUE,COLOR_CYAN,COLOR_MAGENTA,COLOR_WHITE};

   for(bg=0; bg<8 && bg!=color_xlat[cp->bg]; bg++);
   for(fg=0; fg<8 && fg!=color_xlat[cp->fg]; fg++);

   c_bold=!!(cp->attr&A_BOLD);
   c_rev =!!(cp->attr&A_REVERSE);
   c_ul  =!!(cp->attr&A_UNDERLINE);
   c_dim =!!(cp->attr&A_DIM);
   b_bold=!!(bp->attr&A_BOLD);
   b_rev =!!(bp->attr&A_REVERSE);
   b_ul  =!!(bp->attr&A_UNDERLINE);
   b_dim =!!(bp->attr&A_DIM);

   WIN *w=CreateWin(MIDDLE,MIDDLE+5,66,13,DIALOGUE_WIN_ATTR," Edit color ");
   DisplayWin(w);
   PutStr(3,2, "Foreground");
   PutStr(18,2,"Background");
   PutStr(33,2,"Color options");
   PutStr(48,2,"B/W attributes");

   W_Dialogue(color_opt,NULL,NULL,TOEatKey,TOHandleBut);

   CloseWin();
   DestroyWin(w);

   cp->fg=color_xlat[fg];
   cp->bg=color_xlat[bg];
   cp->attr=(c_bold?A_BOLD:0)|(c_rev?A_REVERSE:0)|(c_ul?A_UNDERLINE:0)|(c_dim?A_DIM:0);
   bp->attr=(b_bold?A_BOLD:0)|(b_rev?A_REVERSE:0)|(b_ul?A_UNDERLINE:0)|(b_dim?A_DIM:0);
}

void  ColorsOpt()
{
   color new_color_pal[MAX_COLOR_NO+1];
   color new_bw_pal[MAX_COLOR_NO+1];

   memcpy(new_color_pal,color_pal,sizeof(new_color_pal));
   memcpy(new_bw_pal,bw_pal,sizeof(new_bw_pal));

   WIN *color_win=CreateWin(MIDDLE,MIDDLE,40,22,MENU_ATTR," Select color to tune ");
   DisplayWin(color_win);

   static struct menu m[]=
   {
      {"Normal text                         ",2,1},
      {"Block text                          ",2,2},
      {"Status line                         ",2,3},
      {"Scroll bar                          ",2,4},
      {"Error window                        ",2,5},
      {"Verify window                       ",2,6},
      {"Help window                         ",2,7},
      {"Dialogue window                     ",2,8},
      {"Menu                                ",2,9},
      {"Current button                      ",2,10},
      {"Disabled button                     ",2,11},
      {"Shadow                              ",2,12},
      {"Syntax 1                            ",2,13},
      {"Syntax 2                            ",2,14},
      {"Syntax 3                            ",2,15},
      {"---",2,FDOWN-5},
      {"&Save colors                         ",2,FDOWN-4},
      {"Save as &terminal specific           ",2,FDOWN-3},
      {"&Use colors for this session         ",2,FDOWN-2},
      {"&Cancel                            ^X",2,FDOWN-1},
      {0}
   };

   bool applied=false;
   int curr=0;
   for(;;)
   {
      int res=ReadMenu(m,VERT,MENU_ATTR,CURR_BUTTON_ATTR,curr);
      if(res<0)
      {
	 curr=-res-1;
	 static int color_xlat[]={
	    NORMAL_TEXT,BLOCK_TEXT,STATUS_LINE,SCROLL_BAR,ERROR_WIN,VERIFY_WIN,
	    HELP_WIN,DIALOGUE_WIN,MENU_WIN,CURR_BUTTON,DISABLED_ITEM,SHADOWED,
	    SYNTAX1,SYNTAX2,SYNTAX3};
	 assert(unsigned(curr)<sizeof(color_xlat)/sizeof(color_xlat[0]));
	 int color_no=color_xlat[curr];
	 EditColor(FindColor(new_color_pal,color_no),
		   FindColor(new_bw_pal,color_no));
	 continue;
      }
      if(res=='S' || res=='U' || res=='T')
      {
	 memcpy(color_pal,new_color_pal,sizeof(new_color_pal));
	 memcpy(bw_pal,new_bw_pal,sizeof(new_bw_pal));
	 init_attrs();
      	 applied=true;
      }
      if(res=='S' || res=='T')
      {
	 DescribeColors(bw_pal,color_pal);
	 const char *const n="/.le/colors";
	 char *f=(char*)alloca(strlen(HOME)+strlen(n)+1+strlen(TERM)+1);
	 if(res=='S')
	    sprintf(f,"%s%s",HOME,n);
	 else
	    sprintf(f,"%s%s-%s",HOME,n,TERM);
	 SaveConfToFile(f,colors);
      }
      break;
   }

   CloseWin();
   DestroyWin(color_win);
   if(applied)
   {
      clearok(stdscr,1);
      flag=REDISPLAY_ALL;
      RedisplayAll();
      StatusLine();
   }
}
