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
#include "edit.h"

WIN   *about_window;

void  ShowAbout()
{
   if(about_window)
      return;

   about_window=CreateWin(MIDDLE,MIDDLE,60,12,DIALOGUE_WIN_ATTR," About ",0);
   DisplayWin(about_window);

   PutStr(MIDDLE,2,"Text editor LE");
   PutStr(MIDDLE,3,"Version  " VERSION);
   PutStr(MIDDLE,5,"Copyright (C) 1993-97 by Alexander V. Lukyanov");
   PutStr(MIDDLE,6,"E-Mail: " EMAIL);
}

void  HideAbout()
{
   if(!about_window)
      return;
   CloseWin();
   DestroyWin(about_window);
   about_window=NULL;
}

void  PrintAbout()
{
   printf("Text editor LE | Vesion " VERSION "\n"
          "Copyright (C) 1993-97 by Alexander V. Lukyanov\n"
          "E-Mail: " EMAIL "\n");
}
