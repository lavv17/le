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

typedef unsigned char   byte;
#include "rus.h"

int   coding=0;

int   isrussian(byte ch)
{
   switch(coding)
   {
   case(KOI8):
      return(ch>=0xC0);
   case(D211_KOI):
      return(ch>=0xC0 && ch!=0xFF && ch!=0xDF);
   case(JO_ALT):
      return((ch>=0x80 && ch<0xB0) || (ch>=0xE0 && ch<0xF2));
   case(ALT):
      return((ch>=0x80 && ch<0xB0) || (ch>=0xE0 && ch<0xF0));
   case(MAIN):
      return(ch>=0xB0 && ch<0xF2);
   }
   return(0);
}
int   islowerrus(byte ch)
{
   switch(coding)
   {
   case(KOI8):
      return(ch>=0xC0 && ch<0xE0);
   case(D211_KOI):
      return(ch>=0xE0 && ch<0xFF);
   case(JO_ALT):
      return((ch>=0xA0 && ch<0xB0) || (ch>=0xE0 && ch<0xF1));
   case(ALT):
      return((ch>=0xA0 && ch<0xB0) || (ch>=0xE0 && ch<0xF0));
   case(MAIN):
      return(ch>=0xD0 && ch<0xF1);
   }
   return(0);
}
int   isupperrus(byte ch)
{
   switch(coding)
   {
   case(KOI8):
      return(ch>=0xE0);
   case(D211_KOI):
      return(ch>=0xC0 && ch<0xDF);
   case(JO_ALT):
      return((ch>=0x80 && ch<0xA0) || ch==0xF1);
   case(ALT):
      return(ch>=0x80 && ch<0xA0);
   case(MAIN):
      return((ch>=0xB0 && ch<0xD0) || ch==0xF1);
   }
   return(0);
}
byte  tolowerrus(byte ch)
{
   if(!isupperrus(ch))
      return(ch);
   
    switch(coding)
   {
   case(KOI8):
      return(ch-0x20);
   case(D211_KOI):
      return(ch+0x20);
   case(JO_ALT):
      if(ch==0xF0)
         return(0xF1);
   case(ALT):
      if(ch>=0x80 && ch<0x90)
         return(ch+0x20);
      return(ch+0x50);
   case(MAIN):
      if(ch==0xF0)
         return(0xF1);
      return(ch+0x20);
   }
   return(ch);
}
byte  toupperrus(byte ch)
{
   if(!islowerrus(ch))
      return(ch);
   
   switch(coding)
   {
   case(KOI8):
      return(ch+0x20);
   case(D211_KOI):
      return(ch-0x20);
   case(JO_ALT):
      if(ch==0xF1)
         return(0xF0);
   case(ALT):
      if(ch>=0xE0 && ch<0xF0)
         return(ch-0x50);
      return(ch-0x20);
   case(MAIN):
      if(ch==0xF1)
         return(0xF0);
      return(ch-0x20);
   }
   return(ch);
}
