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

int     InBlock(offs ptr,num line=0,num col=0);
int     GetBlock(void);
void    PutOut(num,num);
extern num block_width,block_height;
extern char **block;

void    RCopy(void);
void    RMove();
void    RDelete();
void    Copy();
void    Move();
void    Delete();
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
int     PipeBlock(char *cmd,int in,int out);
extern  char    BlockFile[256];

void    ConvertToUpper();
void    ConvertToLower();
void    ExchangeCases();

void    BlockType();
void    CheckBlock();

void  PrefixIndent(char *,num);
