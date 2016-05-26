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

#define MAX_FAIL_COUNT   100

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#else
#include <poll.h>
#endif

#ifdef __linux__
#  include <linux/keyboard.h>
#  if HAVE_LINUX_TIOCL_H
#    include <linux/tiocl.h>
#  endif
   static int linux_process_key(int);
   static int ungetstr(const char *str);
#endif

#include "edit.h"
#include "getch.h"

sigjmp_buf getch_return;
bool  getch_return_set=false;

static int fail_count=0;
static void fail()
{
   if(++fail_count>=MAX_FAIL_COUNT)
      raise(SIGHUP);
}

void    UnrefKey(int key) // ???
{
    if(iscntrl(key))
    {
        napms(100);
        flushinp();
    }
}


int   GetRawKey()
{
   int   key;

   UnblockSignals();

   timeout(-1);
   keypad(stdscr,0);
   key=getch();
   if(key==ERR)
      fail();
   else
      fail_count=0;
   keypad(stdscr,1);

   BlockSignals();

   return(key);
}

int   CheckPending()
{
   struct pollfd pfd;
   pfd.fd=0;
   pfd.events=POLLIN;
   return poll(&pfd,1,0);
}

int   WaitForKey(int delay)
{
   int key=GetKey(delay);

#ifdef WITH_MOUSE
   if(key==KEY_MOUSE)
   {
      MEVENT mev;
      if(getmouse(&mev)==OK)
	 ungetmouse(&mev);
   }
   else
#endif
   if(key!=ERR)
      ungetch(key);

   return(key);
}

int   GetKey(int delay)
{
   if(sigsetjmp(getch_return,1)==0)
   {
      getch_return_set=true;
      UnblockSignals();

      bkgdset(NORMAL_TEXT_ATTR->n_attr|' ');   // recent ncurses uses bkgd for default clearing
      timeout(delay);
      int key=getch();
      if(key==ERR)
      {
	 if(delay==-1)
	    fail();
      }
      else
	 fail_count=0;
      timeout(-1);
      bkgdset(' ');

      BlockSignals();
      getch_return_set=false;

      /* on linux try to interpret shift state */
#ifdef __linux__
      key=linux_process_key(key);
#endif

      return key;
   }
   else
   {
      getch_return_set=false;
      return ERR;
   }
}

#ifdef __linux__
/* I hate it, it does not work over telnet */
/* Oh why linux cannot just return different codes for different keys? */

# ifndef TIOCL_GETSHIFTSTATE
#  define TIOCL_GETSHIFTSTATE 6 // older linux versions did not define this
# endif

int linux_process_key(int key)
{
   /* BEWARE OF UNWANTED RECURSION! */
#ifdef TIOCLINUX
   char shift_state=TIOCL_GETSHIFTSTATE;
   if(ioctl(0,TIOCLINUX,&shift_state)<0)
      return key;

   bool shift=(shift_state & (1<<KG_SHIFT));
   bool ctrl =(shift_state & (1<<KG_CTRL));
   bool alt  =(shift_state & (1<<KG_ALT));

   if(!shift && !ctrl && !alt)
      return key;

   // improve function keys a bit...
   {
      int add=12;
      if(ctrl)
	 add=24;
      if(alt || (shift && ctrl))
	 add=36;
      if(shift && key>=KEY_F0+11 && key<=KEY_F0+20)
	 return key+add-10;	// ~F11 and ~F12 lose
      else if(key>=KEY_F0+1 && key<=KEY_F0+12)
	 return key+add;
   }
   // some xterm key sequences are used below.
   int xterm_shift=0;
   if(shift)	     xterm_shift=2;
   if(ctrl)	     xterm_shift=5;
   if(shift && ctrl) xterm_shift=6;
   int code=0;
   char str[16];
   switch(key)
   {
   case KEY_LEFT:
      code='D';
      break;
   case KEY_RIGHT:
      code='C';
      break;
   case KEY_UP:
      code='A';
      break;
   case KEY_DOWN:
      code='B';
      break;
   case KEY_HOME:
      code='H';
      break;
   case KEY_END:
      code='F';
      break;
   }
   if(code)
   {
      sprintf(str,"\033[1;%d%c",xterm_shift,code);
      return ungetstr(str);
   }
   code=0;
   switch(key)
   {
   case KEY_IC:
      code=2;
      break;
   case KEY_DC:
      code=3;
      break;
   case KEY_PPAGE:
      code=5;
      break;
   case KEY_NPAGE:
      code=6;
      break;
   }
   if(code)
   {
      sprintf(str,"\033[%d;%d~",code,xterm_shift);
      return ungetstr(str);
   }

#endif // TIOCLINUX
   return key;
}

static int ungetstr(const char *str)
{
   int len=strlen(str);
   if(len==0)
      return ERR;
   const char *scan=str+len-1;
   while(scan>str)
      ungetch(*scan--);
   return *scan;
}

#endif
