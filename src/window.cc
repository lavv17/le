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

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "xalloca.h"

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

   SetAttr(Upper->a);

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
void  PutWCh(int x,int y,wchar_t ch)
{
   Absolute(&x,1,Upper->w);
   Absolute(&y,1,Upper->h);
   if(x>=0 && y>=0 && x<Upper->w && y<Upper->h)
   {
      move(y+Upper->y,x+Upper->x);
      wchar_t vch=visualize_wchar(ch);
      if(vch!=ch)
	 attrset(curr_attr->so_attr);
      int w=wcwidth(vch);
      if(w==0)
      {
	 wchar_t a[2];
	 a[0]=' ';
	 a[1]=vch;
	 addnwstr(a,2);
      }
      else
	 addnwstr(&vch,1);
      attrset(curr_attr->n_attr);
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

#if USE_MULTIBYTE_CHARS
   int len=strlen(str);
   wchar_t *wstr=(wchar_t*)alloca((len+1)*sizeof(wchar_t));
   memset(wstr,0,(len+1)*sizeof(wchar_t));
   mbstowcs(wstr,str,len);

   while(*wstr && y<Upper->h)
   {
      attrset(curr_attr->n_attr);
      if(*wstr=='\n')
      {
         while(x<Upper->w-1)
            mvaddch(y+Upper->y,(x++)+Upper->x,' ');
         x=bx;
         y++;
      }
      else if(*wstr=='\t')
      {
	 int add=((x-bx+8)&~7)-x+bx;
	 while(add-->0)
	 {
	    if(x<Upper->w)
	       mvaddch(y+Upper->y,x+Upper->x,' ');
	    x++;
	 }
      }
      else
      {
	 wchar_t wc=visualize_wchar(*wstr);
	 int width=wcwidth(wc);
	 if(x>=0 && y>=0 && x+width<=Upper->w)
	 {
            move(y+Upper->y,x+Upper->x);
	    if(wc!=*wstr)
	       attrset(curr_attr->so_attr);
	    addnwstr(&wc,1);
	    x=getcurx(stdscr)-Upper->x;
	 }
	 else
	    x+=width;
      }
      wstr++;
   }
#else // !USE_MULTIBYTE_CHARS
   while(*str && y<Upper->h)
   {
      if(*str=='\n')
      {
         while(x<Upper->w-1)
            mvaddch(y+Upper->y,(x++)+Upper->x,' ');
         x=bx;
         y++;
      }
      else if(*str=='\t')
      {
	 int add=((x-bx+8)&~7)-x+bx;
	 while(add-->0)
	 {
	    if(x<Upper->w)
	       mvaddch(y+Upper->y,x+Upper->x,' ');
	    x++;
	 }
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
#endif // !USE_MULTIBYTE_CHARS
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
   win->a=a;
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
   win_cell *save;
   int      x,y;

   curs_set(0);
   attrset(0);

   win->prev=Upper;
   Upper=win;

   save = win->buf = (win_cell*)malloc((win->h+1)*(win->w+2)*sizeof(win_cell));

   for(y=0; y<win->h+1; y++)
   {
      for(x=0; x<win->w+2; x++)
      {
         scr_get_cell(y+win->y,x+win->x,save);
         if(!(y<win->h && x<win->w) && !(win->flags&NOSHADOW) && x>1 && y>0)
         {
            if(y+win->y<LINES && x+win->x<COLS)
            {
	       win_cell ch=*save;
	       win_cell_set_attrs(&ch,SHADOW_ATTR->n_attr);
               scr_put_cell(y+win->y,x+win->x,&ch);
            }
         }
         save++;
      }
   }
   SetAttr(win->a);

   Clear();
}
void  CloseWin()
{
   win_cell *save;
   int      x,y;

   save = Upper->buf;

   attrset(0);

   for(y=0; y<Upper->h+1; y++)
      for(x=0; x<Upper->w+2; x++,save++)
      {
	 if(y+Upper->y<LINES && x+Upper->x<COLS
	 && x>=0 && y>=0 && (x<Upper->w || !(Upper->flags&NOSHADOW)))
   	    scr_put_cell(y+Upper->y,x+Upper->x,save);
      }

   free(Upper->buf);
   Upper->buf=0;
   Upper=Upper->prev;
   if(Upper)
      SetAttr(Upper->a);
}
