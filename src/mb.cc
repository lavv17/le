/*
 * Copyright (c) 2003-2004 by Alexander V. Lukyanov (lav@yars.free.net)
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

/* $Id$ */

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include "edit.h"
#include <mbswidth.h>
#include "mb.h"

bool  mb_mode=true;
#ifdef USE_MULTIBYTE_CHARS
int   MBCharSize=1;
int   MBCharWidth=1;

static int mb_flags=MBSW_ACCEPT_UNPRINTABLE|MBSW_ACCEPT_INVALID;
static char *mb_ptr;
static int mb_len;

static void MB_Prepare(offs o)
{
   if(o>=ptr1)
   {
      mb_ptr=buffer+o+GapSize;
      mb_len=buffer+BufferSize-mb_ptr;
   }
   else if(o+MB_LEN_MAX<=ptr1)
   {
      mb_ptr=buffer+o;
      mb_len=ptr1-o;
   }
   else
   {
      static char tmpbuf[MB_LEN_MAX*2-1];
      mb_len=GetBlock(tmpbuf,o,MB_LEN_MAX*2-1);
      mb_ptr=tmpbuf;
   }
}

bool MBCheckLeftAt(offs o)
{
   if(!mb_mode)
      return false;
   int left_offset=MB_LEN_MAX;
   if(left_offset>o)
      left_offset=o;
   MB_Prepare(o-left_offset);
   if(mb_len>MB_LEN_MAX*2-1)
      mb_len=MB_LEN_MAX*2-1;
   for(int i=0; i<left_offset; i++)
   {
      mblen(0,0);
      MBCharSize=mblen(mb_ptr+i,mb_len-i);
      if(MBCharSize==left_offset-i)
      {
	 MBCharWidth=mbsnwidth(mb_ptr+i,MBCharSize,mb_flags);
	 return true;
      }
      if(MBCharSize>left_offset-i)
      {
	 MBCharWidth=0;
	 return true;
      }
   }
   MBCharSize=1;
   MBCharWidth=1;
   return false;
}

bool MBCheckAt(offs o)
{
   if(!mb_mode)
      return false;
   MB_Prepare(o);
   mblen(0,0);
   MBCharSize=mblen(mb_ptr,mb_len);
   if(MBCharSize<1)
   {
      MBCharSize=1;
      MBCharWidth=0;
      return false;
   }
   MBCharWidth=mbsnwidth(mb_ptr,MBCharSize,mb_flags);
   return true;
}
wchar_t WCharAt(offs o)
{
   if(!MBCheckAt(o))
      return CharAt(o);
   wchar_t wc=-1;
   mbtowc(&wc,mb_ptr,MBCharSize);
   return wc;
}
#endif
