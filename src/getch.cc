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

#define FAILCOUNT   10

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#include "edit.h"
#include "getch.h"

sigjmp_buf getch_return;
bool  getch_return_set=false;

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

      timeout(delay);
      int key=getch();
      timeout(-1);

      BlockSignals();
      getch_return_set=false;

      return key;
   }
   else
   {
      getch_return_set=false;
      return ERR;
   }
}
