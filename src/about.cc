/*
 * Copyright (c) 1993-2017 by Alexander V. Lukyanov (lav@yars.free.net)
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

const char version_string[]="Text editor LE version " VERSION;
const char copyright[]="Copyright (C) 1993-2017 by Alexander V. Lukyanov";

void  ShowAbout()
{
   if(about_window)
      return;

   about_window=CreateWin(MIDDLE,MIDDLE,69,22,DIALOGUE_WIN_ATTR," About ",0);
   DisplayWin(about_window);

   PutStr(MIDDLE,2,"Text editor LE");
   PutStr(MIDDLE,3,"Version " VERSION);
   PutStr(MIDDLE,5,copyright);
   PutStr(MIDDLE,6,"E-Mail: " EMAIL);

   PutStr(MIDDLE,8,"LE is free software: you can redistribute it and/or modify it");
   PutStr(MIDDLE,9,"under the terms of the GNU General Public License as published by");
   PutStr(MIDDLE,10,"the Free Software Foundation, either version 3 of the License, or");
   PutStr(MIDDLE,11,"(at your option) any later version.");

   PutStr(MIDDLE,13,"This program is distributed in the hope that it will be useful,");
   PutStr(MIDDLE,14,"but WITHOUT ANY WARRANTY; without even the implied warranty of");
   PutStr(MIDDLE,15,"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
   PutStr(MIDDLE,16,"GNU General Public License for more details.");

   PutStr(MIDDLE,18,"You should have received a copy of the GNU General Public License");
   PutStr(MIDDLE,19,"along with LE.  If not, see <http://www.gnu.org/licenses/>.");
}

void  HideAbout()
{
   if(!about_window)
      return;
   CloseWin();
   DestroyWin(about_window);
   about_window=NULL;
}

void PrintVersion()
{
   printf("%s - %s\n%s <%s>\n\n%s",Program,version_string,copyright,EMAIL,
"LE is free software: you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with LE.  If not, see <http://www.gnu.org/licenses/>.\n");
}
