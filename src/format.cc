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

/*_____________________________________________________________________________
**
**  File:           format.cc
**  Description:    Format functions for text editor
**_____________________________________________________________________________
*/
#include <config.h>
#include <string.h>
#include <assert.h>
#include "edit.h"
#include <ctype.h>
#include "keymap.h"

int    LineLen=63;
int    LeftMargin=0;
int    FirstLineMargin=3;
int    LeftAdj=1;
int    RightAdj=0;

void  FormatPara()
{
   num   bcol,bcol1,ncol;
   int   i;

   if(hex || View) /* formatting is not allowed in those modes */
      return;

   flag=1;
   ToLineBegin();
   while(isspace(Char()) && !Eof())
   {
      ExpandTab();
      MoveRight();
   }
   if(Eof())
      return;

   /* ÔÚÊÓğÊÍ ÏÂÓÂÌÓÂË, Õ.Ê. ØıÂŞĞÍ ÚÔÊ ÏÓÎÃÊŞã İÓÎÍÊ ÎığÎÌÎ Ğ ÏÊÓÊÚÎıã ÔÕÓÎİ */
   for(;;)
   {
      while(Char()!=' ' && Char()!='\t' && !Eol())
         MoveRight();
      while((Char()==' ' || Char()=='\t') && !Eol())
         DeleteChar();
      if(Eol())
      {
         DeleteEOL();
         for(i=ncol=0; ncol<LeftMargin && isspace(Char()) && !Eol(); i++)
         {
            if(Char()=='\t')
               ncol=Tabulate(ncol);
            else
               ncol++;
            MoveRight();
         }
         if(Eol() || ((isspace(Char()) || ncol>LeftMargin) && LeftAdj))
         {
            while(i-->0)
               MoveLeft();
            break;
         }
         while(i-->0)
            BackSpace();   /* ØıÂŞĞÕÛ ÔÕÂÓã× ÎÕÔÕØÏ */
      }
      InsertChar(' ');
   }

   NewLine();
   stdcol=GetCol();
   MoveUp();

   if(LeftAdj)
   {
      /* ÔÎõıÂıĞÍ ÎÕÔÕØÏ ÏÊÓÚÎ× ÔÕÓÎİĞ */
      for(i=ncol=0; ncol<LeftMargin+FirstLineMargin && isspace(Char()) && !Eol(); i++)
      {
         if(Char()=='\t')
            ncol=Tabulate(ncol);
         else
            ncol++;
         MoveRight();
      }
      if(ncol<LeftMargin+FirstLineMargin)
      {
         while(!Bol())
            BackSpace();
         for(i=FirstLineMargin+LeftMargin; i>0; i--)
            InsertChar(' ');
      }
      else
      {
         while(ncol<LeftMargin+LineLen/2 && isspace(Char()) && !Eol())
         {
            if(Char()=='\t')
               ncol=Tabulate(ncol);
            else
               ncol++;
            MoveRight();
         }
         while(isspace(Char()) && !Eol())
            DeleteChar();
      }
   }
   else
   {
      while(isspace(Char()) && !Eol())
         DeleteChar();
   }

   for(;;)
   {
      if(GetCol()>LineLen+LeftMargin)
      {
         /* ÊÔŞĞ ÎÊÓÊığÎÊ ÔŞÎÚÎ ÚãŞÊõŞÎ õÂ ÏÓÂÚã× İÓÂ×, ÕÎ ... */
         while(!Bol() && !isspace(CharRel(-1)))
            MoveLeft();
         while(!Bol() && isspace(CharRel(-1)))   /* ğÂ ÎığÎ ÔŞÎÚÎ ÚŞÊÚÎ */
            MoveLeft();
         if(Bol())
         {
            stdcol=0;   /* ÔŞÎÚÎ õÂğĞÍÂÊÕ ÚÔÁ ÔÕÓÎİØ */
            while(!Eol() && isspace(Char()))
               MoveRight();
            while(!Eol() && !isspace(Char()))
               MoveRight();
            if(!Eol())
	    {
               DeleteChar();   /* delete space after the word */
	       NewLine();
	       continue;
	    }
            break;
         }
         else
            DeleteChar();
         bcol1=bcol=GetCol();

	 assert(GetCol()<=LeftMargin+LineLen);

	 if(RightAdj && LeftAdj)
         {
            /* ÓÂÔÕÒğÊÍ ÔÕÓÎİØ ıÎ ğØÙğÎÌÎ ÓÂõÍÊÓÂ */

            int gap_num=0;
            int spaces_to_insert=LineLen+LeftMargin-bcol;
            int i;

            if(spaces_to_insert>0)
            {
               while(!Bol())
               {
                  MoveLeft();
                  if(isspace(Char()))
                     gap_num++;
               }
               while(isspace(Char()) && GetCol()<bcol)
               {
                  MoveRight();
                  gap_num--;
               }
               i=-gap_num/2;
               while(GetCol()<bcol)
               {
                  if(isspace(Char()))
                  {
                     MoveRight();
                     i+=spaces_to_insert;
                     while(i>0)
                     {
                        InsertChar(' ');
                        bcol++;
                        i-=gap_num;
                     }
                  }
                  else
                  {
                     MoveRight();
                  }
               }
            }
            NewLine();
         }
         else if(!LeftAdj && RightAdj)
         {
            ToLineBegin();
            while(bcol<LineLen+LeftMargin)
            {
               InsertChar(' ');
               bcol++;
            }
            GetCol();
            while(GetCol()<bcol)
               MoveRight();
            NewLine();
         }
         else if(!LeftAdj && !RightAdj)
         {
            GetCol();
            while(GetCol()<bcol)
               MoveRight();
            NewLine();
            MoveUp();
            CenterLine();
            MoveDown();
         }
         else
         {
            NewLine();
         }
         for(i=LeftMargin; i>0; i--)
            InsertChar(' ');
      }
      else
      {
	 if(Eol())
	    break;   /* İÎğÊÈ ÔÕÓÎİĞ Ğ ÏÂÓÂÌÓÂËÂ */
         MoveRight();
      }
   }
   if(!LeftAdj && RightAdj)
   {
      bcol=GetCol();
      ToLineBegin();
      while(bcol<LineLen+LeftMargin)
      {
         InsertChar(' ');
         bcol++;
      }
      GetCol();
      while(GetCol()<bcol)
         MoveRight();
   }

   ToLineBegin();
   stdcol=GetCol();
   MoveDown();
}

void  FormatAll()
{
   static  struct  menu FAmenu[]={
   {"   &Ok   ",MIDDLE-6,FDOWN-2},
   {" &Cancel ",MIDDLE+6,FDOWN-2},
   {NULL}};

   if(hex || View)
      return;

   switch(ReadMenuBox(FAmenu,HORIZ,"ALL text will be formatted (no undo)",
      " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
   {
   case(0):
   case('C'):
      return;
   }
   Message("Formatting all document...");
   TextPoint oldpos=CurrentPos;
   CurrentPos=TextBegin;
   while(!Eof())
      FormatPara();
   CurrentPos=oldpos;
}

void  CenterLine()
{
   num shift;
   if(hex || View)
      return;
   flag=1;
   ToLineBegin();
   while(Char()==' ' || Char()=='\t' && !Eol())
      DeleteChar();
   if(Eol())
      return; /* nothing to center */
   ToLineEnd();
   while(isspace(CharRel(-1)))
      BackSpace();
   shift=(LineLen-GetCol())/2+LeftMargin;
   if(shift<=0)
      return; /* too long line */
   ToLineBegin();
   while(shift--)
      InsertChar(' ');
   ToLineBegin();
   stdcol=GetCol();
}

void  FormatFunc()
{
   int   action;

   if(hex || View)
      return;
   ToLineBegin();
   stdcol=GetCol();
again:
   CenterView();
   SetCursor();
   Message("Format: F-Format all P-format Paragraph C-Center line R-align Right");
   SetCursor();
   action=GetNextAction();
   switch(action)
   {
   case(LINE_UP):
       UserLineUp();
       goto again;
   case(LINE_DOWN):
       UserLineDown();
       goto again;
   case(REFRESH_SCREEN):
       UserRefreshScreen();
       break;
   default:
      if(StringTypedLen!=1)
         break;
      switch(StringTyped[0])
      {
      case('P'):
      case('p'):
         Message("Formatting one paragraph...");
         FormatPara();
         RedisplayAll();
         goto again;
      case('R'):
      case('r'):
      {
	 int oldL=LeftAdj,oldR=RightAdj;
	 LeftAdj=0;
	 RightAdj=1;
         Message("Formatting one paragraph (aligned to right)...");
         FormatPara();
         RedisplayAll();
	 LeftAdj=oldL;
	 RightAdj=oldR;
         goto again;
      }
      case('C'):
      case('c'):
         Message("Centering...");
         CenterLine();
         RedisplayLine();
         UserLineDown();
         goto again;
      case('F'):
      case('f'):
         FormatAll();
         flag=REDISPLAY_ALL;
         break;
      }
   }
}
