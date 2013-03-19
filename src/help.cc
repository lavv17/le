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
#include <ctype.h>
#include "edit.h"
#include "keymap.h"
#include <string.h>
#include <stdlib.h>

#if 0
void  Help(char ***h,char *title)
{
   static   struct   menu
   c[]={
   {"  &Cancel  ",MIDDLE,FDOWN-2},
   {NULL}},
   pnc[]={
   {" &Previous ",MIDDLE-12,FDOWN-2},
   {"   &Next   ",MIDDLE,FDOWN-2},
   {"  &Cancel  ",MIDDLE+12,FDOWN-2},
   {NULL}},
   npc[]={
   {"   &Next   ",MIDDLE-12,FDOWN-2},
   {" &Previous ",MIDDLE,FDOWN-2},
   {"  &Cancel  ",MIDDLE+12,FDOWN-2},
   {NULL}},
   pc[]={
   {" &Previous ",MIDDLE-6,FDOWN-2},
   {"  &Cancel  ",MIDDLE+6,FDOWN-2},
   {NULL}},
   nc[]={
   {"   &Next   ",MIDDLE-6,FDOWN-2},
   {"  &Cancel  ",MIDDLE+6,FDOWN-2},
   {NULL}};
   WIN   *HelpWin;
   int   page=0;
   int   backward=0,res,line;

   HelpWin=CreateWin(MIDDLE,MIDDLE,42,21,HELP_WIN_ATTR,title,0);
   DisplayWin(HelpWin);

   do
   {
      Clear();
      for(line=2; line<Upper->h-4 && h[page][line-2]; line++)
         PutStr(2,line,h[page][line-2]);
      if(page==0 && !h[1])
         res=ReadMenu(c,HORIZ,HELP_WIN_ATTR,CURR_BUTTON_ATTR);
      else if(page==0)
         res=ReadMenu(nc,HORIZ,HELP_WIN_ATTR,CURR_BUTTON_ATTR);
      else if(!h[page+1])
         res=ReadMenu(pc,HORIZ,HELP_WIN_ATTR,CURR_BUTTON_ATTR);
      else
         res=ReadMenu(backward?pnc:npc,HORIZ,HELP_WIN_ATTR,CURR_BUTTON_ATTR);
      switch(res)
      {
      case('C'):
         res=0;
         break;
      case('N'):
         page++;
         backward=0;
         break;
      case('P'):
         page--;
         backward=1;
         break;
      }
   }
   while(res);

   CloseWin();
   DestroyWin(HelpWin);
}
#endif //0

static char *LoadHelp(const char *tag)
{
   char *help=(char*)malloc(0x10000);
   if(!help)
      return 0;
   help[0]=0;
   FILE *hf=fopen(PKGDATADIR "/le.hlp","r");
   if(!hf)
   {
      free(help);
      return 0;
   }
   char buf[256];
   char this_tag[256];
   bool tag_match=false;
   buf[255]=0;
   while(fgets(buf,sizeof(buf)-1,hf)!=0)
   {
      if(1==sscanf(buf,"[%[^]]]",this_tag))
      {
	 if(tag_match)
	    return help;
	 if(!strcasecmp(tag,this_tag))
	    tag_match=true;
	 continue;
      }
      if(tag_match)
      {
	 // FIXME: dynamically increment size.
	 if(strlen(help)>0x10000-256)
	 {
	    fclose(hf);
	    return help;
	 }
	 strcat(help,buf);
      }
   }
   fclose(hf);
   if(tag_match)
      return help;
   free(help);
   return 0;
}

void  Help(const char *helpf,const char *title)
{
   char *help=LoadHelp(helpf);
   if(!help)
      return;
   const char  *ptr=help;
   WIN   *HelpWin;
   const int v_m=1;
   const int h_m=2;

   HelpWin=CreateWin(MIDDLE,MIDDLE,48,22,HELP_WIN_ATTR,title,0);
   DisplayWin(HelpWin);

   for(;;)
   {
      int line=v_m;
      const char *c;

      SetAttr(HELP_WIN_ATTR);
      for(c=ptr; *c; c++)
      {
         if(line >= HelpWin->h-v_m)
            break;
         int col=h_m;
         for( ; *c && *c!='\n'; c++)
         {
            if(col >= HelpWin->w-h_m)
               break;

            PutCh(col++,line,(unsigned char)*c);
         }
         while(col < HelpWin->w-h_m)
            PutCh(col++,line,' ');
         c=strchr(c,'\n');
         if(!c)
            break;
         line++;
      }
      while(line < HelpWin->h-v_m)
      {
         for(int col=h_m; col<HelpWin->w-h_m; col++)
            PutCh(col,line,' ');
         line++;
      }
      Message("Use Up,Down,PageUp,PageDown to scroll help text. Ctrl-X - cancel");
      curs_set(0);
      move(LINES-1,COLS-1);

      int action=GetNextAction();
      int i;
      switch(action)
      {
      case(LINE_DOWN):
         c=strchr(ptr,'\n');
         if(!c)
            break;
         ptr=c+1;
         break;
      case(NEXT_PAGE):
         c=ptr;
         for(i=HelpWin->h-v_m*2-1; i>0; i--)
         {
            c=strchr(c,'\n');
            if(!c)
               break;
            c++;
         }
         if(c)
            ptr=c;
         break;
      case(LINE_UP):
         if(ptr==help)
            break;
         ptr--;
         while(ptr>help && ptr[-1]!='\n')
            ptr--;
         break;
      case(PREV_PAGE):
         c=ptr;
         for(i=HelpWin->h-v_m*2-1; i>0; i--)
         {
            if(c==help)
               break;
            c--;
            while(c>help && c[-1]!='\n') c--;
         }
         ptr=c;
         break;
      case(NEWLINE):
      case(QUIT_EDITOR):
         goto quit;
      }
      c=help+strlen(help);
      for(i=HelpWin->h-v_m*2-1; i>0; i--)
      {
         if(c>help && c[-1]=='\n')
            c--;
         while(c>help && c[-1]!='\n')
            c--;
      }
      if(c<ptr)
         ptr=c;
   }
quit:
   CloseWin();
   DestroyWin(HelpWin);
   free(help);
}
