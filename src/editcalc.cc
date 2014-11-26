/*
 * Copyright (c) 1993-2014 by Alexander V. Lukyanov (lav@yars.free.net)
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
            PutStr(MIDDLE,y,stack[i].to_string());
            if(i==sp-2)
               PutStr(2,y,"Y:");
            if(i==sp-1)
               PutStr(2,y,"X:");
         }
      }

      if(getstring("Expression: ",expr,sizeof(expr)-1,&CalcHistory,NULL,"CalcHelp"," Calculator Help ")<1)
         break;
      if(!strcmp(expr,"ins"))
      {
	 for(i=sp-1; i>=0; i--)
	 {
            sprintf(str,"%s%s",stack[i].to_string()," "+(i==0));
	    InsertBlock(str,strlen(str));
	    stdcol=GetCol();
	 }
	 CalcHistory-=expr;
	 break;
      }
      calcerrno=0;
      calculator(expr);
   }
   while(1);
   CloseWin();
   DestroyWin(w);
}
