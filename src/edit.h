/*
 * Copyright (c) 1993-2021 by Alexander V. Lukyanov (lav@yars.free.net)
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

#ifndef EDIT_H
#define EDIT_H

#include <wchar.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#ifndef LE_CURSES_BOOL_TYPE
# define LE_CURSES_BOOL_TYPE char
#endif

#if !defined(bool)
# define bool LE_CURSES_BOOL_TYPE
# define bool_redefined 1
#endif
#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
#endif

#ifdef USE_NCURSES_H
# include <ncurses.h>
#elif HAVE_NCURSESW_NCURSES_H
# include <ncursesw/ncurses.h>
#elif HAVE_NCURSESW_CURSES_H
# include <ncursesw/curses.h>
#elif HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_NCURSES_NCURSES_H
# include <ncurses/ncurses.h>
#elif HAVE_NCURSES_CURSES_H
# include <ncurses/curses.h>
#else
# include <curses.h>
#endif

#ifdef bool_redefined
# undef bool
#endif

#define  EMAIL    "lav@yars.free.net"

#undef	lines
#undef	cols
#undef	newline
#undef	newcol

typedef unsigned char   byte;
typedef long            offs;
typedef long            num;

enum { NO_POS=-1L };

#ifdef HAVE_CONFIGMAKE_H
# include <configmake.h>
#endif
#include "textpoin.h"
#include "color.h"
#include "window.h"
#include "rus.h"
#include "user.h"
#include "screen.h"
#include "menu.h"
#include "menu1.h"
#include "cmd.h"
#include "history.h"
#include "file.h"

extern  int inputmode,editmode;
extern  int noreg,match_case;
#define BACKUP_SUFFIX_LEN 16
extern  char   bak[BACKUP_SUFFIX_LEN];
extern  int    TabSize;
extern  int    IndentSize;
extern  int    Scroll,hscroll;
extern  int    insert;
extern  int    autoindent;
extern  int    BackspaceUnindents;
extern  int    makebak;
extern  int    SavePos,SaveHst;
extern  int    rblock;
extern  int    UseColor;
extern  int    UseTabs;
extern  int    PreferPageTop;

/* When useidl==1 then the editor uses
   insert/delete line capability of a terminal */
extern  int     useidl;

extern   InodeHistory   PositionHistory;
extern   InodeInfo      FileInfo;
extern   History        LoadHistory;

extern int FuncKeysNum;

extern  char    Make[256],Shell[256],Run[256],Compile[256],HelpCmd[256],BakPath[256];
extern  char    FileName[256];

extern  mode_t	  FileMode;
extern  int	  file;
extern  bool	  newfile;

extern  int     View;
enum {
   RO_MODE=1,
   TMP_RO_MODE=2
};

extern  int     ascii,right;    /* modifiers for HEX mode */

extern  char    *HOME,*TERM,*DISPLAY;

extern  char    *buffer;
extern  int     modified;
extern  offs    BufferSize;
extern  offs    GapSize;
extern  offs    ptr1,ptr2,oldptr1,oldptr2;
extern  num     stdcol;

extern  int     hide;

extern  int     message_sp;
extern  int     flag;

extern  byte    chset[];

extern int EolSize;
extern char EolStr[3];
void SetEolStr(const char *);

static const char EOL_UNIX[2]="\n";
static const char EOL_DOS[3]="\r\n";
static const char EOL_MAC[2]="\r";

static inline bool EolIs(const char *e) { return !strcmp(EolStr,e); }

extern   int   TabsInMargin;

extern char Program[];

#define MemStep     (0x2000)
#define Tabulate(c) ((( ((c)<0) ? ((c)-TabSize+1) : (c))/TabSize+1)*TabSize)

#define ALARMDELAY  60  /* one minute */

#define OFF     0
#define ON      1

/* edit modes (values for editmode) */
#define EXACT   0
#define TEXT    1
#define HEXM    2

/* input modes (values for inputmode)*/
#define LATIN   0
#define RUSS    1
#define GRAPH   2

#define in_hex_mode  (editmode==HEXM)
#define Text         (editmode==TEXT && !buffer_mmapped)

num   GetCol();
bool  EolAt(offs o);
bool  BolAt(offs o);
bool  Eol();
bool  Bol();

void  DeleteChar();
void  BackSpace();
int   InsertChar(char ch);
int   ReplaceChar(char ch);

void  MoveLeftOverEOL();
void  MoveRightOverEOL();

void  MoveLineCol(num,num);
void  HideDisplay(void);
char  CharAtLC(num,num);
void  NewLine(void);
void  HardMove(num,num);
void  ExpandTab(void);
bool  IsAlNumAt(offs);

long  getcode(const char *prompt);
int   getcode_char();
int   AskToSave();
void  Quit(void);

void  InstallSignalHandlers(void);
void  ReleaseSignalHandlers(void);
void  SuspendEditor();
void  BlockSignals();
void  UnblockSignals();
char *TmpFileName();
char *HupFileName(int sig);

extern bool buffer_mmapped;

int   ReplaceCharMove(byte);
void  ReplaceCharExt(byte);	 // Replace character under cursor with tab
				 // expanding, line appending, etc.
void  ReplaceCharExtMove(byte);	 // Same, but leave cursor after the new char

void        MoveUp(void);
void        MoveDown(void);
void        ToLineBegin(void);
void        ToLineEnd(void);
void        DeleteEOL(void);
void        DeleteLine(void);
int         getstring(const char *prompt,char *buf,int maxlen,History *history=NULL,
                      int *len=NULL,const char *help=NULL,const char *help_title=NULL);
void        FError(const char *filename);
void        NoMemory();
#define NotMemory NoMemory
offs        LineBegin(offs base);
offs        LineEnd(offs base);
char        *GetWord();
int         GetSpace(num amount);

void        EmptyText();
int         LoadFile(char *name);
int         SaveFile(char *name);
int	    ReopenRW();
void	    SavePosition();   // put current pos to history

void        Initialize();
void        Terminate();

void        Edit();

int         LockFile(int fd,bool drop_write_lock);
int         CheckMode(mode_t);
int	    file_check(const char *);	 /* checks existence or ability to create */

void    DeleteToEOL();
void    DeleteToBOL();
void    DrawFrames();
void    ExpandAllTabs();
void	ExpandSpanTabs();
void    Options();
void    ReadConf();
void    editcalc();
void    CorrectParameters();

void  InitCurses();
void  TermCurses();

void    _clrtoeol(void);

offs    NextLine(offs);
offs    PrevLine(offs);
offs    NextNLines(offs,num);
offs    PrevNLines(offs,num);

void    GoToOffset(void);
void    GoToLineNumber(void);
void    GoToLineNum(num);

void  SeekStdCol();
static inline void SetStdCol() { stdcol=NO_POS; }
static inline num GetStdCol() { return stdcol==NO_POS?stdcol=GetCol():stdcol; }
static inline void AddStdCol(num i) { stdcol=GetStdCol()+i; }
static inline num SaveStdCol() { return Text?GetStdCol():NO_POS; }
static inline void RestoreStdCol(num s) { stdcol=s; }

void  CheckWindowResize();

extern  void    Quit(void),
                Options(void),
                HideDisplay(void),Indent(void),Unindent(void),
                FindBlockBegin(void),FindBlockEnd(void),ConvertToLower(void),
                ConvertToUpper(void),ExchangeCases(void),BlockType(void),
                FindMatch(void),DoMake(void),DoRun(void),DoCompile(void),
                DoShell(void),editcalc(void),DrawFrames(void),
                ExpandAllTabs(void),
                TermOpt(void),SaveOpt(void),UpdtOpt(void),AppearOpt(void),
                edit_chset(void),SaveTermOpt(void),GoToLineNumber(void),
                FormatOptions(void),DOS_UNIX(void);

void  PreModify();
int   PreUserEdit();

int   choose_ch();

int   InsertBlock(const char *block,num len,const char *rblock=NULL,num rlen=0);
int   ReplaceBlock(const char *block,num len);
int   CopyBlock(offs from,num len);
int   CopyBlockOver(offs from,num len);
int   ReadBlock(int fd,num len,num *act_read);
int   ReadBlockOver(int fd,num len,num *act_read);
int   ReplaceTextFromFile(int fd,num len,num *act_read);
int   WriteBlock(int fd,offs from,num len,num *act_written);
int   DeleteBlock(num left,num right);
int   GetBlock(char *copy,offs from,num size);
int   Undelete();
void  CheckPoint();
offs  ScanForCharForward(offs start,byte ch);
void  InsertAutoindent(num oldcol);
offs  FindMatch(char op);

void  Help(const char *help,const char *title);
/*void  Help(char ***help,char *title);*/

void  ActivateMainMenu();

num   MarginSizeAt(offs);

void  UnrefKey(int key);

void  InitModifyKeyTables();
int   ModifyKey(int key);

void  define_pairs();
void  InitMenu();

int   CountNewLines(offs start,num size,num *unix_nl=0,num *dos_nl=0,num *mac_nl=0);
void  ConvertFromUnixToDos(offs start,num size);
void  ConvertFromDosToUnix(offs start,num size);

int   Suffix(const char *,const char *);

bool BlockEqAt(offs,const char *,int);

#define STATUS_LINE_ATTR    find_attr(STATUS_LINE)
#define NORMAL_TEXT_ATTR    find_attr(NORMAL_TEXT)
#define BLOCK_TEXT_ATTR     find_attr(BLOCK_TEXT)
#define ERROR_WIN_ATTR      find_attr(ERROR_WIN)
#define VERIFY_WIN_ATTR     find_attr(VERIFY_WIN)
#define CURR_BUTTON_ATTR    find_attr(CURR_BUTTON)
#define HELP_WIN_ATTR       find_attr(HELP_WIN)
#define DIALOGUE_WIN_ATTR   find_attr(DIALOGUE_WIN)
#define MENU_ATTR           find_attr(MENU_WIN)
#define DISABLED_ITEM_ATTR  find_attr(DISABLED_ITEM)
#define SCROLL_BAR_ATTR     find_attr(SCROLL_BAR)
#define SHADOW_ATTR	    find_attr(SHADOWED)

#ifndef __MSDOS__
#define LockEnforce(mode)   ((mode&S_ISGID) && !(mode&S_IXGRP))
#define LockEnforceStrip(mode) ((mode_t)(mode&(~S_ISGID)))
#else
#define LockEnforce(mode)   0
#define LockEnforceStrip(mode) (mode)
#endif

#define  REDISPLAY_ALL     1
#define  REDISPLAY_LINE    2
#define  REDISPLAY_AFTER   4
#define  REDISPLAY_RANGE   8

#include "inline.h"
#include "mb.h"
#include "chset.h"

int   isslash(char);

int write_loop(int fd,const char *ptr,num size,num *written);

static inline bool E_AGAIN(const int e) {
   return (e==EAGAIN || e==EWOULDBLOCK || e==EINTR);
}

void  ProcessDragMark();

#if ! defined __builtin_expect && __GNUC__ < 3
# define __builtin_expect(expr, expected) (expr)
#endif

#endif // EDIT_H
