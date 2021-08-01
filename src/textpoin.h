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

#ifndef TEXTPOINT_H
#define TEXTPOINT_H

#include <string.h>

#define  COLUNDEFINED      1
#define  LINEUNDEFINED     2
#define  CHAR_SPLIT	   4

class TextPoint
{
   offs  offset;
   num   line,col;
   int   flags;
   static TextPoint cached_array[];
   static int cached_array_ptr;

   TextPoint   *next;
   static TextPoint   *base;

   void  AddTextPoint();
   void  DeleteTextPoint();

   void  FindOffset();
   void  FindLineCol();

   void  Init() {
      offset=line=col=flags=0;
   }

public:
   offs  Offset() const
   {
      return(offset);
   }
   num   Line();
   num   Col();

   num	 LineSimple() const
   {
      if(flags&LINEUNDEFINED)
         return -1;
      return(line);
   }
   num	 ColSimple() const
   {
      if(flags&(COLUNDEFINED|LINEUNDEFINED))
         return -1;
      return(col);
   }

   TextPoint();
   TextPoint(offs);
   TextPoint(num,num);
   TextPoint(const TextPoint&);
   TextPoint(offs,num,num);

   void CacheTextPoint() const;

   ~TextPoint();

   const TextPoint& operator=(const TextPoint& tp)
   {
      offset=tp.offset;
      line=tp.line;
      col=tp.col;
      flags=tp.flags;
      return(*this);
   }
   const TextPoint& operator+=(num shift);
   const TextPoint& operator-=(num shift)
   {
      return(*this+=-shift);
   }
   operator offs() const
   {
      return(offset);
   }

   static TextPoint ForcedLineCol(num l,num c);

   static   void  ResetTextPoints();
   static   void  OrFlags(int mask);
   static   void  CheckSplit(offs,offs);

   friend   int   InsertBlock(const char *,num,const char *,num);
   friend   int   DeleteBlock(num,num);
   friend   int   ReplaceBlock(const char *,num);

   void Check() const {};
};

extern TextPoint  CurrentPos;
extern TextPoint  ScreenTop;
extern TextPoint  BlockBegin;
extern TextPoint  BlockEnd;
extern TextPoint  TextEnd;
extern TextPoint  TextBegin;

#define ScrPtr  ScreenTop.Offset()
#define ScrLine ScreenTop.Line()

#endif
