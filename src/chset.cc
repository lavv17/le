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
#include <ctype.h>
#include <wctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"

byte  chset[CHSET_SIZE+1];

void  clear_chset()
{
   int i;
   for(i=0; i<CHSET_SIZE; i++)
      chset[i]='@';
   chset[i]=0;
}

void  set_chset_bit(int i)
{
   chset[i/CHSET_BITS_PER_BYTE]|=(1<<i%CHSET_BITS_PER_BYTE);
}

void  init_chset()
{
/*   int i;
   clear_chset();
   for(i=0; i<256; i++)
      if(iscntrl(i))
         chset[i/CHSET_BITS_PER_BYTE]|=(1<<i%CHSET_BITS_PER_BYTE);*/
   set_chset_8bit_noctrl();
}

void  set_chset_8bit()
{
   int i;
   clear_chset();
   for(i=0; i<32; i++)
      set_chset_bit(i);
   set_chset_bit(127);
}
void  set_chset_8bit_noctrl()
{
   int i;
   clear_chset();
   for(i=0; i<32; i++)
      set_chset_bit(i);
   for(i=127; i<128+32; i++)
      set_chset_bit(i);
}

void  edit_chset()
{
   WIN *w;
   int i,j;
   static int curr=0;
   byte  old[CHSET_SIZE];
   char  s[256];
   char  chstr[3];
   int    action;

   memcpy(old,chset,sizeof(old));
   w=CreateWin(MIDDLE,MIDDLE,3*16+4,16+6,NORMAL_TEXT_ATTR," Character Set Visualisation ",0);
   DisplayWin(w);

   for(;;)
   {
      SetAttr(NORMAL_TEXT_ATTR);
      Clear();
      if(curr<32)
         sprintf(chstr,"^%c",curr+'@');
      else
         sprintf(chstr,"%c",curr);
      sprintf(s,"The current character is '%s', %3d, 0%03o, 0x%02X",chstr,curr,curr,curr);

      PutStr(2,2,s);
      for(i=0; i<16; i++)
         for(j=0; j<16; j++)
         {
            SetAttr((i<<4)+j==curr?CURR_BUTTON_ATTR:NORMAL_TEXT_ATTR);
            PutCh(i*3+2,j+4,' ');
            PutCh(i*3+3,j+4,(i<<4)+j);
            PutCh(i*3+4,j+4,' ');
         }
      action=GetNextAction();
      switch(action)
      {
      case(CANCEL):
         memcpy(chset,old,sizeof(old));
      case(NEWLINE):
         goto done;
      case(LINE_UP):
         if((curr&15)==0)
            curr|=15;
         else
            curr--;
         break;
      case(LINE_DOWN):
         if((curr&15)==15)
            curr&=~15;
         else
            curr++;
         break;
      case(CHAR_LEFT):
         if((curr&0xF0)==0)
            curr|=0xF0;
         else
            curr-=16;
         break;
      case(CHAR_RIGHT):
         if((curr&0xF0)==0xF0)
            curr&=~0xF0;
         else
            curr+=16;
         break;
      default:
         if(StringTypedLen!=1)
            break;
         switch(StringTyped[0])
         {
         case(' '):
            if(curr<32)
               chset[curr/CHSET_BITS_PER_BYTE]|=(1<<(curr%CHSET_BITS_PER_BYTE));
            else
               chset[curr/CHSET_BITS_PER_BYTE]^=(1<<(curr%CHSET_BITS_PER_BYTE));
            clearok(stdscr,TRUE);
            break;
         }
      }
   }
done:
   flag=REDISPLAY_ALL;
   CloseWin();
   DestroyWin(w);
}

int  choose_ch()
{
   WIN *w;
   int i,j;
   static int curr=0;
   int   res=-1;
   char  s[256];
   char  chstr[3];
   int   action;

   w=CreateWin(MIDDLE,MIDDLE,3*16+4,16+6,DIALOGUE_WIN_ATTR," Character Set ",0);
   DisplayWin(w);

   for(;;)
   {
      SetAttr(DIALOGUE_WIN_ATTR);
      Clear();

      if(curr<32)
         sprintf(chstr,"^%c",curr+'@');
      else
         sprintf(chstr,"%c",curr);
      sprintf(s,"The current character is '%s', %3d, 0%03o, 0x%02X",chstr,curr,curr,curr);

      PutStr(2,2,s);
      for(i=0; i<16; i++)
         for(j=0; j<16; j++)
         {
            SetAttr((i<<4)+j==curr?CURR_BUTTON_ATTR:DIALOGUE_WIN_ATTR);
            PutCh(i*3+2,j+4,' ');
            PutCh(i*3+3,j+4,(i<<4)+j);
            PutCh(i*3+4,j+4,' ');
         }
      action=GetNextAction();
      switch(action)
      {
      case(NEWLINE):
         res=curr;
         goto done;
      case(CANCEL):
         res=-1;
         goto done;
      case(LINE_UP):
         if((curr&15)==0)
            curr|=15;
         else
            curr--;
         break;
      case(LINE_DOWN):
         if((curr&15)==15)
            curr&=~15;
         else
            curr++;
         break;
      case(CHAR_LEFT):
         if((curr&0xF0)==0)
            curr|=0xF0;
         else
            curr-=16;
         break;
      case(CHAR_RIGHT):
         if((curr&0xF0)==0xF0)
            curr&=~0xF0;
         else
            curr+=16;
         break;
      }
   }
done:
   CloseWin();
   DestroyWin(w);
   return(res);
}
void  addch_visual(chtype ch)
{
   attrset(curr_attr->n_attr);
   if(ch&A_ALTCHARSET)
      addch(ch);
   else
   {
      unsigned char ct=ch&A_CHARTEXT;
      if(chset[ct/CHSET_BITS_PER_BYTE]&(1<<(ct%CHSET_BITS_PER_BYTE)))
      {
	 if(ct<32)
	    ct+='@';
	 else if(ct==127)
	    ct='?';
	 else
	    ct='.';
	 attrset(curr_attr->so_attr);
	 addch(ct);
	 attrset(curr_attr->n_attr);
      }
      else
	 addch(ch);
   }
}

bool chset_isprint(int c)
{
   if(c>=256 || c<0)
      return true;
   return !(chset[c/CHSET_BITS_PER_BYTE]&(1<<(c%CHSET_BITS_PER_BYTE)));
}

chtype visualize(struct attr *a,chtype ch)
{
   unsigned char ct=ch&A_CHARTEXT;
   if(!chset_isprint(ct))
   {
      if(ct<32)
	 ct+='@';
      else if(ct==127)
	 ct='?';
      else
	 ct='.';
      return ct|a->so_attr;
   }
   return ch;
}

wchar_t visualize_wchar(wchar_t wc)
{
   if(iswprint(wc))
      return wc;
   if(wc==0x80 && !chset_isprint(wc))
      return '?';
   if(wc<32)
      wc+='@';
   else if(wc==127)
      wc='?';
   else
      wc='.';
   return wc;
}
