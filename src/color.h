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

enum
{
   STATUS_LINE=0,  
   NORMAL_TEXT,  
   BLOCK_TEXT,   
   ERROR_WIN,    
   VERIFY_WIN,   
   CURR_BUTTON,  
   HELP_WIN,     
   DIALOGUE_WIN, 
   MENU_WIN,         
   DISABLED_ITEM,
   SCROLL_BAR,   
   SHADOWED,
   
   MAX_COLOR_NO,
};

struct attr
{
   short no;
   chtype attr,so_attr;
};

struct color
{
   int	    no;
   chtype   attr;
   int	    fg,bg;
};

void  init_attrs();
struct attr *find_attr(int no);

extern char color_descriptions[MAX_COLOR_NO*2][256];

void  ParseColors();

void  DumpDefaultColors(FILE *);
