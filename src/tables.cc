/*
 * Copyright (c) 1993-2006 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "edit.h"
#include "mb.h"

#ifndef  MSDOS
/************************** TABLES FOR D211 *****************************/
static const char rl_d211[]={
'“','Q','⁄','W',' ','E','”','R','’','T','„','Y','ÿ','U','–','I',
'Œ','O','œ','P','ì','[','≤',']','¬','A','‘','S','˝','D','À','F',
'Ã','G','Æ','H','◊','J','›','K','ﬁ','L','í','\\','ı','Z','€','X',
'»','C','Ÿ','V','√','B','','N','Õ','M','±','q','ï','w','Ö','e',
'¯','r','ü','t','∑','y','','u','â','i','è','o','','p','ö','{',
'ú','}','ﬂ','a','','s','Ñ','d','Ü','f','á','g','à','h','ä','j',
'ã','k','å','l','õ','|','î','z','∞','x','É','c','˜','v','Ç','b',
'é','n','ç','m','Ä','`','¡','~','ë','@','ù','$','º','^','´','%',
0};

/************************* TABLES FOR VTA2000 ***************************/
static const char rl_vta2000[]={
'¡','@','¬','A','√','B','»','C','˝','D',' ','E',
'À','F','Ã','G','Æ','H','–','I','◊','J','›','K','ﬁ','L','Õ','M',
'','N','Œ','O','œ','P','“','Q','”','R','‘','S','’','T','ÿ','U',
'Ÿ','V','⁄','W','€','X','„','Y','ı','Z','ö','[','õ','\\','ú',']',
'ù','_','Ä','`','ﬂ','a','Ç','b','É','c','Ñ','d','Ö','e','Ü','f',
'á','g','à','h','â','i','ä','j','ã','k','å','l','ç','m','é','n',
'è','o','','p','±','q','¯','r','','s','ü','t','','u','˜','v',
'ï','w','∞','x','∑','y','î','z','ì','{','í','|','≤','}','ë','~',
0};
#endif

static const char *table;

#if USE_MULTIBYTE_CHARS
static unsigned short wtable[]={
0x401,'~', '"','@', ':','%', ',','^', '.','&', ';','*',
0x419,'Q', 0x426,'W', 0x423,'E', 0x41A,'R', 0x415,'T', 0x41D,'Y', 0x413,'U', 0x428,'I', 0x429,'O', 0x417,'P', 0x425,'{', 0x42A,'}',
0x424,'A', 0x42B,'S', 0x412,'D', 0x410,'F', 0x41F,'G', 0x420,'H', 0x41E,'J', 0x41B,'K', 0x414,'L', 0x416,':', 0x42D,'"',
0x42F,'Z', 0x427,'X', 0x421,'C', 0x41C,'V', 0x418,'B', 0x422,'N', 0x42C,'M', 0x411,'<', 0x42E,'>',
0x451,'`',
0x439,'q', 0x446,'w', 0x443,'e', 0x43A,'r', 0x435,'t', 0x43D,'y', 0x433,'u', 0x448,'i', 0x449,'o', 0x437,'p', 0x445,'[', 0x44A,']',
0x444,'a', 0x44B,'s', 0x432,'d', 0x430,'f', 0x43F,'g', 0x440,'h', 0x43E,'j', 0x43B,'k', 0x434,'l', 0x436,';', 0x44D,'\'',
0x44F,'z', 0x447,'x', 0x441,'c', 0x43C,'v', 0x438,'b', 0x442,'n', 0x44C,'m', 0x431,',', 0x44E,'.',
0};
#endif

void  InitModifyKeyTables()
{
#ifndef MSDOS
   if(!strcmp(TERM,"vta2000"))
      table=rl_vta2000;
   else if(!strcmp(TERM,"d211"))
      table=rl_d211;
#endif
}

int   ModifyKey(int key)
{
   int   i;
   if(inputmode!=LATIN && key!='\n' && key!='\t')
   {
      if(inputmode==RUSS)
      {
	 if(table) {
	    for(i=0; table[i]!='\0'; i+=2)
	    {
	       if(table[i+1]==key)
	       {
		  key=table[i];
		  break;
	       }
	    }
#if USE_MULTIBYTE_CHARS
	 } else {
	    for(i=0; wtable[i]; i+=2) {
	       if(wtable[i+1]==key) {
		  char mb[MB_LEN_MAX];
		  key=wtable[i];
		  int len=wctomb(mb,key);
		  if(len<=0)
		     return wtable[i+1];
		  for(i=len; i>1; i--)
		     ungetch((unsigned char)mb[i-1]);
		  return (unsigned char)mb[0];
	       }
	    }
#endif//USE_MULTIBYTE_CHARS
	 }
      }
      else
         key+=128;
   }
   return(key);
}
