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

#ifndef MENU_H
#define MENU_H

struct  menu
{
   char	 *text;
   int   x,y;
};

char  ItemChar(char *i);
int   ItemLen(char *i);
void  DisplayItem(int x,int y,char *i,attr *a);
int   ReadMenu(struct menu *m,int dir,attr *a,attr *a1);
int   ReadMenuBox(struct menu *m,int dir,const char *msg,const char *title,attr *a,attr *a1);

#define HORIZ   1
#define VERT    2

#endif MENU_H
