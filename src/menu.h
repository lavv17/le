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

#ifndef MENU_H
#define MENU_H

struct  menu
{
   const char *text;
   int   x,y;
};

char  ItemChar(const char *i);
int   ItemLen(const char *i);
void  DisplayItem(int x,int y,const char *i,const attr *a);
int   ReadMenu(const struct menu *m,int dir,const attr *a,const attr *a1,int curr=0);
int   ReadMenuBox(struct menu *m,int dir,const char *msg,const char *title,const attr *a,const attr *a1);

#define HORIZ   1
#define VERT    2

#endif//MENU_H
