/* 
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

static inline
byte  CharAt(register offs offset)
{
   if(offset>=ptr1)
      offset+=GapSize;
   if(offset>=0 && offset<BufferSize)
      return(buffer[offset]);
   return(' ');
}
// same as above, but without bound check
static inline
byte  CharAt_NoCheck(register offs offset)
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
num     GetCol()
{
    return(CurrentPos.Col());
}
static inline
num     GetLine()
{
    return(CurrentPos.Line());
}
static inline
void    DeleteChar()
{
   DeleteBlock(0,1);
}
static inline
void    BackSpace()
{
   DeleteBlock(1,0);
}
static inline
int EolAt(offs o)
{
   return((CharAt(o)==EolStr[0] && (EolSize<2 || CharAt(o+1)==EolStr[1]))
      || EofAt(o));
}
static inline
int BolAt(offs o)
{
   return((CharAt(o-EolSize)==EolStr[0] && (EolSize<2 || CharAt(o-1)==EolStr[1]))
      || BofAt(o));
}
static inline
int Eol()
{
   return(EolAt(CurrentPos));
}
static inline
int Bol()
{
   return(BolAt(CurrentPos));
}
static inline
void   InsertChar(char ch)
{
   InsertBlock(&ch,1);
}
