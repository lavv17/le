/* 
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

/*
 * This code actually is VERY old and must be reorganized or deleted,
 * but I don't have time nor wish to do that
 */

#include <string.h>
#include <unistd.h>
#include "edit.h"

#ifndef  MSDOS
/************************** TABLES FOR D211 *****************************/
char  rl_d211[]={
'Ñ','Q','×','W','Å','E','Ò','R','Ô','T','Ù','Y','Õ','U','É','I',
'Ï','O','Ð','P','û','[','ý',']','Á','A','Ó','S','Ä','D','Æ','F',
'Ç','G','È','H','Ê','J','Ë','K','Ì','L','ü','\\','Ú','Z','Ø','X',
'Ã','C','Ö','V','Â','B','Î','N','Í','M','ñ','q','÷','w','å','e',
'ò','r','ô','t','ù','y','õ','u','é','i','ï','o','ð','p','Û','{',
'Ý','}','á','a','ó','s','ä','d','æ','f','ç','g','è','h','ê','j',
'ë','k','ì','l','Ü','|','ú','z','ø','x','ã','c','ö','v','â','b',
'î','n','í','m','à','`','À','~','þ','@','Þ','$','¬','^','®','%',
0};

/************************* TABLES FOR VTA2000 ************************/
char  rl_vta2000[]={
'À','@','Á','A','Â','B','Ã','C','Ä','D','Å','E',
'Æ','F','Ç','G','È','H','É','I','Ê','J','Ë','K','Ì','L','Í','M',
'Î','N','Ï','O','Ð','P','Ñ','Q','Ò','R','Ó','S','Ô','T','Õ','U',
'Ö','V','×','W','Ø','X','Ù','Y','Ú','Z','Û','[','Ü','\\','Ý',']',
'Þ','_','à','`','á','a','â','b','ã','c','ä','d','å','e','æ','f',
'ç','g','è','h','é','i','ê','j','ë','k','ì','l','í','m','î','n',
'ï','o','ð','p','ñ','q','ò','r','ó','s','ô','t','õ','u','ö','v',
'÷','w','ø','x','ù','y','ú','z','û','{','ü','|','ý','}','þ','~',
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
