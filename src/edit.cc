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

/* edit.c : main editor loop */

#include <config.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <errno.h>
#include "edit.h"
#include "calc.h"
#include "keymap.h"
#include "block.h"
#include "screen.h"
#ifdef WITH_MOUSE
# include "mouse.h"
#endif

#include <getopt.h>

// #include <term.h>

#ifdef __MSDOS__
#define	 fcntl(x,y,z)	(0)
#endif

#include <xalloca.h>

char  Program[256];

extern  const char    MainHelp[];

char  BakPath[256]="";

char  PosName[256]="";
char  HstName[256]="";

int   SavePos=1;
int   SaveHst=1;

int   View=FALSE;

int   ShowMatchPos=TRUE;

char  *HOME,*TERM,*DISPLAY;

int   UseColor=1;
int   UseTabs=1;
int   IndentSize=8;
int   BackspaceUnindents=1;
int   PreferPageTop=1;

void  GoToLineNum(num line_num)
{
   CurrentPos=TextPoint(line_num,0);
   stdcol=GetCol();
}

int   getcode()
{
   long  i;
   static char ch[10];
   static getcode_active=0;

   if(getcode_active)
      return(-1);

   getcode_active=1;

   if(getstring("Char: ",ch,sizeof(ch)-1)<1)
   {
      getcode_active=0;
      return(-1);
   }
   getcode_active=0;
   i=strtol(ch,0,0);
   if(i<0 || i>255)
      return(-1);
   return((int)i);
}

void ProcessDragMark()
{
   if(CurrentPos<*DragMark)
   {
      if(CurrentPos!=BlockBegin || (Text && stdcol>GetCol()))
      {
	 BlockEnd=*DragMark;
	 UserSetBlockBegin();
      }
   }
   else if(CurrentPos>=*DragMark)
   {
      if(CurrentPos!=BlockEnd || (Text && stdcol>GetCol()))
      {
	 BlockBegin=*DragMark;
	 UserSetBlockEnd();
      }
   }
}

void    Edit()
{
   int      key;
   num      std;
   int      action;
   ActionProc proc;
   num	 old_num_of_lines=-1;

   while(1)
   {
      std=stdcol;
      if(!hex)
      {
         while(GetCol()>std)
           MoveLeft();
         while(GetCol()<std && !Eol())
           MoveRight();
      }
      if(hex)
         flag|=REDISPLAY_LINE;

      if(!hex && !View && old_num_of_lines!=TextEnd.Line())
      {
         flag|=REDISPLAY_AFTER;
         old_num_of_lines=TextEnd.Line();
      }

      if(DragMark)
	 ProcessDragMark();

      SyncTextWin();
      StatusLine();
      SetCursor();

      action=GetNextAction();

      if(action==WINDOW_RESIZE)
      {
	 CorrectParameters();
	 flag|=REDISPLAY_ALL;
      	 continue;
      }

      if(hex)
      {
         ShowMatchPos=0;
         RedisplayLine();
         ShowMatchPos=1;
      }

#ifdef WITH_MOUSE
      if(action==MOUSE_ACTION)
      {
	 MEVENT mev;
	 if(getmouse(&mev)==ERR)
	    continue;
	 if(InTextWin(mev.y,mev.x))
	    MouseInTextWin(mev);
	 else if(InScrollBar(mev.y,mev.x))
	    MouseInScrollBar(mev);
      }
#endif // WITH_MOUSE

      if(action==QUIT_EDITOR)
      {
         Quit();
         continue;
      }

      proc=GetActionProc(EditorActionProcTable,action);
      if(proc)
      {
	 proc();
	 continue;
      }
      if(action!=NO_ACTION)
	 continue;
      if(StringTypedLen>1)
	 continue;
      key=(byte)(StringTyped[0]);
      if(hex && key=='\t')
      {
	 ascii=!ascii;
	 continue;
      }
      if(View)
	 continue;
      if(!ascii && hex)
      {
	 if(key<0 || key>255)
	    continue;
	 if(isdigit(key))
	    key-='0';
	 else
	 {
	    key=toupper(key);
	    if(key>='A' && key<='F')
	       key-='A'-0x0A;
	    else
	    {
	       UnrefKey(key);
	       continue;
	    }
	 }
	 if(((insert && !right) || Eof())
	 && !buffer_mmapped)
	 {
	    InsertChar(0);
	    MoveLeft();
	    flag|=REDISPLAY_AFTER;
	 }
	 if(right)
	 {
	    if(ReplaceCharMove((Char()&0xF0)+key)==OK)
	       right=0;
	 }
	 else
	 {
	    if(ReplaceChar((Char()&0x0F)+(key<<4))==OK)
	       right=1;
	 }
	 flag|=REDISPLAY_LINE;
      }
      else
      {
	 if(key>255 || (key>=0 && key<' ' && key!='\n' && key!='\t'))
	 {
	    UnrefKey(key);
	    continue;
	 }
	 key=ModifyKey(key);
	 if(buffer_mmapped)
	 {
	    if(Eol())
	       flag|=REDISPLAY_AFTER;
	    else
	       flag|=REDISPLAY_LINE;
	    ReplaceCharMove(key);
	    continue;
	 }
	 if(hex)
	 {
	    if(insert)
	    {
	       InsertChar(key);
	       flag|=REDISPLAY_AFTER;
	    }
	    else
	    {
	       ReplaceCharMove(key);
	       flag|=REDISPLAY_LINE;
	    }
	    continue;
	 }
	 switch(key)
	 {
	 case('\n'):
	    UserNewLine();
	    break;
	 case('\t'):
	    UserIndent();
	    break;
	 default:   /* not a newline and not a tab */
	    if(insert || Eol() || (Char()=='\t' && Tabulate(GetCol())!=(GetCol()+1)))
	    {
	       UserInsertChar(key);
	    }
	    else
	    {
	       InsertChar(key);
	       DeleteChar();
	    }
	    flag|=REDISPLAY_LINE;
	 }
      }
   }
}
void    Quit()
{
   if(AskToSave())
      Terminate();
}
int     AskToSave()
{
   if(modified)
   {
      static struct menu Menu[]={
      {"   &Yes   "},{"   &No   "},{" &Cancel "},
      {NULL}};
      int result=TRUE;

      switch(ReadMenuBox(Menu,HORIZ,"The file has been modified. Save?","",
	 VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
      {
      case('Y'):
         errno=0;
         result=(UserSave()==OK);
         if(!result && modified)
         {
            UserSaveAs();
            result=!modified;
         }
         break;
      case(0):
      case('C'):
         result=FALSE;
         break;
      case('N'):
         result=TRUE;
      }
      return(result);
   }
   SavePosition();
   return(TRUE);
}

#if defined(NCURSES_VERSION) || defined(__NCURSES_H)
#define NCUR
#endif

void  InitCurses()
{
#ifdef NCUR
   static bool init=false;

   if(init)
   {
      endwin();
      initscr();  // for some old ncurses
      doupdate();
      return;
   }

   initscr();
   init=true;

#else
   static SCREEN *le_scr=NULL;

   if(le_scr!=NULL)
   {
      delscreen(le_scr);
      del_curterm(cur_term);
   }

   le_scr=newterm(NULL,stdout,stdin);
   if(le_scr==NULL)
   {
      fprintf(stderr,"le: newterm() failed. Check your $TERM variable.\n");
      exit(1);
   }
#endif

   start_color();
#ifdef NCURSES_VERSION_MAJOR
   extern int can_use_default_colors;
   can_use_default_colors = (use_default_colors()==OK);
#endif

   cbreak();
   noecho();
   nonl();
   meta(stdscr,TRUE);
   raw();
   intrflush(stdscr,FALSE);
   keypad(stdscr,TRUE);

   idlok(stdscr,useidl);
   scrollok(stdscr,FALSE);

#ifdef WITH_MOUSE
   mousemask(ALL_MOUSE_EVENTS,0);
#endif
}
void  TermCurses()
{
   move(LINES-1,0);
   clrtoeol();
   refresh();
   endwin();
}

int optUseColor=-1;

void    Initialize()
{
   FILE    *f;

   InitTables();
   initcalc();

#ifndef MSDOS
   char  *filename=(char*)alloca((strlen(HOME)|15)+17);
   sprintf(filename,"%s/.le",HOME);
   mkdir(filename,0700);
   strcat(filename,"/tmp");
   mkdir(filename,0700);
   sprintf(HstName,"%s/.le/history",HOME);
#else
   sprintf(HstName,"%s/le.hst",HOME);
#endif

   InstallSignalHandlers();

   InitCurses();

   ReadConf();

   if(optUseColor!=-1)
      UseColor=optUseColor;

   init_attrs();

   InitMenu();

   struct flock l;
   l.l_type=F_RDLCK;
   l.l_whence=SEEK_SET;
   l.l_start=l.l_len=0;
   Message("Loading history...");
   f=fopen(HstName,"rb");
   if(f && fcntl(fileno(f),F_SETLKW,&l)!=-1)
   {
      LoadHistory.ReadFrom(f);
      SearchHistory.ReadFrom(f);
      PositionHistory.ReadFrom(f);
      fclose(f);
   }

   EditorReadKeymap();
   RebuildKeyTree();
}
void    Terminate()
{
   FILE    *f;

   alarm(0);
   EmptyText();

   curs_set(1);

   if(strcmp(FileName,HstName))
   {
      if(SaveHst)
      {
         Message("Saving history...");
         int fd=open(HstName,O_RDWR|O_CREAT,0644);
         struct flock l;
	 l.l_type=F_RDLCK;
	 l.l_whence=SEEK_SET;
	 l.l_start=l.l_len=0;
         if(fd!=-1 && fcntl(fd,F_SETLKW,&l)!=-1)
         {
            f=fdopen(fd,"r+b");

            History  oldLoadHistory;
            History  oldSearchHistory;
            InodeHistory oldPositionHistory;

            oldLoadHistory.ReadFrom(f);
            oldSearchHistory.ReadFrom(f);
            oldPositionHistory.ReadFrom(f);
            LoadHistory.Merge(oldLoadHistory);
            SearchHistory.Merge(oldSearchHistory);
            PositionHistory.Merge(oldPositionHistory);

            rewind(f);

            LoadHistory.WriteTo(f);
            SearchHistory.WriteTo(f);
            PositionHistory.WriteTo(f);

            fclose(f);
         }
         close(fd);
      }
   }

   TermCurses();

   exit(0);
}

const char version_string[]="Text editor LE version " VERSION;

void  PrintUsage(int arg)
{
   (void)arg;
   printf("Usage: le [OPTIONS] [FILES...]\n"
	  "\n"
	  "-r  --read-only    permanent read only mode (view)\n"
	  "-h  --hex-mode     start in hex mode\n"
	  "-b  --black-white  black and white mode\n"
	  "-c  --color        color mode\n"
	  "    --dump-keymap  dump default keymap to stdout and exit\n"
	  "    --dump-colors  dump default color map to stdout and exit\n"
	  "    --mmap         load file using mmap (read only)\n"
	  "    --mmap-rw      mmap read-write. Use with extreme caution,\n"
	  "                   expecially on your hard disk! No undo, all\n"
	  "                   changes go directly to file/disk.\n"
	  "    --help         this description\n"
	  "    --version      print LE version\n"
	  "\n"
	  "The last file will be loaded. If no files specified, last readable file\n"
	  "from history will be loaded if the path is relative or it is the last.\n");
   exit(1);
}

int     main(int argc,char **argv)
{
   int   optView=-1,opteditmode=-1,optWarpLine=0;
   int	 opt_use_mmap=-1;
   int   opt;

   enum {
      DUMP_KEYMAP=1024,
      DUMP_COLORS,
      PRINT_HELP,
      PRINT_VERSION,
      USE_MMAP,
      USE_MMAP_RW
   };

   static struct option le_options[]=
   {
      {"help",no_argument,0,PRINT_HELP},
      {"version",no_argument,0,PRINT_VERSION},
      {"dump-keymap",no_argument,0,DUMP_KEYMAP},
      {"dump-colors",no_argument,0,DUMP_COLORS},
      {"read-only",no_argument,0,'r'},
      {"hex-mode",no_argument,0,'h'},
      {"black-white",no_argument,0,'b'},
      {"color",no_argument,0,'c'},
#ifdef HAVE_MMAP
      {"mmap",no_argument,0,USE_MMAP},
      {"mmap-rw",no_argument,0,USE_MMAP_RW},
#endif
      {0,0,0,0}
   };

   HistoryLine    *hl=0;
   char  newname[256];

   strcpy(Program,le_basename(argv[0]));

#ifdef __MSDOS__
    HOME=".";
    TERM="ansi";
    _fmode=O_BINARY;
#else
   setlocale(LC_ALL,"");

   HOME=getenv("HOME");
   if(HOME==NULL)
   {
      fprintf(stderr,"Cannot get the value of HOME\n\r");
      return(1);
   }
   TERM=getenv("TERM");
   if(TERM==NULL)
   {
      fprintf(stderr,"Cannot get the value of TERM\n\r");
      return(1);
   }
   DISPLAY=getenv("DISPLAY");
#endif

   while((opt=getopt_long(argc,argv,"rhbc",le_options,0))!=-1)
   {
      switch(opt)
      {
      case('r'):
         optView=1;
#ifdef HAVE_MMAP
	 opt_use_mmap=1;
#endif
         break;
      case('h'):
         opteditmode=HEXM;
         break;
      case('b'):
         optUseColor=0;
         break;
      case('c'):
	 optUseColor=1;
	 break;
      case('?'):
	 fprintf(stderr,"%s: Try `%s --help' for more information\n",Program,argv[0]);
	 exit(1);
      case(DUMP_KEYMAP):
	 WriteActionMap(stdout);
	 exit(0);
      case(DUMP_COLORS):
	 DumpDefaultColors(stdout);
	 exit(0);
      case(PRINT_HELP):
	 PrintUsage(0);
	 exit(0);
      case(PRINT_VERSION):
	 printf("%s - %s\n",Program,version_string);
	 exit(0);
      case(USE_MMAP):
	 opt_use_mmap=1;
	 if(optView==-1)
	    optView=2;
	 else
	    optView|=2;
	 opteditmode=HEXM;
	 break;
      case(USE_MMAP_RW):
	 opt_use_mmap=1;
	 if(optView!=-1)
	    optView&=~2;
	 opteditmode=HEXM;
      	 break;
      }
   }
   if(optUseColor!=-1)
      UseColor=optUseColor;

   Initialize();

   if(optView!=-1)
      View=!!optView;
   if(opteditmode!=-1)
      editmode=opteditmode;
   if(optUseColor!=-1)
      UseColor=optUseColor;
   if(opt_use_mmap!=-1)
      buffer_mmapped=opt_use_mmap;

   if(optind<argc-1 && argv[optind][0]=='+' && isdigit(argv[optind][1]))
   {
      optWarpLine=atoi(argv[optind]);
      optind++;
   }

   if(optind>=argc)
   {
      LoadHistory.Open();
      bool first=true;
      for(;;)
      {
	 hl=LoadHistory.Prev();
	 if(!hl)
	    break;
	 if(hl->line[0] && (first || hl->line[0]!='/')
	 && access(hl->line,R_OK)!=-1)
	 {
	    strcpy(newname,hl->line);
      	    break;
	 }
	 first=false;
      }

      if(!hl)
      {
         ShowAbout();
         if(getstring("Load: ",newname,255,&LoadHistory,NULL,NULL)<1
                                             || ChooseFileName(newname)<0)
            Terminate();
         HideAbout();
      }
   }
   else
   {
      for(; optind<argc; optind++)
         LoadHistory+=argv[optind];
      sprintf(newname,"%.255s",argv[argc-1]);
      if(ChooseFileName(newname)<0)
	 newname[0]=0;
   }
   if(newname[0] && file_check(newname)==ERR)
   {
      if(View)
	 Terminate();
      newname[0]=0;
   }
   if(LoadFile(newname)!=ERR)
   {
      if(optWarpLine>0)
	 GoToLineNum(optWarpLine-1);
   }
   Edit();
   Terminate();
   return 0;
}
