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

#include <config.h>
#include <stdlib.h>
#include <unistd.h>
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

      num     i,j;
      num     line1=BlockBegin.Line();
      num     line2=BlockEnd.Line();
      num     col1=BlockBegin.Col();
      num     col2=BlockEnd.Col();

      if(col1==col2)
      {
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
      text=(char*)malloc(width*height);
      if(!text)
      {
	 NotMemory();
	 return(false);
      }
      for(j=0; j<height; j++)
      {
	 for(i=0; i<width; i++)
	 {
	    byte ch=CharAtLC(line1+j,col1+i);
	    if(ch=='\t')
	       ch=' ';
	    text[j*width+i]=ch;
	 }
      }
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
      char *s=text;
      while(i<end)
	 *s++=CharAt_NoCheck(i++);
   }
   return(true);
}

int ClipBoard::Paste()
{
   int res=OK;

   if(!text)
      return true;

   PreUserEdit();

   if(rect)
   {
      num l=GetLine();
      num c=GetCol();
      for(int i=0; i<height; i++)
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
	    break;
      }
   }
   else
   {
      res=InsertBlock(text,width*height);
   }
   return res==OK;
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
   *buf=(char*)malloc(width*height);
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
