/*
 * Copyright (c) 1993-2000 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <errno.h>
#include "edit.h"
#include "keymap.h"

#include "block.h"
#include "options.h"
#include "keymap.h"
#include "format.h"
#include "search.h"
#include "colormnu.h"

void  EditorReadKeymap()
{
   char  filename[1024];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap-%s",HOME,TERM);
   f=fopen(filename,"r");
   if(f==NULL)
   {
      sprintf(filename,"%s/keymap-%s",PKGDATADIR,TERM);
      f=fopen(filename,"r");
      if(f==NULL)
      {
         sprintf(filename,"%s/.le/keymap",HOME);
         f=fopen(filename,"r");
         if(f==NULL)
         {
            sprintf(filename,"%s/keymap",PKGDATADIR);
            f=fopen(filename,"r");
            if(f==NULL)
               return;
         }
      }
   }

   errno=0;
   ReadActionMap(f);
   if(errno)
   {
      FError(filename);
   }

   fclose(f);
}

void LoadKeymapEmacs()
{
   const char *k=PKGDATADIR"/keymap-emacs";
   FILE *f=fopen(k,"r");
   if(!f)
   {
      FError(k);
      return;
   }
   ReadActionMap(f);
   fclose(f);
   RebuildKeyTree();
   LoadMainMenu();
}
void LoadKeymapDefault()
{
   FreeActionCodeTable();
   ActionCodeTable=DefaultActionCodeTable;
   RebuildKeyTree();
   LoadMainMenu();
}
void SaveKeymap()
{
   char  filename[1024];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap",HOME);
   f=fopen(filename,"w");
   if(!f)
   {
      FError(filename);
      return;
   }
   WriteActionMap(f);
   fclose(f);
}
void SaveKeymapForTerminal()
{
   char  filename[1024];
   FILE  *f;

   sprintf(filename,"%s/.le/keymap-%s",HOME,TERM);
   f=fopen(filename,"w");
   if(!f)
   {
      FError(filename);
      return;
   }
   WriteActionMap(f);
   fclose(f);
}
