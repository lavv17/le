/*
 * Copyright (c) 1993-2004 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include "about.h"

WIN   *about_window;

const char copyright[]="Copyright (C) 1993-2004 by Alexander V. Lukyanov";

void  ShowAbout()
{
   if(about_window)
      return;

   about_window=CreateWin(MIDDLE,MIDDLE,60,14,DIALOGUE_WIN_ATTR," About ",0);
   DisplayWin(about_window);

   PutStr(MIDDLE,2,"Text editor LE");
   PutStr(MIDDLE,3,"Version " VERSION);
   PutStr(MIDDLE,5,copyright);
   PutStr(MIDDLE,6,"E-Mail: " EMAIL);
   PutStr(MIDDLE,8,"This is free software that gives you freedom to use,");
   PutStr(MIDDLE,9,"modify and redistribute it under certain conditions.");
   PutStr(MIDDLE,10,"See the file `COPYING' in the distribution of LE for");
   PutStr(MIDDLE,11,"more information.   There is ABSOLUTELY NO WARRANTY.");
}

void  HideAbout()
{
   if(!about_window)
      return;
   CloseWin();
   DestroyWin(about_window);
   about_window=NULL;
}
