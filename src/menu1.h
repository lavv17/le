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

/* Definitions for pull-down menu */

#ifndef MENU1_H
#define MENU1_H

#include <stdlib.h>

enum menu_conds
{
   SUBM=1,
   FUNC=2,
   HIDE=4,
   FREE_TEXT=8,
   MENU_COND_RW=16,
   MENU_COND_RO=32,
   MENU_COND_BLOCK=64,
   MENU_COND_NO_MM=128,
   MENU_COND_CLIPBOARD=256
};

typedef struct sr_menu
{
   char	*text;	/* the text of the item, NULL means end of (sub)menu */
   unsigned fl;
   union {
      int action;    // for FUNC
      int curritem;  // for END (text==NULL)
      WIN *win;	     // for SUBM
   };

   void FreeText() {
      if(fl&FREE_TEXT) {
	 free(text);
	 text=0;
      }
   }
   void SetText(char *new_text) {
      FreeText();
      text=new_text;
      fl|=FREE_TEXT;
   }
} Menu1;

void LoadMainMenu();

#endif /* MENU1_H */
