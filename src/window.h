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

#ifndef WINDOW_H
#define WINDOW_H

typedef struct  win
{
    chtype  *buf;
    int     x,y;
    int     w,h;
    struct attr *a;
    const char  *title;
    struct win  *prev;
    int     flags;
} WIN;

#define SIGN    0x1000
#define FRIGHT  0x2000
#define MIDDLE  0x4000
#define FDOWN   FRIGHT

/* window flags */
#define NOSHADOW    1

WIN   *CreateWin(int x,int y,unsigned w,unsigned h,struct attr *a,
                 const char *title,int flags=0);
void  DisplayWin(WIN *);
void  CloseWin();
void  DestroyWin(WIN *);

void  Absolute(int *x,int width,int field);
void  GotoXY(int x,int y);
void  Clear();
void  PutStr(int x,int y,const char *s);
void  PutCh(int x,int y,chtype ch);

extern struct attr *curr_attr;
extern WIN *Upper;

inline void  SetAttr(struct attr *a)
{
   curr_attr=a;
   attrset(a->attr);
}

#endif /* WINDOW_H */
