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
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <setjmp.h>
#include "edit.h"

#ifndef __MSDOS__
#include <termios.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#endif

#include "xalloca.h"

int   resize_flag=0;

void  BlockSignals()
{
   sigset_t ss;

   sigemptyset(&ss);
   sigaddset(&ss,SIGALRM);
#ifdef SIGWINCH
   sigaddset(&ss,SIGWINCH);
#endif

   sigprocmask(SIG_BLOCK,&ss,NULL);
}
void  UnblockSignals()
{
   sigset_t ss;

   sigemptyset(&ss);
   sigaddset(&ss,SIGALRM);
#ifdef SIGWINCH
   sigaddset(&ss,SIGWINCH);
#endif

   sigprocmask(SIG_UNBLOCK,&ss,NULL);
}

void  CheckWindowResize()
{
#if !defined(__MSDOS__) && defined(TIOCGWINSZ)
   struct winsize winsz;
   static disable_resize=0;

   resize_flag=0;

   if(disable_resize)
      return;

   winsz.ws_col=COLS;
   winsz.ws_row=LINES;
   ioctl(0,TIOCGWINSZ,&winsz);
   if(winsz.ws_col && winsz.ws_row
   && (winsz.ws_col!=COLS || winsz.ws_row!=LINES)
   && !(getenv("LINES") && getenv("COLUMNS")))
   {
      WIN *w=CreateWin(0,0,COLS,LINES,NORMAL_TEXT_ATTR,"",NOSHADOW);
      DisplayWin(w);
      InitCurses(); // this does endwin() automatically
      if(winsz.ws_col!=COLS || winsz.ws_row!=LINES)
      {
	 beep();
         disable_resize=1;
      }
      clearok(stdscr,TRUE);
      CorrectParameters();
      flag|=REDISPLAY_ALL;
      CloseWin();
      DestroyWin(w);
   }
#endif
}

void  resize_sig(int sig)
{
   (void)sig;
   resize_flag=1;

   extern bool getch_return_set;
   extern sigjmp_buf getch_return;

   if(getch_return_set)
      siglongjmp(getch_return,1);
}

void    SuspendEditor()
{
#ifndef __MSDOS__
/*    clear();*/
    curs_set(1);
    refresh();
    endwin();
    kill(getpid(),SIGSTOP);
    refresh();
#else
   ErrMsg("Suspending is not supported under MSDOG");
#endif
}

static char mem[4000];
char *TmpFileName()
{
#ifndef __MSDOS__
   sprintf(mem,"%s/.le/tmp/",HOME);
   char *add=mem+strlen(mem);
   strcpy(add,FileName);
   while(*add)
   {
      if(isslash(*add))
	 *add='_';
      add++;
   }
   sprintf(add,".%d",(int)getpid());
#else
   sprintf(mem,"le%d.res",(int)getpid());
#endif
   return mem;
}
char *HupFileName(int sig)
{
#ifndef __MSDOS__
   sprintf(mem,"%s/.le/tmp/DUMP-%d-",HOME,sig);
   char *add=mem+strlen(mem);
   strcpy(add,FileName);
   while(*add)
   {
      if(isslash(*add))
	 *add='_';
      add++;
   }
   sprintf(add,".%d",(int)getpid());
#else
   sprintf(mem,"le%d.hup",(int)getpid());
#endif
   return mem;
}

void    hup(int sig)
{
   endwin();

   if(modified)
   {
      char *s=HupFileName(sig);
      fprintf(stderr,"le: Caught signal %d, dumping text to %s\n",sig,s);
      int fd=creat(s,0600);
      num act_written;
      WriteBlock(fd,0,Size(),&act_written);
      close(fd);
   }
   else
   {
      fprintf(stderr,"le: Caught signal %d\n",sig);
   }
   exit(1);
}

void    alarmsave(int a)
{
   static offs dump_pos=0;
   static int fd=-1;
   static int interrupted=0;
   const chunk=0x20000;

   (void)a;

   // check if the text changed
   if(modified==1)
   {
      // it did - reset dump state
      dump_pos=0;
      if(fd!=-1)
      {
	 interrupted++;
	 close(fd);
      }
      char *s=TmpFileName();
      fd=open(s,O_CREAT|O_WRONLY|O_TRUNC,0600);
      if(fd==-1)
      {
	 alarm(ALARMDELAY);
	 return;
      }
      modified=3;
   }
   // if the dump is in progress
   if(modified==3)
   {
      num  act_written;
      if(WriteBlock(fd,dump_pos,(interrupted>5?Size():chunk),&act_written)!=OK)
      {
      done:
	 close(fd);
	 fd=-1;
	 // mark it as dumped
	 modified=2;
      }
      else
      {
	 dump_pos+=act_written;
      	 if(dump_pos>=Size())
	 {
	    interrupted=0;
	    goto done;
	 }
	 // after a second write next chunk
      	 alarm(1);
	 return;
      }
   }
   alarm(ALARMDELAY);
}

/* This pair of functions is to work
   with signal handlers - install and release */
struct sigaction  OldSIGHUP;
struct sigaction  OldSIGINT;
struct sigaction  OldSIGQUIT;
struct sigaction  OldSIGTSTP;

void    InstallSignalHandlers()
{
   struct sigaction  hupaction;
#ifdef __GNUC__
#define  SA_HANDLER_TYPE   typeof(OldSIGHUP.sa_handler)
#else
#define  SA_HANDLER_TYPE   void(*)(int)
#endif
#ifndef SA_RESTART
#define SA_RESTART 0
#endif

   struct sigaction  alarmsaveaction;
   struct sigaction  ign_action;
   struct sigaction  suspend_action;
   struct sigaction  resize_action;

   hupaction.sa_handler=(SA_HANDLER_TYPE)hup;
   hupaction.sa_flags=0;
   sigfillset(&hupaction.sa_mask);

   alarmsaveaction.sa_handler=(SA_HANDLER_TYPE)alarmsave;
   alarmsaveaction.sa_flags=SA_RESTART;
   sigemptyset(&alarmsaveaction.sa_mask);

   ign_action.sa_handler=(SA_HANDLER_TYPE)SIG_IGN;
   ign_action.sa_flags=0;

   suspend_action.sa_handler=(SA_HANDLER_TYPE)SuspendEditor;
   suspend_action.sa_flags=0;
   sigemptyset(&suspend_action.sa_mask);

   resize_action.sa_handler=(SA_HANDLER_TYPE)resize_sig;
   resize_action.sa_flags=0;
   sigemptyset(&resize_action.sa_mask);

   BlockSignals();

   /* catch signals to dump editing file and exit */
   sigaction(SIGHUP,&hupaction,&OldSIGHUP);

#ifndef DEBUG
   sigaction(SIGILL,&hupaction,NULL);
#ifdef SIGTRAP
   sigaction(SIGTRAP,&hupaction,NULL);
#endif
   sigaction(SIGABRT,&hupaction,NULL);
#ifdef SIGEMT
   sigaction(SIGEMT,&hupaction,NULL);
#endif
   sigaction(SIGFPE,&hupaction,NULL);
#if SIGBUS
   sigaction(SIGBUS,&hupaction,NULL);
#endif
   sigaction(SIGSEGV,&hupaction,NULL);
#ifdef SIGSYS
   sigaction(SIGSYS,&hupaction,NULL);
#endif
#endif /* DEBUG */

   sigaction(SIGTERM,&hupaction,NULL);
#ifdef SIGPWR
   sigaction(SIGPWR,&hupaction,NULL);
#endif

   sigaction(SIGALRM,&alarmsaveaction,NULL);

   sigaction(SIGINT,&ign_action,&OldSIGINT);
   sigaction(SIGQUIT,&ign_action,&OldSIGQUIT);
#ifdef SIGTSTP
   sigaction(SIGTSTP,&suspend_action,&OldSIGTSTP);
#endif
#ifdef SIGVTALARM
   sigaction(SIGVTALRM,&ign_action,NULL);
#endif
#ifdef SIGPIPE
   sigaction(SIGPIPE,&ign_action,NULL);
#endif
   sigaction(SIGUSR1,&ign_action,NULL);
   sigaction(SIGUSR2,&ign_action,NULL);

#ifdef SIGWINCH
   sigaction(SIGWINCH,&resize_action,NULL);
#endif
#ifdef SIGCHLD
   sigaction(SIGCHLD,&ign_action,NULL);
#endif
#ifdef SIGTTIN
   sigaction(SIGTTIN,&suspend_action,NULL);
#endif
#ifdef SIGTTOU
   sigaction(SIGTTOU,&suspend_action,NULL);
#endif
}
void    ReleaseSignalHandlers()
{
   struct sigaction  dfl_action;

   dfl_action.sa_handler=(SA_HANDLER_TYPE)SIG_DFL;
   dfl_action.sa_flags=0;

   alarm(0);   /* turn off alarm */

   sigaction(SIGHUP,&OldSIGHUP,NULL);
   sigaction(SIGILL,&dfl_action,NULL);
#ifdef SIGTRAP
   sigaction(SIGTRAP,&dfl_action,NULL);
#endif
   sigaction(SIGABRT,&dfl_action,NULL);
#ifdef SIGEMT
   sigaction(SIGEMT,&dfl_action,NULL);
#endif
   sigaction(SIGFPE,&dfl_action,NULL);
#ifdef SIGBUS
   sigaction(SIGBUS,&dfl_action,NULL);
#endif
   sigaction(SIGSEGV,&dfl_action,NULL);
#ifdef SIGSYS
   sigaction(SIGSYS,&dfl_action,NULL);
#endif
   sigaction(SIGTERM,&dfl_action,NULL);
#ifdef SIGPWR
   sigaction(SIGPWR,&dfl_action,NULL);
#endif

   sigaction(SIGALRM,&dfl_action,NULL);

   sigaction(SIGINT,&OldSIGINT,NULL);
   sigaction(SIGQUIT,&OldSIGQUIT,NULL);
#ifdef SIGTSTP
   sigaction(SIGTSTP,&OldSIGTSTP,NULL);
#endif
#ifdef SIGVTALRM
   sigaction(SIGVTALRM,&dfl_action,NULL);
#endif
#ifdef SIGPIPE
   sigaction(SIGPIPE,&dfl_action,NULL);
#endif
   sigaction(SIGUSR1,&dfl_action,NULL);
   sigaction(SIGUSR2,&dfl_action,NULL);
//   sigaction(SIGWINCH,&dfl_action,NULL);

   UnblockSignals();
}
