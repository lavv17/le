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

#include <config.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

#include <xalloca.h>
#include "edit.h"
#include "options.h"

struct attr attr_table[MAX_COLOR_NO];
int   attr_num;

int   next_pair;
static int find_pair(int fg,int bg)
{
   for(int i=1; i<next_pair; i++)
   {
      short fg1,bg1;
      pair_content(i,&fg1,&bg1);
      if(fg1==fg && bg1==bg)
	 return i;
   }
   init_pair(next_pair,fg,bg);
   return next_pair++;
}

void  init_attr_table(struct color *pal)
{
   int i;
   int pair;

   next_pair=1;
   memset(attr_table,0,sizeof(attr_table));
   for(i=0; pal[i].no!=-1; i++)
   {
      int an=pal[i].no;
      assert(an>=0 && an<MAX_COLOR_NO);
      attr_table[an].attr=0;
      if(pal[i].fg || pal[i].bg)
      {
	 pair=find_pair(pal[i].fg,pal[i].bg);
	 attr_table[an].attr|=COLOR_PAIR(pair);
      }
      attr_table[an].attr|=pal[i].attr;
   }

   /* make standout attributes */
   for(i=0; i<MAX_COLOR_NO; i++)
   {
      if(i==DISABLED_ITEM || i==SHADOWED)
      {
	 attr_table[i].so_attr=attr_table[i].attr;
	 continue;
      }
      int p=0;
      while(pal[p].no!=i && pal[p].no!=-1)
	 p++;
      if(pal[p].fg || pal[p].bg)
      {
	 if(pal[p].fg==COLOR_YELLOW)
	    attr_table[i].so_attr=attr_table[i].attr^A_BOLD;
	 else
	 {
	    attr_table[i].so_attr=(attr_table[i].attr|A_BOLD)&~A_COLOR;
	    pair=find_pair(COLOR_YELLOW,pal[p].bg);
	    attr_table[i].so_attr|=COLOR_PAIR(pair);
	 }
      }
      else
	 attr_table[i].so_attr=attr_table[i].attr^A_BOLD;
   }
}

struct color default_color_pal[]=
{
   {STATUS_LINE,  A_NORMAL,   COLOR_BLACK,   COLOR_CYAN},
   {NORMAL_TEXT,  A_NORMAL,   COLOR_WHITE,   COLOR_BLUE},
   {BLOCK_TEXT,	  A_NORMAL,   COLOR_BLACK,   COLOR_WHITE},
   {ERROR_WIN,	  A_BOLD,     COLOR_WHITE,   COLOR_RED},
   {VERIFY_WIN,	  A_NORMAL,   COLOR_BLACK,   COLOR_CYAN},
   {CURR_BUTTON,  A_BOLD,     COLOR_WHITE,   COLOR_BLACK},
   {HELP_WIN,	  A_NORMAL,   COLOR_BLACK,   COLOR_CYAN},
   {DIALOGUE_WIN, A_NORMAL,   COLOR_BLACK,   COLOR_WHITE},
   {MENU_WIN,	  A_NORMAL,   COLOR_BLACK,   COLOR_CYAN},
   {DISABLED_ITEM,A_NORMAL,   COLOR_BLACK,   COLOR_CYAN},
   {SCROLL_BAR,	  A_NORMAL,   COLOR_CYAN,    COLOR_BLACK},
   {SHADOWED,	  A_NORMAL,   COLOR_WHITE,   COLOR_BLACK},
   {SYNTAX1,	  A_BOLD,     COLOR_YELLOW,  COLOR_BLUE},
   {SYNTAX2,	  A_BOLD,     COLOR_CYAN,    COLOR_BLUE},
   {SYNTAX3,	  A_BOLD,     COLOR_GREEN,   COLOR_BLUE},
   {-1}
};
struct color default_bw_pal[]=
{
   {STATUS_LINE,  A_REVERSE|A_DIM   },
   {NORMAL_TEXT,  A_NORMAL	    },
   {BLOCK_TEXT,	  A_REVERSE	    },
   {ERROR_WIN,	  A_BOLD|A_REVERSE  },
   {VERIFY_WIN,	  A_REVERSE	    },
   {CURR_BUTTON,  A_NORMAL	    },
   {HELP_WIN,	  A_REVERSE	    },
   {DIALOGUE_WIN, A_REVERSE	    },
   {MENU_WIN,	  A_REVERSE	    },
   {DISABLED_ITEM,A_REVERSE	    },
   {SCROLL_BAR,	  A_DIM		    },
   {SHADOWED,	  A_DIM		    },
   {SYNTAX1,	  A_BOLD	    },
   {SYNTAX2,	  A_BOLD	    },
   {SYNTAX3,	  A_DIM		    },
   {-1}
};

struct color color_pal[MAX_COLOR_NO+1];
struct color bw_pal[MAX_COLOR_NO+1];

void  init_attrs()
{
   if(has_colors() && UseColor)
      init_attr_table(color_pal);
   else
      init_attr_table(bw_pal);
}

char color_descriptions[MAX_COLOR_NO*2][256];

struct attr_name
{
   const char *name;
   color value;
};

const attr_name attr_names_table[]=
{
   {"rev",	  {0,A_REVERSE}},
   {"bold",	  {0,A_BOLD}},
   {"dim",	  {0,A_DIM}},
   {"ul",	  {0,A_UNDERLINE}},

   {"fg:black",	  {0,0,COLOR_BLACK,0}},
   {"fg:green",	  {0,0,COLOR_GREEN,0}},
   {"fg:red",	  {0,0,COLOR_RED,0}},
   {"fg:yellow",  {0,0,COLOR_YELLOW,0}},
   {"fg:blue",	  {0,0,COLOR_BLUE,0}},
   {"fg:cyan",	  {0,0,COLOR_CYAN,0}},
   {"fg:magenta", {0,0,COLOR_MAGENTA,0}},
   {"fg:white",	  {0,0,COLOR_WHITE,0}},

   {"bg:black",	  {0,0,0,COLOR_BLACK}},
   {"bg:green",	  {0,0,0,COLOR_GREEN}},
   {"bg:red",	  {0,0,0,COLOR_RED}},
   {"bg:yellow",  {0,0,0,COLOR_YELLOW}},
   {"bg:blue",	  {0,0,0,COLOR_BLUE}},
   {"bg:cyan",	  {0,0,0,COLOR_CYAN}},
   {"bg:magenta", {0,0,0,COLOR_MAGENTA}},
   {"bg:white",	  {0,0,0,COLOR_WHITE}},

   {NULL},
};

color *FindColor(color *pal,int no)
{
   for(color *scan=pal; scan->no!=-1; scan++)
   {
      if(scan->no==no)
      {
	 return scan;
      }
   }
   return 0;
}

void  ParseOneColor(color *pal,const char *desc,int no)
{
   color c;
   char	 *d=(char*)alloca(strlen(desc)+1);
   int	 good=0;

   strcpy(d,desc);

   c.attr=c.fg=c.bg=0;
   c.no=no;

   while(*d && isspace(*d))
      d++;

   char *tok=strtok(d,",");
   while(tok)
   {
      char *eq=strchr(tok,'=');
      if(eq)
	 *eq=':';
      for(const attr_name *an=attr_names_table; an->name; an++)
      {
	 if(!strcmp(an->name,tok))
	 {
	    good=1;
	    c.attr|=an->value.attr;
	    if(c.fg==0)
	       c.fg=an->value.fg;
	    if(c.bg==0)
	       c.bg=an->value.bg;
	    break;
	 }
      }
      tok=strtok(NULL,",");
   }
   if(good)
   {
      *FindColor(pal,no)=c;
   }
}

void  ParseColors()
{
   int n;

   memcpy(color_pal,default_color_pal,sizeof(default_color_pal));

   for(n=0; n<MAX_COLOR_NO; n++)
      ParseOneColor(color_pal,color_descriptions[n],n);

   memcpy(bw_pal,default_bw_pal,sizeof(default_bw_pal));

   for( ; n<MAX_COLOR_NO*2; n++)
      ParseOneColor(bw_pal,color_descriptions[n],n-MAX_COLOR_NO);

}

void  DescribeOneColor(char *const desc,color *cp)
{
   color c=*cp;
   const attr_name *a;
   char *d=desc;

   a=attr_names_table;
   while(a->name)
   {
      if(c.fg && a->value.fg==c.fg)
      {
	 sprintf(d,",%s",a->name);
	 d+=strlen(d);
	 c.fg=0;
      }
      if(c.bg && a->value.bg==c.bg)
      {
	 sprintf(d,",%s",a->name);
	 d+=strlen(d);
	 c.bg=0;
      }
      if(c.attr&a->value.attr)
      {
	 sprintf(d,",%s",a->name);
	 d+=strlen(d);
	 c.attr&=~a->value.attr;
      }
      a++;
   }
   if(desc[0]==',')
   {
      memmove(desc,desc+1,strlen(desc));
   }
}

void  DescribeColors(color *bw,color *co)
{
   int i=0;
   for( ; i<MAX_COLOR_NO; i++)
      DescribeOneColor(color_descriptions[i],FindColor(co,i));
   for( ; i<2*MAX_COLOR_NO; i++)
      DescribeOneColor(color_descriptions[i],FindColor(bw,i-MAX_COLOR_NO));
}

void  DumpDefaultColors(FILE *f)
{
   DescribeColors(default_bw_pal,default_color_pal);

   extern struct init colors[];
   SaveConfToOpenFile(f,colors);
}
