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
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "edit.h"
#include "keymap.h"

struct   graphchar
{
   byte  ch;
   byte  prol[4];
};

struct graphchar  DumbCoding[]=
{
/*   {'|',{0,1,0,1}},{'+',{1,1,1,1}},{'-',{1,0,1,0}},{'=',{2,0,2,0}},
   {'+',{1,1,0,0}},{'+',{0,1,1,0}},{'+',{0,0,1,1}},{'+',{1,0,0,1}},*/

   {'|',{0,1,0,1}},{'+',{1,1,0,1}},{'+',{2,1,0,1}},{'+',{1,2,0,2}},
   {'+',{1,0,0,2}},{'+',{2,0,0,1}},{'+',{2,2,0,2}},{'|',{0,2,0,2}},
   {'+',{2,0,0,2}},{'+',{2,2,0,0}},{'+',{1,2,0,0}},{'+',{2,1,0,0}},
   {'+',{1,0,0,1}},{'+',{0,1,1,0}},{'+',{1,1,1,0}},{'+',{1,0,1,1}},
   {'+',{0,1,1,1}},{'-',{1,0,1,0}},{'+',{1,1,1,1}},{'+',{0,1,2,1}},
   {'+',{0,2,1,2}},{'+',{0,2,2,0}},{'+',{0,0,2,2}},{'+',{2,2,2,0}},
   {'+',{2,0,2,2}},{'+',{0,2,2,2}},{'=',{2,0,2,0}},{'#',{2,2,2,2}},
   {'+',{2,1,2,0}},{'+',{1,2,1,0}},{'+',{2,0,2,1}},{'+',{1,0,1,2}},
   {'+',{0,2,1,0}},{'+',{0,1,2,0}},{'+',{0,0,2,1}},{'+',{0,0,1,2}},
   {'+',{1,2,1,2}},{'=',{2,1,2,1}},{'+',{1,1,0,0}},{'+',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};

struct graphchar  AlternativeCoding[]=
{
   {'þ',{0,1,0,1}},{'³',{1,1,0,1}},{'´',{2,1,0,1}},{'µ',{1,2,0,2}},
   {'¸',{1,0,0,2}},{'¹',{2,0,0,1}},{' ',{2,2,0,2}},{'¦',{0,2,0,2}},
   {'©',{2,0,0,2}},{'­',{2,2,0,0}},{'»',{1,2,0,0}},{'¾',{2,1,0,0}},
   {'À',{1,0,0,1}},{'Á',{0,1,1,0}},{'Â',{1,1,1,0}},{'Ã',{1,0,1,1}},
   {'È',{0,1,1,1}},{'ý',{1,0,1,0}},{'Ê',{1,1,1,1}},{'Ë',{0,1,2,1}},
   {'Ì',{0,2,1,2}},{'®',{0,2,2,0}},{'Ð',{0,0,2,2}},{'×',{2,2,2,0}},
   {'Ý',{2,0,2,2}},{'Þ',{0,2,2,2}},{'Í',{2,0,2,0}},{'ð',{2,2,2,2}},
   {'Î',{2,1,2,0}},{'Ï',{1,2,1,0}},{'Ò',{2,0,2,1}},{'Ó',{1,0,1,2}},
   {'Ô',{0,2,1,0}},{'Õ',{0,1,2,0}},{'Ø',{0,0,2,1}},{'Ù',{0,0,1,2}},
   {'Ú',{1,2,1,2}},{'Û',{2,1,2,1}},{'ã',{1,1,0,0}},{'õ',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};
struct graphchar  KOI8Coding[]=
{

/* {'â',{0,1,0,1}},{'ä',{1,1,0,1}},{'à',{2,1,0,1}},{'å',{1,2,0,2}},
   {'ç',{1,0,0,2}},{'ê',{2,0,0,1}},{'ë',{2,2,0,2}},{'è',{0,2,0,2}},
   {'ï',{2,0,0,2}},{'î',{2,2,0,0}},{'ì',{1,2,0,0}},{'Ä',{2,1,0,0}},
   {'Å',{1,0,0,1}},{'É',{0,1,1,0}},{'æ',{1,1,1,0}},{'Æ',{1,0,1,1}},
   {'ô',{0,1,1,1}},{'ö',{1,0,1,0}},{'ò',{1,1,1,1}},{'û',{0,1,2,1}},
   {'ù',{0,2,1,2}},{'ÿ',{0,2,2,0}},{'Ö',{0,0,2,2}},{'Ü',{2,2,2,0}},
   {'¢',{2,0,2,2}},{'£',{0,2,2,2}},{'¥',{2,0,2,0}},{'¤',{2,2,2,2}},
   {'–',{2,1,2,0}},{'á',{1,2,1,0}},{'í',{2,0,2,1}},{'ó',{1,0,1,2}},
   {'ú',{0,2,1,0}},{'ñ',{0,1,2,0}},{'Ñ',{0,0,2,1}},{'ª',{0,0,1,2}},
   {'º',{1,2,1,2}},{'¿',{2,1,2,1}},{'¨',{1,1,0,0}},{'¬',{0,0,1,1}},*/

   {'ï',{0,1,0,1}},{'î',{1,1,0,1}},{'ì',{2,1,0,1}},{'Ä',{1,2,0,2}},
   {'Å',{1,0,0,2}},{'É',{2,0,0,1}},{'æ',{2,2,0,2}},{'Æ',{0,2,0,2}},
   {'ô',{2,0,0,2}},{'ö',{2,2,0,0}},{'ò',{1,2,0,0}},{'û',{2,1,0,0}},
   {'ù',{1,0,0,1}},{'ÿ',{0,1,1,0}},{'Ö',{1,1,1,0}},{'Ü',{1,0,1,1}},
   {'¢',{0,1,1,1}},{'£',{1,0,1,0}},{'¥',{1,1,1,1}},{'¤',{0,1,2,1}},
   {'–',{0,2,1,2}},{'á',{0,2,2,0}},{'í',{0,0,2,2}},{'ó',{2,2,2,0}},
   {'ú',{2,0,2,2}},{'ñ',{0,2,2,2}},{'Ñ',{2,0,2,0}},{'ª',{2,2,2,2}},
   {'º',{2,1,2,0}},{'¿',{1,2,1,0}},{'¨',{2,0,2,1}},{'¬',{1,0,1,2}},
   {'½',{0,2,1,0}},{'¼',{0,1,2,0}},{'¡',{0,0,2,1}},{'«',{0,0,1,2}},
   {'¯',{1,2,1,2}},{'—',{2,1,2,1}},{'˜',{1,1,0,0}},{'™',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};
struct graphchar  KOI8_linux_Coding[]=
{
   {'â',{0,1,0,1}},{'ä',{1,1,0,1}},{'à',{2,1,0,1}},{'å',{1,2,0,2}},
   {'ç',{1,0,0,2}},{'ê',{2,0,0,1}},{'ë',{2,2,0,2}},{'¦',{0,2,0,2}},
   {'ï',{2,0,0,2}},{'î',{2,2,0,0}},{'ì',{1,2,0,0}},{'Ä',{2,1,0,0}},
   {'Å',{1,0,0,1}},{'É',{0,1,1,0}},{'æ',{1,1,1,0}},{'Æ',{1,0,1,1}},
   {'ô',{0,1,1,1}},{'ö',{1,0,1,0}},{'ò',{1,1,1,1}},{'û',{0,1,2,1}},
   {'ù',{0,2,1,2}},{'ÿ',{0,2,2,0}},{'Ö',{0,0,2,2}},{'Ü',{2,2,2,0}},
   {'À',{2,0,2,2}},{'£',{0,2,2,2}},{'¥',{2,0,2,0}},{'¤',{2,2,2,2}},
   {'–',{2,1,2,0}},{'á',{1,2,1,0}},{'í',{2,0,2,1}},{'ó',{1,0,1,2}},
   {'—',{0,2,1,0}},{'ñ',{0,1,2,0}},{'Ñ',{0,0,2,1}},{'ª',{0,0,1,2}},
   {'º',{1,2,1,2}},{'¿',{2,1,2,1}},{'¨',{1,1,0,0}},{'¬',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};

struct graphchar  *GraphSets[]=
{
   AlternativeCoding,
   KOI8Coding,
   KOI8_linux_Coding,
   DumbCoding,
   NULL
};
struct graphchar  *CurrGraphSet;
int               grsetno=0;

int   CharIndex(byte ch)
{
   int i;
   for(i=0; CurrGraphSet[i].ch && CurrGraphSet[i].ch!=ch; i++);
   return i;
}
int   DirIndex(int x,int y)
{
   return(x==0?y+2:x+1);
}
void  TurnXY(int *x,int *y)
{
   int x1=*y;
   *y=-*x;
   *x=x1;
}
byte  HowProlonged(byte ch,int x,int y)
{
   return(CurrGraphSet[CharIndex(ch)].prol[DirIndex(x,y)]);
}

int   IsProlonged(byte ch,int x,int y,byte how)
{
   int   dir=DirIndex(x,y);

   for(int i=0; CurrGraphSet[i].ch; i++)
      if(ch==CurrGraphSet[i].ch && how==CurrGraphSet[i].prol[dir])
         return(1);

   return(how==0);
}

void  Prolong(int x,int y,byte how)
{
   struct graphchar  need;
   int               best;
   int               bestdist;
   int               dist;
   int               i;
   int               j;

   need=CurrGraphSet[CharIndex(Char())];
   need.prol[DirIndex(x,y)]=how;

   for(i=1,TurnXY(&x,&y); i<4; i++,TurnXY(&x,&y))
      need.prol[DirIndex(x,y)]=HowProlonged(CharAtLC(GetLine()+y,GetCol()+x),-x,-y);

   best=-1;
   bestdist=30000;
   for(i=0; CurrGraphSet[i].ch; i++)
   {
      j=DirIndex(x,y);
      dist=16*abs(need.prol[j]-CurrGraphSet[i].prol[j]);
      j=(j+1)&3;
      dist+=4*abs(need.prol[j]-CurrGraphSet[i].prol[j]);
      j=(j+1)&3;
      dist+=abs(need.prol[j]-CurrGraphSet[i].prol[j]);
      j=(j+1)&3;
      dist+=4*abs(need.prol[j]-CurrGraphSet[i].prol[j]);

      if(dist<bestdist)
      {
         bestdist=dist;
         best=i;
      }
   }
   ReplaceChar(CurrGraphSet[best].ch);
}

void  Draw(int x,int y,byte how)
{
   if(IsProlonged(Char(),x,y,how))
   {
      if(GetCol()<-x || GetLine()<-y)
         return;
      Prolong(x,y,how);
      if(y)
         RedisplayLine();
      HardMove(GetLine()+y,GetCol()+x);
      Prolong(-x,-y,how);
   }
   else
      Prolong(x,y,how);
   RedisplayLine();
}

void    DrawFrames(void)
{
   extern
   char  **FramesHelp[];
   static
   int   curr_frame=1;
   int   action;
   char  sign[4];

   if(hex || View)
      return;

   CurrGraphSet=GraphSets[grsetno];

   do
   {
      LocateCursor();
      attrset(STATUS_LINE_ATTR->attr);
      sprintf(sign," $%d",curr_frame);
      mvaddstr(StatusLineY,COLS-4,sign);
      SetCursor();
      action=GetNextAction();
      switch(action)
      {
      case(EDITOR_HELP):
         Help(FramesHelp," Frame-Drawing Help ");
         break;
      case(CHAR_LEFT):
         Draw(-1,0,curr_frame);
         break;
      case(CHAR_RIGHT):
         Draw(1,0,curr_frame);
         break;
      case(LINE_UP):
         Draw(0,-1,curr_frame);
         break;
      case(LINE_DOWN):
         Draw(0,1,curr_frame);
         break;
      case(REFRESH_SCREEN):
         UserRefreshScreen();
         break;
      default:
         if(StringTypedLen!=1)
         {
            beep();
            break;
         }
         switch(StringTyped[0])
         {
         case('\t'):
            curr_frame=(curr_frame==2)?0:curr_frame+1;
            break;
         default:
            beep();
            return;
         }
      }
   }
   while(1);
}
