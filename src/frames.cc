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

struct   GraphChar
{
   int  ch;
   enum { L=0, T, R, B };
   char prol[4];
};

struct GraphChar  DumbCoding[]=
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

struct GraphChar  AlternativeCoding[]=
{
   {'≥',{0,1,0,1}},{'¥',{1,1,0,1}},{'µ',{2,1,0,1}},{'∂',{1,2,0,2}},
   {'∑',{1,0,0,2}},{'∏',{2,0,0,1}},{'π',{2,2,0,2}},{'∫',{0,2,0,2}},
   {'ª',{2,0,0,2}},{'º',{2,2,0,0}},{'Ω',{1,2,0,0}},{'æ',{2,1,0,0}},
   {'ø',{1,0,0,1}},{'¿',{0,1,1,0}},{'¡',{1,1,1,0}},{'¬',{1,0,1,1}},
   {'√',{0,1,1,1}},{'ƒ',{1,0,1,0}},{'≈',{1,1,1,1}},{'∆',{0,1,2,1}},
   {'«',{0,2,1,2}},{'»',{0,2,2,0}},{'…',{0,0,2,2}},{' ',{2,2,2,0}},
   {'À',{2,0,2,2}},{'Ã',{0,2,2,2}},{'Õ',{2,0,2,0}},{'Œ',{2,2,2,2}},
   {'œ',{2,1,2,0}},{'–',{1,2,1,0}},{'—',{2,0,2,1}},{'“',{1,0,1,2}},
   {'”',{0,2,1,0}},{'‘',{0,1,2,0}},{'’',{0,0,2,1}},{'÷',{0,0,1,2}},
   {'◊',{1,2,1,2}},{'ÿ',{2,1,2,1}},{'Ÿ',{1,1,0,0}},{'⁄',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};
struct GraphChar  KOI8Coding[]=
{

/* {'É',{0,1,0,1}},{'Ñ',{1,1,0,1}},{'Ö',{2,1,0,1}},{'Ü',{1,2,0,2}},
   {'á',{1,0,0,2}},{'à',{2,0,0,1}},{'â',{2,2,0,2}},{'ä',{0,2,0,2}},
   {'ã',{2,0,0,2}},{'å',{2,2,0,0}},{'ç',{1,2,0,0}},{'é',{2,1,0,0}},
   {'è',{1,0,0,1}},{'ê',{0,1,1,0}},{'ë',{1,1,1,0}},{'í',{1,0,1,1}},
   {'ì',{0,1,1,1}},{'î',{1,0,1,0}},{'ï',{1,1,1,1}},{'ñ',{0,1,2,1}},
   {'ó',{0,2,1,2}},{'ò',{0,2,2,0}},{'ô',{0,0,2,2}},{'ö',{2,2,2,0}},
   {'õ',{2,0,2,2}},{'ú',{0,2,2,2}},{'ù',{2,0,2,0}},{'û',{2,2,2,2}},
   {'ü',{2,1,2,0}},{'†',{1,2,1,0}},{'°',{2,0,2,1}},{'¢',{1,0,1,2}},
   {'£',{0,2,1,0}},{'§',{0,1,2,0}},{'•',{0,0,2,1}},{'¶',{0,0,1,2}},
   {'ß',{1,2,1,2}},{'®',{2,1,2,1}},{'©',{1,1,0,0}},{'™',{0,0,1,1}},*/

   {'ã',{0,1,0,1}},{'å',{1,1,0,1}},{'ç',{2,1,0,1}},{'é',{1,2,0,2}},
   {'è',{1,0,0,2}},{'ê',{2,0,0,1}},{'ë',{2,2,0,2}},{'í',{0,2,0,2}},
   {'ì',{2,0,0,2}},{'î',{2,2,0,0}},{'ï',{1,2,0,0}},{'ñ',{2,1,0,0}},
   {'ó',{1,0,0,1}},{'ò',{0,1,1,0}},{'ô',{1,1,1,0}},{'ö',{1,0,1,1}},
   {'õ',{0,1,1,1}},{'ú',{1,0,1,0}},{'ù',{1,1,1,1}},{'û',{0,1,2,1}},
   {'ü',{0,2,1,2}},{'†',{0,2,2,0}},{'°',{0,0,2,2}},{'¢',{2,2,2,0}},
   {'£',{2,0,2,2}},{'§',{0,2,2,2}},{'•',{2,0,2,0}},{'¶',{2,2,2,2}},
   {'ß',{2,1,2,0}},{'®',{1,2,1,0}},{'©',{2,0,2,1}},{'™',{1,0,1,2}},
   {'´',{0,2,1,0}},{'¨',{0,1,2,0}},{'≠',{0,0,2,1}},{'Æ',{0,0,1,2}},
   {'Ø',{1,2,1,2}},{'∞',{2,1,2,1}},{'±',{1,1,0,0}},{'≤',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};
struct GraphChar  KOI8_linux_Coding[]=
{
   {'É',{0,1,0,1}},{'Ñ',{1,1,0,1}},{'Ö',{2,1,0,1}},{'Ü',{1,2,0,2}},
   {'á',{1,0,0,2}},{'à',{2,0,0,1}},{'â',{2,2,0,2}},{'∫',{0,2,0,2}},
   {'ã',{2,0,0,2}},{'å',{2,2,0,0}},{'ç',{1,2,0,0}},{'é',{2,1,0,0}},
   {'è',{1,0,0,1}},{'ê',{0,1,1,0}},{'ë',{1,1,1,0}},{'í',{1,0,1,1}},
   {'ì',{0,1,1,1}},{'î',{1,0,1,0}},{'ï',{1,1,1,1}},{'ñ',{0,1,2,1}},
   {'ó',{0,2,1,2}},{'ò',{0,2,2,0}},{'ô',{0,0,2,2}},{'ö',{2,2,2,0}},
   {'ø',{2,0,2,2}},{'ú',{0,2,2,2}},{'ù',{2,0,2,0}},{'û',{2,2,2,2}},
   {'ü',{2,1,2,0}},{'†',{1,2,1,0}},{'°',{2,0,2,1}},{'¢',{1,0,1,2}},
   {'∞',{0,2,1,0}},{'§',{0,1,2,0}},{'•',{0,0,2,1}},{'¶',{0,0,1,2}},
   {'ß',{1,2,1,2}},{'®',{2,1,2,1}},{'©',{1,1,0,0}},{'™',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};

struct GraphChar UnicodeCoding[]=
{
/* {'‚îÇ',{0,1,0,1}},{'‚î§',{1,1,0,1}},{'‚ï°',{2,1,0,1}},{'‚ï¢',{1,2,0,2}},
   {'‚ïñ',{1,0,0,2}},{'‚ïï',{2,0,0,1}},{'‚ï£',{2,2,0,2}},{'‚ïë',{0,2,0,2}},
   {'‚ïó',{2,0,0,2}},{'‚ïù',{2,2,0,0}},{'‚ïú',{1,2,0,0}},{'‚ïõ',{2,1,0,0}},
   {'‚îê',{1,0,0,1}},{'‚îî',{0,1,1,0}},{'‚î¥',{1,1,1,0}},{'‚î¨',{1,0,1,1}},
   {'‚îú',{0,1,1,1}},{'‚îÄ',{1,0,1,0}},{'‚îº',{1,1,1,1}},{'‚ïû',{0,1,2,1}},
   {'‚ïü',{0,2,1,2}},{'‚ïö',{0,2,2,0}},{'‚ïî',{0,0,2,2}},{'‚ï©',{2,2,2,0}},
   {'‚ï¶',{2,0,2,2}},{'‚ï†',{0,2,2,2}},{'‚ïê',{2,0,2,0}},{'‚ï¨',{2,2,2,2}},
   {'‚ïß',{2,1,2,0}},{'‚ï®',{1,2,1,0}},{'‚ï§',{2,0,2,1}},{'‚ï•',{1,0,1,2}},
   {'‚ïô',{0,2,1,0}},{'‚ïò',{0,1,2,0}},{'‚ïí',{0,0,2,1}},{'‚ïì',{0,0,1,2}},
   {'‚ï´',{1,2,1,2}},{'‚ï™',{2,1,2,1}},{'‚îò',{1,1,0,0}},{'‚îå',{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}} */
   {0x2502,{0,1,0,1}},{0x2524,{1,1,0,1}},{0x2561,{2,1,0,1}},{0x2562,{1,2,0,2}},
   {0x2556,{1,0,0,2}},{0x2555,{2,0,0,1}},{0x2563,{2,2,0,2}},{0x2551,{0,2,0,2}},
   {0x2557,{2,0,0,2}},{0x255D,{2,2,0,0}},{0x255C,{1,2,0,0}},{0x255B,{2,1,0,0}},
   {0x2510,{1,0,0,1}},{0x2514,{0,1,1,0}},{0x2534,{1,1,1,0}},{0x252C,{1,0,1,1}},
   {0x251C,{0,1,1,1}},{0x2500,{1,0,1,0}},{0x253C,{1,1,1,1}},{0x255E,{0,1,2,1}},
   {0x255F,{0,2,1,2}},{0x255A,{0,2,2,0}},{0x2554,{0,0,2,2}},{0x2569,{2,2,2,0}},
   {0x2566,{2,0,2,2}},{0x2560,{0,2,2,2}},{0x2550,{2,0,2,0}},{0x256C,{2,2,2,2}},
   {0x2567,{2,1,2,0}},{0x2568,{1,2,1,0}},{0x2564,{2,0,2,1}},{0x2565,{1,0,1,2}},
   {0x2559,{0,2,1,0}},{0x2558,{0,1,2,0}},{0x2552,{0,0,2,1}},{0x2553,{0,0,1,2}},
   {0x256B,{1,2,1,2}},{0x256A,{2,1,2,1}},{0x2518,{1,1,0,0}},{0x250C,{0,0,1,1}},
   {' ',{0,0,0,0}},{0,{0,0,0,0}}
};

struct GraphChar  *GraphSets[]=
{
   AlternativeCoding,
   KOI8Coding,
   KOI8_linux_Coding,
   DumbCoding,
   NULL
};
struct GraphChar  *CurrGraphSet;
int               grsetno=0;

int   CharIndex(int ch)
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
int  HowProlonged(int ch,int x,int y)
{
   return(CurrGraphSet[CharIndex(ch)].prol[DirIndex(x,y)]);
}

int   IsProlonged(int ch,int x,int y,byte how)
{
   int   dir=DirIndex(x,y);

   for(int i=0; CurrGraphSet[i].ch; i++)
      if(ch==CurrGraphSet[i].ch && how==CurrGraphSet[i].prol[dir])
         return(1);

   return(how==0);
}

void  Prolong(int x,int y,byte how)
{
   struct GraphChar  need;
   int               best;
   int               bestdist;
   int               dist;
   int               i;
   int               j;

   need=CurrGraphSet[CharIndex(mb_mode?WChar():Char())];
   need.prol[DirIndex(x,y)]=how;

   for(i=1,TurnXY(&x,&y); i<4; i++,TurnXY(&x,&y))
   {
      int ch=mb_mode?WCharAtLC(GetLine()+y,GetCol()+x)
		    :CharAtLC(GetLine()+y,GetCol()+x);
      need.prol[DirIndex(x,y)]=HowProlonged(ch,-x,-y);
   }

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
   if(mb_mode)
      ReplaceWCharExt(CurrGraphSet[best].ch);
   else
      ReplaceCharExt(CurrGraphSet[best].ch);
}

void  Draw(int x,int y,byte how)
{
   if(IsProlonged(mb_mode?WChar():Char(),x,y,how))
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
   static
   int   curr_frame=1;
   int   action;
   char  sign[4];

   if(hex || View)
      return;

   CurrGraphSet=GraphSets[grsetno];
   if(mb_mode)
      CurrGraphSet=UnicodeCoding;

   do
   {
      ClearMessage();
      SyncTextWin();
      StatusLine();
      attrset(STATUS_LINE_ATTR->n_attr);
      sprintf(sign," $%d",curr_frame);
      mvaddstr(StatusLineY,COLS-4,sign);
      SetCursor();

      action=GetNextAction();
      switch(action)
      {
      case(EDITOR_HELP):
         Help("FramesHelp"," Frame-Drawing Help ");
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
