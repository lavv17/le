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
#include "edit.h"
#include "menu1.h"
#include "block.h"
#include "options.h"
#include "clipbrd.h"
#include "format.h"
#include "colormnu.h"
#include "about.h"
#include "search.h"

#define RW  MENU_COND_RW
#define BLK MENU_COND_BLOCK
#define noMM MENU_COND_NO_MM
#define RW_BLK MENU_COND_RW|MENU_COND_BLOCK
#define RW_noMM MENU_COND_RW|MENU_COND_NO_MM
#define RW_BLK_noMM MENU_COND_RW|MENU_COND_BLOCK|MENU_COND_NO_MM
#define RW_CLIP MENU_COND_RW|MENU_COND_CLIPBOARD

typedef void (*f)();

Menu1 MainMenu[]={
{" &File ",SUBM},
   {" &Load         F3 ",FUNC+HIDE,UserLoad		    },
   {" &Save         F2 ",FUNC+HIDE,(f)UserSave,		 RW },
   {" &Quit         ^X ",FUNC+HIDE,Quit			    },
   {NULL},
{" &Help ",SUBM},
   {" &About              ",FUNC+HIDE,UserAbout},
   {NULL},
{NULL}};
