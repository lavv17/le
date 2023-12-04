/*
 * Copyright (c) 1993-2004 by Alexander V. Lukyanov (lav@yars.free.net)
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

static inline
byte  CharAt(offs offset)
{
   if(offset>=ptr1)
      offset+=GapSize;
   if(offset>=0 && offset<BufferSize)
      return(buffer[offset]);
   return(' ');
}
// same as above, but without bound check
static inline
byte  CharAt_NoCheck(offs offset)
{
   if(offset>=ptr1)
      offset+=GapSize;
   return(buffer[offset]);
}
static inline
offs    Size()
{
   return(BufferSize-GapSize);
}
static inline
offs    Offset()
{
   return(CurrentPos.Offset());
}
static inline
int Eof()
{
   return(Offset()>=Size());
}
static inline
int EofAt(offs o)
{
   return(o>=Size());
}
static inline
int Bof()
{
   return(Offset()<=0);
}
static inline
int BofAt(offs o)
{
   return(o<=0);
}
static inline
byte    Char()
{
   return(CharAt(Offset()));
}
static inline
byte    CharRel(offs sh)
{
   return(CharAt(Offset()+sh));
}
static inline
byte    CharRel_NoCheck(offs sh)
{
   return(CharAt_NoCheck(Offset()+sh));
}
static inline
bool  IsAlNumRel(offs sh)
{
   return IsAlNumAt(Offset()+sh);
}

#include "mb.h"

static inline
bool  IsAlNumLeft()
{
   return IsAlNumAt(Offset()-CharSizeLeft());
}
static inline
void    MoveRight()
{
   CurrentPos+=1;
}
static inline
void    MoveLeft()
{
   CurrentPos-=1;
}
static inline
num     GetLine()
{
   return(CurrentPos.Line());
}
static inline
bool le_isspace(int c)
{
   return c==' ' || c=='\t';
}
static inline
bool Space()
{
   return le_isspace(WChar());
}
static inline
bool SpaceLeft()
{
   return le_isspace(WCharLeft());
}
static inline
bool SpaceLeftAt(offs pos)
{
   return le_isspace(WCharLeftAt(pos));
}

static inline bool BlockEqLeftAt(offs o,const char *s,int len) { return BlockEqAt(o-len,s,len); }
static inline bool BlockEq(const char *s,int len) { return BlockEqAt(Offset(),s,len); }
static inline bool BlockEqLeft(const char *s,int len) { return BlockEqLeftAt(Offset(),s,len); }

static inline int xstrcmp(const char *s1,const char *s2)
{
   if(s1==s2)
      return 0;
   if(s1==0 || s2==0)
      return 1;
   return strcmp(s1,s2);
}
static inline int xstrncmp(const char *s1,const char *s2,size_t len)
{
   if(s1==s2 || len==0)
      return 0;
   if(s1==0 || s2==0)
      return 1;
   return strncmp(s1,s2,len);
}
static inline int xstrcasecmp(const char *s1,const char *s2)
{
   if(s1==s2)
      return 0;
   if(s1==0 || s2==0)
      return 1;
   return strcasecmp(s1,s2);
}
static inline size_t xstrlen(const char *s)
{
   if(s==0)
      return 0;
   return strlen(s);
}
