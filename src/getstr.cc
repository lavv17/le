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

/* getstr.c : Get string from the user */

#include <config.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"
#include "getch.h"

int   getstring(const char *pr,char *buf,int maxlen,History* history,int *len,
                const char *help,const char *title)
{
   int      pos,action,ch;
   int      width,start;
   int      shift,i,stuff;
   HistoryLine *hl;

   if(history)
      history->Open();

   if(message_sp==0)
      message_sp=1;
   width=COLS-strlen(pr)-1;
   pos=0;
   shift=0;
   start=TRUE;
   if(len==NULL)
   {
      len=(&stuff);
      if(history)
         buf[(*len)=0]='\0';
      else
         (*len)=strlen(buf);
   }
   else
   {
      if(history)
         buf[(*len)=0]='\0';
   }
   do
   {
      if(pos-shift>width)
         shift=pos-width;
      if(pos-shift<0)
         shift=pos;
      SetAttr(STATUS_LINE_ATTR);
      mvaddstr(LINES-1,0,pr);
      for(i=0; i<width && i+shift<(*len); i++)
      {
         addch_visual((byte)buf[i+shift]);
      }
      while(i++<=width)
	 addch(' ');

      move(LINES-1,pos-shift+strlen(pr));
      curs_set(1);
      action=GetNextAction();
      switch(action)
      {
         case(EDITOR_HELP):
            if(!help)
               break;
            Help(help,title);
            break;
         case(CANCEL):
            return(-1);
         case(NEWLINE):
            if(history!=NULL && *len!=0)
               *history+=HistoryLine(buf,*len);
            buf[*len]=0;
            return(*len);
         case(LINE_UP):
            if(history==NULL)
               break;
            hl=history->Prev();
            if(hl)
               memcpy(buf,hl->line,*len=hl->len);
            else
               *len=0;
            pos=0;
	    start=1;
            break;
         case(LINE_DOWN):
            if(history==NULL)
               break;
            hl=history->Next();
            if(hl)
               memcpy(buf,hl->line,*len=hl->len);
            else
               *len=0;
            pos=0;
            start=1;
            break;
         case(BACKSPACE_CHAR):
            if(pos==0)
               break;
            pos--;
         case(DELETE_CHAR):
            if(pos==*len)
               break;
            for(i=pos; i<*len; i++)
               buf[i]=buf[i+1];
            (*len)--;
            start=FALSE;
            break;
         case(LINE_END):
            pos=(*len);
            start=0;
            break;
         case(LINE_BEGIN):
            pos=start=0;
            break;
         case(CHAR_LEFT):
            start=0;
            if(pos)
               pos--;
            break;
         case(DELETE_TO_EOL):
            (*len)=pos;
            buf[pos]=0;
            break;
         case(CHAR_RIGHT):
            start=0;
            if(pos<(*len))
               pos++;
            break;
         case(CHOOSE_CHAR):
            ch=choose_ch();
            if(ch==-1)
               break;
            goto ins;
         case(ENTER_CHAR_CODE):
            ch=getcode();
            if(ch==-1)
               break;
            goto ins;
         case(ENTER_CONTROL_CHAR):
            ch=GetRawKey();
            goto ins;
         default:
            if(StringTypedLen!=1)
               break;
            ch=StringTyped[0];
            if(ch>=0 && ch<' ')
               break;
      ins:
            if(start)
            {
               buf[(*len)=shift=pos=0]='\0';
               if(history)
                  history->Open();  // reopen history so it seeks to the begin
            }
            start=FALSE;
            if((*len)==maxlen)
               break;
            for(i=(*len); i>=pos; i--)
               buf[i+1]=buf[i];
            (*len)++;
            buf[pos++]=ModifyKey(ch);
      }
   }
   while(TRUE);
/*NOTREACHED*/
}

int   ModifyKey(int key)
{
   int   i;
   if(inputmode!=LATIN && key!='\n' && key!='\t')
   {
      if(inputmode==RUSS)
         for(i=0; table[i]!='\0'; i+=2)
         {
            if(table[i+1]==key)
            {
               key=table[i];
               break;
            }
         }
      else
         key+=128;
   }
   return(key);
}
