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
#include <poll.h>
#include <errno.h>
#include "edit.h"

void    UnrefKey(int key) // ???
{
    if(iscntrl(key))
    {
        poll(NULL,0L,100);
        flushinp();
    }
}

#ifdef __MSDOS__
static DosMultiByteKey=0;
#endif

int   GetRawKey()
{
   int   key;

   UnblockSignals();

#ifndef __MSDOS__
   byte  ch;
   if(read(0,&ch,1)!=1)
      key=ERR;
   else
      key=ch;
#else
   key=bdos(7,0,0)&255;
   if(key==0 && DosMultiByteKey==0)
      DosMultiByteKey=1;
   else
      DosMultiByteKey=0;
#endif

   BlockSignals();

   return(key);
}

int   WaitForKey(int delay)
{
   int   res;

#ifdef __MSDOS__
   if(DosMultiByteKey>0)
      return(OK);
#endif

   UnblockSignals();

   errno=0;

   struct pollfd  pfd;
   pfd.fd=0;
   pfd.events=POLLIN;
   res=(poll(&pfd,1,0)!=1 || !(pfd.revents&POLLIN))?ERR:OK;
   if(res==ERR && errno!=EINTR)
   {
      refresh();
      fflush(stdout);
      res=(poll(&pfd,1,delay)!=1 || !(pfd.revents&POLLIN))?ERR:OK;
   }

   BlockSignals();

   return(res);
}
