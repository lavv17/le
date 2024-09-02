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

static inline int InBlock(offs ptr)
{
   return(!hide && ptr>=BlockBegin && ptr<BlockEnd);
}

int   rInBlock(num line,num col);

static inline int InBlock(offs ptr,num line,num col)
{
   return((rblock && !in_hex_mode) ? rInBlock(line,col) : InBlock(ptr));
}

void    Copy();
void    Move();
int     Delete();
void    Read();
void    Write();

void    Indent();
void    Unindent();
void    DoIndent(int i);
void    DoUnindent(int i);

int     Islower();
int     Isupper();
byte    Toupper();
byte    Tolower();
byte    Inverse();

void    BlockFunc();
int     PipeBlock(const char *cmd,bool in,bool out);
extern  char    BlockFile[256];

void    ConvertToUpper();
void    ConvertToLower();
void    ExchangeCases();

void    BlockType();
void    CheckBlock();

void  PrefixIndent(const char *,num);

int OptionallyConvertBlockNewLines(const char *bname);

extern TextPoint *DragMark;
