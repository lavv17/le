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
#include <string.h>
#include <stdlib.h>
#include "edit.h"

attr  *curr_attr;

WIN      *Upper = NULL;

void  Absolute(int *x,int w,int f)
{
   if(*x<SIGN)    /* already absoluted */
      return;
   if(*x&SIGN)    /* offset < 0 */
   {
      if(*x&FRIGHT)        /* MIDDLE */
         *x=(f-w)/2+*x-MIDDLE;
      else
         *x=f-w+*x-FRIGHT; /* RIGHT */
   }
   else        /* offset >= 0 */
   {
      if(*x&MIDDLE)
         *x=(f-w)/2+*x-MIDDLE;   /* MIDDLE */
      else
         *x=f-w+*x-FRIGHT; /* RIGHT */
   }
}

void  Clear()
{
   int      x,y;

   SetAttr(&Upper->a);

   for(y=0; y<Upper->h; y++)
      for(x=0; x<Upper->w; x++)
         mvaddch(y+Upper->y,x+Upper->x,' ');
   if(Upper->w>2 && Upper->h>2)
   {
      PutCh(0,0,ACS_ULCORNER);
      PutCh(Upper->w-1,0,ACS_URCORNER);
      PutCh(0,Upper->h-1,ACS_LLCORNER);
      PutCh(Upper->w-1,Upper->h-1,ACS_LRCORNER);
      for(x=1; x<Upper->w-1; x++)
      {
         PutCh(x,0,ACS_HLINE);
         PutCh(x,Upper->h-1,ACS_HLINE);
      }
      for(y=1; y<Upper->h-1; y++)
      {
         PutCh(0,y,ACS_VLINE);
         PutCh(Upper->w-1,y,ACS_VLINE);
      }
      PutStr(MIDDLE,0,Upper->title);
   }
}
void  PutCh(int x,int y,chtype ch)
{
   Absolute(&x,1,Upper->w);
   Absolute(&y,1,Upper->h);
   if(x>=0 && y>=0 && x<Upper->w && y<Upper->h)
   {
      move(y+Upper->y,x+Upper->x);
      addch_visual(ch);
   }
}
void  GotoXY(int x,int y)
{
   Absolute(&x,1,Upper->w);
   Absolute(&y,1,Upper->h);
   if(x>=0 && y>=0 && x<Upper->w && y<Upper->h)
   {
      move(y+Upper->y,x+Upper->x);
   }
}
void  PutStr(int x,int y,const char *str)
{
   Absolute(&x,strlen(str),Upper->w);
   Absolute(&y,1,Upper->h);

   int   bx=x;

   while(*str && y<Upper->h)
   {
      if(*str=='\n')
      {
         while(x<Upper->w-1)
            mvaddch(y+Upper->y,(x++)+Upper->x,' ');
         x=bx;
         y++;
      }
      else
      {
         if(x>=0 && y>=0 && x<Upper->w)
	 {
            move(y+Upper->y,x+Upper->x);
	    addch_visual((byte)*str);
	 }
	 x++;
      }
      str++;
   }
}

WIN   *CreateWin(int x,int y,unsigned w,unsigned h,struct attr *a,
                 const char *title,int flags)
{
   WIN   *win;

   win=(WIN*)malloc(sizeof(WIN));

   Absolute(&x,w,COLS);
   Absolute(&y,h,LINES);

   if(w>(unsigned)COLS)
      w=COLS;
   if(h>(unsigned)LINES)
      h=LINES;
   if(x<0)
      x=0;
   if(y<0)
      y=0;
   if(x+w>(unsigned)COLS)
      x=COLS-w;
   if(y+h>(unsigned)LINES-1)
      y=LINES-1-h;

   win->x=x;
   win->y=y;
   win->w=w;
   win->h=h;
   win->a=*a;
   win->title=title;
   win->buf=NULL;
   win->flags=flags;

   return(win);
}
void  DestroyWin(WIN *win)
{
   free(win);
}
void  DisplayWin(WIN *win)
{
   chtype   *save;
   int      x,y;

   curs_set(0);

   win->prev=Upper;
   Upper=win;

   save = win->buf = (chtype*)malloc((win->h+1)*(win->w+2)*sizeof(chtype));

   for(y=0; y<win->h+1; y++)
   {
      for(x=0; x<win->w+2; x++)
      {
         *save=mvinch(y+win->y,x+win->x);
         if(!(y<win->h && x<win->w) && !(win->flags&NOSHADOW) && x>1 && y>0)
         {
            if(y+win->y<LINES && x+win->x<COLS)
            {
               attrset(SHADOW_ATTR->attr);
               mvaddch(y+win->y,x+win->x,*save&(A_CHARTEXT|A_ALTCHARSET));
            }
         }
         save++;
      }
   }
   SetAttr(&win->a);

   Clear();
}
void  CloseWin()
{
   chtype   *save;
   int      x,y;

   save = Upper->buf;

   attrset(0);

   for(y=0; y<Upper->h+1; y++)
      for(x=0; x<Upper->w+2; x++,save++)
      {
	 if(y+Upper->y<LINES && x+Upper->x<COLS
	 && x>=0 && y>=0)
   	    mvaddch(y+Upper->y,x+Upper->x,*save);
      }

   free(Upper->buf);
   Upper->buf=0;
   Upper=Upper->prev;
   if(Upper)
      SetAttr(&Upper->a);
}
