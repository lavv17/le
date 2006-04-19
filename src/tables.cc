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

/*
 * This code actually is VERY old and must be reorganized or deleted,
 * but I don't have time nor wish to do that
 */
#ifdef WITH_MODIFYKEY

#include <config.h>
#include <string.h>
#include <unistd.h>
#include "edit.h"

#ifndef  MSDOS
/************************** TABLES FOR D211 *****************************/
char  rl_d211[]={
'“','Q','⁄','W',' ','E','”','R','’','T','„','Y','ÿ','U','–','I',
'Œ','O','œ','P','ì','[','≤',']','¬','A','‘','S','˝','D','À','F',
'Ã','G','Æ','H','◊','J','›','K','ﬁ','L','í','\\','ı','Z','€','X',
'»','C','Ÿ','V','√','B','','N','Õ','M','±','q','ï','w','Ö','e',
'¯','r','ü','t','∑','y','','u','â','i','è','o','','p','ö','{',
'ú','}','ﬂ','a','','s','Ñ','d','Ü','f','á','g','à','h','ä','j',
'ã','k','å','l','õ','|','î','z','∞','x','É','c','˜','v','Ç','b',
'é','n','ç','m','Ä','`','¡','~','ë','@','ù','$','º','^','´','%',
0};

/************************* TABLES FOR VTA2000 ************************/
char  rl_vta2000[]={
'¡','@','¬','A','√','B','»','C','˝','D',' ','E',
'À','F','Ã','G','Æ','H','–','I','◊','J','›','K','ﬁ','L','Õ','M',
'','N','Œ','O','œ','P','“','Q','”','R','‘','S','’','T','ÿ','U',
'Ÿ','V','⁄','W','€','X','„','Y','ı','Z','ö','[','õ','\\','ú',']',
'ù','_','Ä','`','ﬂ','a','Ç','b','É','c','Ñ','d','Ö','e','Ü','f',
'á','g','à','h','â','i','ä','j','ã','k','å','l','ç','m','é','n',
'è','o','','p','±','q','¯','r','','s','ü','t','','u','˜','v',
'ï','w','∞','x','∑','y','î','z','ì','{','í','|','≤','}','ë','~',
0};

/************************* TABLES FOR DEFAULT ************************/
char  rl_default[]={
0};

/*************************** FUNCTIONS ****************************/

char  *table=        rl_default;
#else
char  rl_ibm[]={0};
char  *table=rl_ibm;
#endif

void  InitTables()
{
   init_chset();
#ifndef  MSDOS
   if(!strcmp(TERM,"vta2000"))
      table=rl_vta2000;
   else if(!strcmp(TERM,"d211"))
      table=rl_d211;
#endif
}

#endif // WITH_MODIFYKEY
