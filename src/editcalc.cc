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
#include "calc.h"

extern const char CalcHelp[];

void  editcalc()
{
   WIN   *w;
   static   char  expr[256]="";
   char  str[256];
   int   i,y;
   static   History  CalcHistory;

   w=CreateWin(FRIGHT,FDOWN,40,16,DIALOGUE_WIN_ATTR," Calculator ",0);
   DisplayWin(w);

   calcerrno=0;
   do
   {
      Clear();
      if(calcerrno)
         PutStr(2,1,calcerrmsg());
      if(sp<1)
         PutStr(MIDDLE,2,"Stack empty");
      else
      {
	 i=sp-Upper->h+3;
	 if(i<0)
	    i=0;
         for(y=2; i<sp; i++,y++)
         {
            sprintf(str,"%.*g",15,stack[i]);
            PutStr(MIDDLE,y,str);
            if(i==sp-2)
               PutStr(2,y,"Y");
            if(i==sp-1)
               PutStr(2,y,"X");
         }
      }

      if(getstring("Expression: ",expr,sizeof(expr)-1,&CalcHistory,NULL,CalcHelp," Calculator Help ")<1)
         break;
      calcerrno=0;
      calculator(expr);
   }
   while(1);
   CloseWin();
   DestroyWin(w);
}
