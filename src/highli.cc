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

/* $Id$ */

#include "highli.h"
#include "edit.h"
#include "screen.h"

char *curr_highlight=0;
re_pattern_buffer hl_compiled;
re_registers	  hl_regs;
char  hl_fastmap[256];

void InitHighlight(const char *filename)
{
   if(curr_highlight)
      regfree(&hl_compiled);

   curr_highlight="[^a-zA-Z_](else|if|switch|case)[^a-zA-Z_]";

   re_syntax_options=RE_SYNTAX_POSIX_EGREP;
   const char *err=re_compile_pattern(curr_highlight,strlen(curr_highlight),
				       &hl_compiled);
   if(err)
   {
      ErrMsg(err);
      return;
   }
   hl_compiled.newline_anchor=1;
   hl_compiled.fastmap=hl_fastmap;
}
