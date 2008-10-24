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

#ifndef HIGHLI_H
#define HIGHLI_H

#include <stdlib.h>

extern "C" {
#include <regex.h>
}

extern int hl_option,hl_active,hl_lines;

void InitHighlight();

struct syntax_hl
{
   char *rexp;
   int	 mask;
   int	 color;
   re_pattern_buffer rexp_c;
   re_registers regs;

   syntax_hl *next;

   static char *selector;
   static syntax_hl *chain;
   static void free_chain();
   static void attrib_line(const char *buf1,int len1,const char *buf2,int len2,
			   unsigned char *line);

   syntax_hl(int color,int mask);
   ~syntax_hl();
   const char *set_rexp(const char *rexp,bool icase);
};

#endif//HIGHLI_H
