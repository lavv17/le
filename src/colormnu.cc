/*
 * Copyright (c) 1998 by Alexander V. Lukyanov (lav@yars.free.net)
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

#include <stdio.h>
#include <unistd.h>
#include "edit.h"
#include "colormnu.h"
#include "options.h"
#include "efopen.h"

void ColorsSaveToFile(const char *f)
{
   DescribeColors(bw_pal,color_pal);
   SaveConfToFile(f,colors);
}

static const char *const colors_file="/.le/colors";
void ColorsSave()
{
   char f[strlen(HOME)+strlen(colors_file)+1];
   sprintf(f,"%s%s",HOME,colors_file);
   ColorsSaveToFile(f);
}

void ColorsSaveForTerminal()
{
   char f[strlen(HOME)+strlen(colors_file)+1+strlen(TERM)+1];
   sprintf(f,"%s%s-%s",HOME,colors_file,TERM);
   ColorsSaveToFile(f);
}

void LoadColor(const char *name)
{
#ifdef EMBED_DATADIR
   char fn[strlen(name)+1];
   sprintf(fn,"%s",name);
   FILE *f=efopen(fn,"r");
#else
   char fn[strlen(PKGDATADIR)+1+strlen(name)+1];
   sprintf(fn,"%s/%s",PKGDATADIR,name);
   FILE *f=fopen(fn,"r");
#endif
   if(!f)
   {
      FError(fn);
      return;
   }
   ReadConfFromOpenFile(f,colors);
   fclose(f);
   ParseColors();
   init_attrs();
   clearok(stdscr,1);
   flag=REDISPLAY_ALL;
}

void LoadColorDefault()
{
   memcpy(color_pal,default_color_pal,sizeof(default_color_pal));
   memcpy(bw_pal,default_bw_pal,sizeof(default_bw_pal));
   init_attrs();
#if !defined(NCURSES_VERSION_PATCH) || NCURSES_VERSION_PATCH<980627
   clearok(stdscr,1);
#endif
   flag=REDISPLAY_ALL;
}

void LoadColorDefaultBG()
{
   LoadColor("colors-defbg");
}
void LoadColorBlue()
{
   LoadColor("colors-blue");
}
void LoadColorBlack()
{
   LoadColor("colors-black");
}
void LoadColorWhite()
{
   LoadColor("colors-white");
}
void LoadColorGreen()
{
   LoadColor("colors-green");
}
