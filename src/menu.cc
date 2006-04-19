/*
 * Copyright (c) 1993-2005 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#include "menu.h"

char  ItemChar(const char *i)
{
   for(; *i; i++)
      if(*i=='&')
         return(toupper(i[1]));
   return(0);
}
int   ItemLen(const char *i)
{
   int   len=0;
   for(; *i; i++)
      if(*i!='&' || !i[1])
         len++;
   return(len);
}
void  DisplayItem(int x,int y,const char *i,const attr *a)
{
   Absolute(&x,ItemLen(i),Upper->w);
   if(!strcmp(i,"---"))
   {
      int w=Upper->w;
      SetAttr(a);
      while(x<w-2)
	 PutACS(x++,y,HLINE);
      return;
   }
   attr r=*a;
   r.n_attr=a->so_attr;
   for(; *i; i++)
   {
      if(*i=='&')
      {
	 SetAttr(&r);
         PutCh(x++,y,(byte)(*++i));
      }
      else
      {
	 SetAttr(a);
         PutCh(x++,y,(byte)(*i));
      }
   }
   SetAttr(a);
}

void  display(const struct menu *mi,const attr *a)
{
   DisplayItem(mi->x,mi->y,mi->text,a);
}

int   ReadMenu(const struct menu *m,int dir,const attr *a,const attr *ca,int curr)
{
   int   i,action,key;

   curs_set(0);
   do
   {
      for(i=0; m[i].text; i++)
      {
         if(i!=curr)
            display(&m[i],a);
         else
            display(&m[i],ca);
      }
      move(LINES-1,COLS-1);
      action=GetNextAction();
      switch(action)
      {
      case(CHAR_LEFT):
         if(dir==HORIZ)
         {
	    if(curr==0)
               while(m[curr].text)
                  curr++;
            curr--;
         }
         break;
      case(CHAR_RIGHT):
         if(dir==HORIZ)
         {
right:         curr++;
            if(m[curr].text==NULL)
               curr=0;
         }
         break;
      case(LINE_UP):
         if(dir==VERT)
         {
	    for(;;)
	    {
	       if(curr==0)
		  while(m[curr].text)
		     curr++;
	       curr--;
	       if(strcmp(m[curr].text,"---"))
	       	  break;
	    }
	 }
         break;
      case(LINE_DOWN):
         if(dir==VERT)
         {
	    for(;;)
	    {
	       curr++;
	       if(m[curr].text==NULL)
		  curr=0;
	       if(strcmp(m[curr].text,"---"))
	       	  break;
	    }
	 }
         break;
      case(CANCEL):
         return(0);
      case(NEWLINE):
	 i=ItemChar(m[curr].text);
	 if(i)
	    return i;
         return(-1-curr);
      default:
         if(StringTypedLen!=1)
            break;
         if(StringTyped[0]==9)
            goto right;
         key=toupper(StringTyped[0]);
         for(i=0; m[i].text; i++)
            if(key==ItemChar(m[i].text))
               return(key);
      }
   }
   while(1);
/*NOTREACHED*/
}

void  GetTextGeometry(const char *s,int *w,int *h)
{
   int	 x;

   *w=0;
   *h=0;
   while(*s)
   {
      (*h)++;
      x=0;
      while(*s && *s!='\n')
	 x++,s++;
      if(*s)
	 s++;
      if(x>*w)
	 *w=x;
   }
}

int   ReadMenuBox(struct menu *m,int dir,const char *msg,const char *title,
		  const attr *a,const attr *a1)
{
   int	 w,h;
   int	 len;
   int	 pos;
   int	 i;

   GetTextGeometry(msg,&w,&h);

   if(dir==HORIZ)
   {
      len=-2;
      for(i=0; m[i].text; i++)
      {
	 len+=ItemLen(m[i].text)+2;
	 m[i].y=FDOWN-2;
      }
      if(len>w)
	 w=len;
      if(w<COLS/4)
	 w=COLS/4;
      if(h>0)
	 h+=2;
      else
	 h+=1;
      pos=(w-len)/2;
      for(i=0; m[i].text; i++)
      {
	 m[i].x=pos+2;
	 pos+=ItemLen(m[i].text)+2;
      }
   }
   else
      abort();

   WIN	 *win=CreateWin(MIDDLE,MIDDLE,w+4,h+4,a,title);
   DisplayWin(win);
   PutStr(2,2,msg);
   int res=ReadMenu(m,dir,a,a1);
   CloseWin();
   DestroyWin(win);

   return(res);
}
