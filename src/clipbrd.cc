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

/* $Id$ */

#include <config.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "edit.h"
#include "clipbrd.h"

ClipBoard MainClipBoard;

ClipBoard::ClipBoard()
{
   text=0;
   width=0;
   height=0;
}
ClipBoard::~ClipBoard()
{
   if(text)
      free(text);
}

void ClipBoard::Empty()
{
   if(text)
      free(text);
   text=0;
   width=0;
   height=0;
}

int ClipBoard::Copy()
{
   Empty();

   if(rblock)
   {
      rect=true;
      toeol=false;

      num     i,j;
      num     line1=BlockBegin.Line();
      num     line2=BlockEnd.Line();
      num     col1=BlockBegin.Col();
      num     col2=BlockEnd.Col();

      if(col1==col2)
      {
	 toeol=true;
	 for(i=line1; i<=line2; i++)
	 {
	    GoToLineNum(i);
	    ToLineEnd();
	    if(GetCol()>col2)
	       col2=GetCol();
	 }
      }
      width=col2-col1;
      height=line2-line1+1;
      text=(char*)malloc(width*height+1);
      if(!text)
      {
	 NotMemory();
	 return(false);
      }
      TextPoint bb(line1,col1);
      num c=bb.Col();
      num l=bb.Line();
      offs o=bb.Offset();
      char *text_ptr=text;
      for(j=0; j<height; j++)
      {
	 i=0;
	 while(l<line1+j)
	 {
	    if(EofAt(o))
	       goto fill_space;
	    o=NextLine(o);
	    if(!BolAt(o))
	       goto fill_space;
	    l++;
	    c=0;
	 }
	 while(c<col1)
	 {
	    if(EolAt(o))
	       goto next_line;
	    if(CharAt_NoCheck(o)=='\t')
	    {
	       num c1=Tabulate(c);
	       if(c1>col1)
	       	  break;
	       c=c1;
	    }
	    else
	       c++;
	    o++;
	 }
	 while(i<width)
	 {
	    if(EolAt(o))
	       goto next_line;
	    byte ch=CharAt_NoCheck(o);
	    if(ch=='\t')
	    {
	       num c1=Tabulate(c);
	       int k=c1-c;
	       if(c<col1)
		  k-=(col1-c);
	       if(k>(width-i))
		  k=(width-i);
	       i+=k;
	       while(k-->0)
		  *text_ptr++=' ';
	       c=c1;
	       o++;
	       continue;
	    }
	    c++;
	    o++;
	    i++;
	    *text_ptr++=ch;
	 }
      next_line:;
	 while(i<width)
	 {
	    i++;
	    *text_ptr++=' ';
	 }
      }
   fill_space:
      // fill the rest with spaces
      int rest=width*height-(text_ptr-text);
      if(rest>0)
	 memset(text_ptr,' ',rest);
   }
   else /* !rblock */
   {
      rect=false;

      num i=BlockBegin,end=BlockEnd;
      width=end-i;
      height=1;
      text=(char*)malloc(width*height);
      if(!text)
      {
	 NotMemory();
	 return(false);
      }
      GetBlock(text,BlockBegin,width);
   }
   return(true);
}

int ClipBoard::Paste(bool mark)
{
   int res=OK;

   if(!text)
      return true;

   if(!PreUserEdit())
      return false;

   if(rect)
   {
      num l=GetLine();
      num c=GetCol();
      int i;

      // if the block is unlimited on the right and there is some text to the
      // right of current position, insert blank lines to make place for block
      if(toeol)
      {
	 for(i=0; i<height; i++)
	 {
	    MoveLineCol(l+i,c);
	    while(!Eol() && (Char()==' ' || Char()=='\t'))
	       MoveRight();
	    if(!Eol())
	    {
	       GoToLineNum(l);
	       for(i=0; i<height; i++)
		  NewLine();
	       break;
	    }
	 }
      }

      if(mark)
      {
         HardMove(l,c);
	 hide=1;
	 BlockBegin=CurrentPos;
      }
      for(i=0; i<height; i++)
      {
	 HardMove(l+i,c);
	 num ll=width;
	 if(Eol())
	 {
	    while(ll>0 && text[i*width+ll-1]==' ')
	       ll--;
	 }
	 res=InsertBlock(&text[i*width],ll);
	 if(res!=OK)
	    return false;
      }
      if(mark)
	 HardMove(BlockBegin.Line()+height-1,BlockBegin.Col()+(toeol?0:width));
   }
   else
   {
      if(mark)
	 BlockBegin=CurrentPos;
      res=InsertBlock(text,width*height);
      if(res!=OK)
         return false;
   }
   if(mark)
   {
      BlockEnd=CurrentPos;
      rblock=rect;
      hide=0;
   }

   stdcol=GetCol();
   return true;
}

int ClipBoard::Write(int fd)
{
   if(!text)
      return 0;
   for(int i=0; i<height; i++)
   {
      num ll=width;
      while(ll>0 && text[i*width+ll-1]==' ')
	 ll--;
      if(write(fd,&text[i*width],ll)!=ll)
	 return -1;
      if(write(fd,EolStr,EolSize)!=EolSize)
	 return -1;
   }
   return 0;
}

int ClipBoard::Linearize(char **buf,int *len)
{
   if(!text)
      return 0;
   *buf=(char*)malloc((width+EolSize)*height+1);
   if(!*buf)
   {
      NotMemory();
      return 0;
   }
   char *store=*buf;
   *len=0;
   for(int i=0; i<height; i++)
   {
      num ll=width;
      while(ll>0 && text[i*width+ll-1]==' ')
	 ll--;
      memcpy(store,&text[i*width],ll);
      *len+=ll;
      store+=ll;
      memcpy(store,EolStr,EolSize);
      *len+=EolSize;
      store+=EolSize;
   }
   return 1;
}
