/*
 * Copyright (c) 2003-2006 by Alexander V. Lukyanov (lav@yars.free.net)
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

#ifdef USE_MULTIBYTE_CHARS
bool  mb_mode=false;
int   MBCharSize=1;
int   MBCharWidth=1;
bool  MBCharInvalid=false;

#define REPLACEMENT_CHARACTER 0xFFFD

// static int mb_flags=MBSW_ACCEPT_UNPRINTABLE|MBSW_ACCEPT_INVALID;
static const char *last_mb_ptr;
static int last_mb_len;
static wchar_t last_wc;

static void MB_Prepare(offs o)
{
   if(o>=ptr1)
   {
      last_mb_ptr=buffer+o+GapSize;
      last_mb_len=buffer+BufferSize-last_mb_ptr;
   }
   else if(o+MB_LEN_MAX<=ptr1)
   {
      last_mb_ptr=buffer+o;
      last_mb_len=ptr1-o;
   }
   else
   {
      static char tmpbuf[MB_LEN_MAX*2-1];
      last_mb_len=GetBlock(tmpbuf,o,MB_LEN_MAX*2-1);
      last_mb_ptr=tmpbuf;
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
   if(last_mb_len>MB_LEN_MAX*2-1)
      last_mb_len=MB_LEN_MAX*2-1;
   for(int i=0; i<left_offset; i++)
   {
      mbtowc(0,0,0);
      last_wc=-1;
      MBCharInvalid=false;
      MBCharSize=mbtowc(&last_wc,last_mb_ptr+i,last_mb_len-i);
      if(MBCharSize<=0)
      {
	 MBCharSize=1;
	 MBCharInvalid=true;
      }
      if(MBCharSize==left_offset-i)
      {
	 MBCharWidth=wcwidth(visualize_wchar(last_wc));
	 if(MBCharWidth<0)
	 {
	    MBCharWidth=1;
	    MBCharInvalid=true;
	 }
	 return true;
      }
      if(MBCharSize>left_offset-i)
      {
	 MBCharSize=left_offset-i;
	 MBCharWidth=0;
	 MBCharInvalid=true;
	 return true;
      }
   }
   MBCharSize=1;
   MBCharWidth=1;
   MBCharInvalid=true;
   return false;
}

bool MBCheckAt(offs o)
{
   if(!mb_mode)
      return false;
   MBCharInvalid=false;
   MB_Prepare(o);
   mbtowc(0,0,0);
   last_wc=-1;
   MBCharSize=mbtowc(&last_wc,last_mb_ptr,last_mb_len);
   if(MBCharSize<1)
   {
      MBCharInvalid=true;
      MBCharSize=1;
      MBCharWidth=1;
      return false;
   }
   MBCharWidth=wcwidth(visualize_wchar(last_wc));
   if(MBCharWidth<0)
   {
      MBCharInvalid=true;
      MBCharWidth=1;
   }
   return true;
}
wchar_t WCharAt(offs o)
{
   if(!MBCheckAt(o))
      return CharAt(o);
   return last_wc==-1?REPLACEMENT_CHARACTER:last_wc;
}
wchar_t WCharLeftAt(offs o)
{
   if(!MBCheckLeftAt(o))
      return CharAt(o-1);
   return last_wc==-1?REPLACEMENT_CHARACTER:last_wc;
}

void mb_get_col(const char *buf,int pos,int *col,int len)
{
   if(!mb_mode)
   {
      *col=pos;
      return;
   }
   *col=0;
   for(int i=0; i<pos; )
   {
      mbtowc(0,0,0);
      wchar_t wc=REPLACEMENT_CHARACTER;
      int ch_len=mbtowc(&wc,buf+i,len-i);
      if(ch_len<1)
	 ch_len=1;
      wc=visualize_wchar(wc);
      *col+=wcwidth(wc);
      i+=ch_len;
   }
}
void mb_char_left(const char *buf,int *pos,int *col,int len)
{
   if(!mb_mode)
   {
      *col=--*pos;
      return;
   }
   *col=0;
   for(int i=0; i<*pos; )
   {
      mbtowc(0,0,0);
      wchar_t wc=REPLACEMENT_CHARACTER;
      int ch_len=mbtowc(&wc,buf+i,len-i);
      if(ch_len<1)
	 ch_len=1;
      if(i+ch_len>=*pos)
      {
	 *pos=i;
	 return;
      }
      wc=visualize_wchar(wc);
      *col+=wcwidth(wc);
      i+=ch_len;
   }
}
void mb_char_right(const char *buf,int *pos,int *col,int len)
{
   if(!mb_mode)
   {
      *col=++*pos;
      return;
   }
   wchar_t wc=REPLACEMENT_CHARACTER;
   mbtowc(0,0,0);
   int ch_len=mbtowc(&wc,buf+*pos,len-*pos);
   if(ch_len<1)
      ch_len=1;
   wc=visualize_wchar(wc);
   int ch_width=wcwidth(wc);
   *pos+=ch_len;
   *col+=ch_width;
}
int mb_get_pos_for_col(const char *buf,int target_col,int len)
{
   int pos=0;
   int col=0;
   while(pos<len && col<target_col)
      mb_char_right(buf,&pos,&col,len);
   return pos;
}
int mb_len(const char *buf,int len)
{
   mblen(0,0);
   int ch_len=mblen(buf,len);
   if(ch_len<1)
      ch_len=1;
   return ch_len;
}
wchar_t mb_to_wc(const char *buf,int len,int *ch_len_out,int *ch_width)
{
   if(!mb_mode)
   {
      if(ch_len_out)
	 *ch_len_out=1;
      if(ch_width)
	 *ch_width=1;
      return *buf;
   }
   wchar_t wc=REPLACEMENT_CHARACTER;
   mbtowc(0,0,0);
   int ch_len=mbtowc(&wc,buf,len);
   if(ch_len<1)
      ch_len=1;
   if(ch_len_out)
      *ch_len_out=ch_len;
   if(ch_width)
   {
      wc=visualize_wchar(wc);
      *ch_width=wcwidth(wc);
   }
   return wc;
}
void InsertWChar(wchar_t ch)
{
   char buf[MB_CUR_MAX+1];
   int len=wctomb(buf,ch);
   if(len<=0)
      return;
   InsertBlock(buf,len);
}
void ReplaceWCharMove(wchar_t ch)
{
   char buf[MB_CUR_MAX+1];
   int len=wctomb(buf,ch);
   if(len<=0)
      return;
   MBCheckRight();
   if(MBCharSize!=len)
   {
      DeleteBlock(0,MBCharSize);
      InsertBlock(buf,len);
   }
   else
   {
      ReplaceBlock(buf,len);
      CurrentPos+=len;
   }
}
#endif
