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

/* getstr.c : Get string from the user */

#include <config.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#include "getch.h"
#include "mb.h"

int   getstring(const char *pr,char *buf,int maxlen,History* history,int *len,
                const char *help,const char *title)
{
   int      pos,col,action,ch;
   int      width,start;
   int      shift,i,c,stuff;
   int	    ch_len;
   HistoryLine *hl;

   if(history)
      history->Open();

   if(message_sp==0)
      message_sp=1;
   width=COLS-strlen(pr)-1;
   pos=0;
   col=0;
   shift=0;
   start=TRUE;
   if(len==NULL)
   {
      len=(&stuff);
      (*len)=strlen(buf);
   }

   if(history)
   {
      for(;;)
      {
	 hl=history->Prev();
	 if(!hl)
	    break;
	 if(*len==hl->len && !memcmp(buf,hl->line,*len))
	    break;
      }
   }

   do
   {
      if(col==-1)
	 mb_get_col(buf,pos,&col,*len);
      if(col-shift>width)
         shift=col-width;
      if(col-shift<0)
         shift=col;
      SetAttr(STATUS_LINE_ATTR);
      mvaddstr(LINES-1,0,(char*)pr);
      for(i=0,c=0; c<=width+shift && i<(*len); )
      {
#if USE_MULTIBYTE_CHARS
	 if(mb_mode)
	 {
	    wchar_t wc;
	    int ch_len=mbtowc(&wc,buf+i,(*len)-i);
	    if(ch_len<1 || (ch_len==1 && !chset_isprint(buf[i+shift])))
	    {
	       if(c>=shift)
		  addch_visual((byte)buf[i+shift]);
	       i++; c++;
	    }
	    else
	    {
	       wchar_t vwc=visualize_wchar(wc);
	       if(c>=shift)
	       {
		  if(wc!=vwc)
		     attrset(curr_attr->so_attr);
		  addnwstr(&vwc,1);
		  attrset(curr_attr->n_attr);
	       }
	       i+=ch_len;
	       c+=wcwidth(vwc);
	    }
	 }
	 else // note the following block
#endif
	 {
	    if(c>=shift)
	       addch_visual((byte)buf[i+shift]);
	    i++; c++;
	 }
      }
      while(c++<=width+shift)
	 addch(' ');

      move(LINES-1,col-shift+strlen(pr));
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
	    col=0;
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
	    col=0;
            start=1;
            break;
         case(BACKSPACE_CHAR):
            if(pos==0)
               break;
	    mb_char_left(buf,&pos,&col,*len);
         case(DELETE_CHAR):
            if(pos==*len)
               break;
	    mblen(0,0);
	    ch_len=mblen(buf+pos,*len-pos);
	    if(ch_len<1)
	       ch_len=1;
            for(i=pos; i+ch_len<=*len; i++)
               buf[i]=buf[i+ch_len];
            (*len)-=ch_len;
            start=FALSE;
            break;
         case(LINE_END):
            pos=(*len);
	    col=-1;
            start=0;
            break;
         case(LINE_BEGIN):
            pos=col=start=0;
            break;
         case(CHAR_LEFT):
            start=0;
	    if(pos)
	       mb_char_left(buf,&pos,&col,*len);
            break;
         case(DELETE_TO_EOL):
            (*len)=pos;
            buf[pos]=0;
            break;
         case(CHAR_RIGHT):
            start=0;
            if(pos<(*len))
	       mb_char_right(buf,&pos,&col,*len);
            break;
         case(CHOOSE_CHAR):
            ch=choose_ch();
            if(ch==-1)
               break;
	    StringTyped[0]=ch;
	    StringTypedLen=1;
            goto ins;
         case(ENTER_CHAR_CODE):
            ch=getcode_char();
            if(ch==-1)
               break;
	    StringTyped[0]=ch;
	    StringTypedLen=1;
            goto ins;
         case(ENTER_WCHAR_CODE):
            ch=getcode_wchar();
            if(ch==-1)
               break;
	    StringTypedLen=wctomb((char*)StringTyped,ch);
	    if(StringTypedLen<1)
	       break;
            goto ins;
         case(ENTER_CONTROL_CHAR):
	    ch=GetRawKey();
	    StringTyped[0]=ch;
	    StringTypedLen=1;
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
            if((*len)+StringTypedLen>maxlen)
               break;
            for(i=(*len); i>=pos; i--)
               buf[i+StringTypedLen]=buf[i];
            (*len)+=StringTypedLen;
	    for(i=0; i<StringTypedLen; i++)
	       buf[pos++]=ModifyKey(StringTyped[i]);
	    col=-1;
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
