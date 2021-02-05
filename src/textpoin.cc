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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include "edit.h"
#include "mb.h"

TextPoint   CurrentPos;
TextPoint   ScreenTop;
TextPoint   TextBegin;
TextPoint   TextEnd;
TextPoint   BlockBegin;
TextPoint   BlockEnd;

static const int cached_array_size=64;
TextPoint   TextPoint::cached_array[cached_array_size];
int	    TextPoint::cached_array_ptr;

TextPoint   *TextPoint::base=NULL;

void  TextPoint::AddTextPoint()
{
   next=base;
   base=this;
   Check();
}
void  TextPoint::DeleteTextPoint()
{
   for(TextPoint**scan=&base; *scan; scan=&((*scan)->next))
   {
      if(*scan==this)
      {
         *scan=(*scan)->next;
         return;
      }
   }
}

TextPoint::TextPoint(offs o)
{
   Init();
   offset=o;
   if(offset<=0)
   {
      offset=0;
      flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
   }
   else
   {
      if(offset>Size())
	 offset=Size();
      flags|=COLUNDEFINED|LINEUNDEFINED;
   }
   AddTextPoint();
}
TextPoint::TextPoint(offs o,num l,num c)
{
   Init();
   offset=o;
   if(l==-1)
   {
      flags=COLUNDEFINED|LINEUNDEFINED;
      line=col=0;
   }
   else if(c==-1)
   {
      flags=COLUNDEFINED;
      line=l;
      col=0;
   }
   else
   {
      flags=0;
      line=l;
      col=c;
   }
   AddTextPoint();
}

TextPoint::TextPoint()
{
   Init();
   AddTextPoint();
}
TextPoint::TextPoint(num l,num c)
{
   Init();
   line=l;
   col=c;
   FindOffset();
   AddTextPoint();
}
TextPoint::TextPoint(const TextPoint& tp)
{
   memcpy(this,&tp,sizeof(tp));
   AddTextPoint();
}

TextPoint::~TextPoint()
{
   DeleteTextPoint();
   CacheTextPoint();
}

void TextPoint::CacheTextPoint() const
{
   cached_array[cached_array_ptr++]=*this;
   cached_array_ptr&=(cached_array_size-1);
}

void  TextPoint::FindOffset()
{
   if(line<0)
      line=0,col=0;
   if(col<0)
      col=0;
   if(line==0 && col==0)
   {
      offset=0;
      flags&=~(LINEUNDEFINED|COLUNDEFINED|CHAR_SPLIT);
      return;
   }

   TextPoint   *scan=base;
   TextPoint   *found=NULL;
   for( ; scan; scan=scan->next)
   {
      if(!(scan->flags&LINEUNDEFINED))
      {
         if(!found || abs(this->line-scan->line)<abs(this->line-found->line)
         || (scan->line==this->line && !(scan->flags&(COLUNDEFINED|CHAR_SPLIT))
             && scan->offset<found->offset))
         {
            found=scan;
         }
      }
   }

   offs  o;
   num   l,c;

   if(found)
   {
      if(found->flags&(COLUNDEFINED|CHAR_SPLIT))
      {
         o=LineBegin(found->offset);
         c=0;
      }
      else
      {
         o=found->offset;
         c=found->col;
      }
      l=found->line;
   }
   else
      o=c=l=0;

   if(l>line)
   {
      while(l>line)
      {
         o=PrevLine(o);
         l--;
      }
      c=0;
   }
   else if(l<line)
   {
      if(EofAt(o))
      {
         offset=o;
         line=l;
         col=c;
         flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
	 Check();
         return;
      }
      while(l<line)
      {
         o=LineEnd(o);
         if(EofAt(o) || o+EolSize>Size())
         {
            offset=o;
            line=l;
            col=0;
            flags|=COLUNDEFINED;
            flags&=~(LINEUNDEFINED|CHAR_SPLIT);
            return;
         }
         o+=EolSize;
         l++;
         if(EofAt(o))
         {
            offset=o;
            line=l;
            col=0;
            flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
	    Check();
            return;
         }
      }
      c=0;
   }
   while(c>col)
   {
      if(CharAt(o-1)=='\t' || mb_mode)
      {
         o=LineBegin(o);
         c=0;
         break;
      }
      c--;
      o--;
   }
   while(c<col)
   {
      if(EolAt(o))
         break;
      if(CharAt(o)=='\t')
         c=Tabulate(c);
      else
         c+=CharWidthAt(o);
      o+=CharSizeAt(o);
   }
   col=c;
   offset=o;
   flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
   Check();
}

void  TextPoint::FindLineCol()
{
   if(offset<=0)
   {
      col=line=offset=0;
      flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
      return;
   }

   if(!(flags&(COLUNDEFINED|LINEUNDEFINED)))
      return;

   TextPoint   *found=NULL;
   num	 dist=INT_MAX;
   for(TextPoint *scan=base; scan; scan=scan->next)
   {
      if(!(scan->flags&LINEUNDEFINED))
      {
	 if(!found || abs(this->offset-scan->offset)<dist)
	 {
	    dist=abs(this->offset-scan->offset);
	    found=scan;
	    if(dist==0)
	       break;
	 }
      }
   }

   offs  o;
   num   l,c;
   int char_split=0; // CHAR_SPLIT or 0

   if(found)
   {
      if(found->flags&(COLUNDEFINED|CHAR_SPLIT))
      {
         o=LineBegin(found->offset);
         c=0;
      }
      else
      {
         o=found->offset;
         c=found->col;
	 char_split=(found->flags&CHAR_SPLIT);
      }
      l=found->line;
   }
   else
      o=c=l=0;

   while(o>offset)
   {
      if(BolAt(o) || CharAt(o-1)=='\t' || mb_mode)
         break;
      c--;
      o--;
   }
   if(o>offset)
   {
      o=LineBegin(o);
      c=0;
      char_split=0;
      while(o>offset)
      {
         o=PrevLine(o);
         l--;
      }
   }
   while(o<offset)
   {
      offs next_line=NextLine(o);
      if(next_line>offset || !BolAt(next_line))
	 break;
      o=next_line;
      l++;
      c=0;
      char_split=0;
   }
   while(o<offset && !EofAt(o))
   {
      if(BolAt(o+1))
      {
	 o++;
         l++;
         c=0;
	 char_split=0;
	 continue;
      }
      else if(char_split)
      {
	 o++;
	 if(MBCheckLeftAt(o) && !MBCharInvalid)
	 {
	    c+=MBCharWidth;
	    char_split=0;
	 }
      }
      else if(CharAt(o)=='\t')
      {
         c=Tabulate(c);
	 o++;
      }
      else
      {
	 MBCheckAt(o);
         if(o+MBCharSize>offset)
	 {
	    o=offset;
	    char_split=CHAR_SPLIT;
	    break;
	 }
	 c+=MBCharWidth;
	 o+=MBCharSize;
      }
   }
   col=c;
   line=l;
   offset=o;
   flags&=~(COLUNDEFINED|LINEUNDEFINED|CHAR_SPLIT);
   flags|=char_split;
   Check();
}

const TextPoint& TextPoint::operator+=(num shift)
{
   offs newoffs=offset+shift;
   if(newoffs<0)
      newoffs=0;
   if(newoffs>Size())
      newoffs=Size();
   TextPoint res(newoffs);
   if(!(this->flags&LINEUNDEFINED))
      res.FindLineCol();
   *this=res;
   return(*this);
}

void  TextPoint::ResetTextPoints()
{
   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
   {
      scan->line=scan->col=scan->offset=0;
      scan->flags&=~(LINEUNDEFINED|COLUNDEFINED|CHAR_SPLIT);
   }
}
void  TextPoint::OrFlags(int mask)
{
   mask&=COLUNDEFINED|LINEUNDEFINED;
   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
      if(scan->offset>0)
	 scan->flags|=mask;
}

TextPoint TextPoint::ForcedLineCol(num l,num c)
{
   num old_stdcol=SaveStdCol();
   TextPoint old_pos=CurrentPos;
   HardMove(l,c);
   TextPoint res=CurrentPos;
   CurrentPos=old_pos;
   RestoreStdCol(old_stdcol);
   return res;
}

num TextPoint::Line()
{
   if(flags&LINEUNDEFINED)
      FindLineCol();
   return(line);
}
num TextPoint::Col()
{
   if(flags&(COLUNDEFINED|LINEUNDEFINED))
      FindLineCol();
   return(col);
}

// check for split characters in the offset range
void TextPoint::CheckSplit(offs o1,offs o2)
{
   for(TextPoint *scan=base; scan; scan=scan->next)
   {
      offs o=scan->offset;
      if(o<o1 || o>=o2)
	 continue;
      if(scan->flags&CHAR_SPLIT) {
	 if(MBCheckLeftAt(o) && !MBCharInvalid)
	    scan->flags&=~CHAR_SPLIT;
      } else {
	 if(MBCheckLeftAt(o) && MBCharSplit)
	    scan->flags|=CHAR_SPLIT;
      }
   }
}

#if 0
void TextPoint::Check() const
{
   assert((flags&(COLUNDEFINED|CHAR_SPLIT)) || !((col==0) ^ BolAt(offset)));
}
#endif
