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

/* Search & replace for the editor */
/* $Id$ */

#include <config.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"
extern "C" {
#include <regex.h>
}

#define SEARCH  1
#define REPLACE 2

#define FORWARD   1
#define BACKWARD  2

History  SearchHistory;

byte  pattern[256];
int   patlen=0;

byte  replace[256];
int   replen=0;

offs  fndind=0;
num   fndlen=0;

static struct re_registers regs;
static struct re_pattern_buffer rexp;
static bool rexp_compiled=false;
static char fastmap[256];

int   LastOp=0;
int   LastDir=FORWARD;

TextPoint   back_tp;


char *my_memrchr(const char *mem,char ch,int len)
{
   const char *ptr=mem+len;
   while(ptr>mem)
      if(*--ptr==ch)
	 return (char*)ptr;
   return 0;
}

void  NotFound()
{
   Message("Search string not found.");
   CurrentPos=back_tp;
   stdcol=GetCol();
   SetCursor();
   WaitForKey();
}

int   CompilePattern()
{
   if(rexp_compiled)
      regfree(&rexp);

   re_syntax_options=RE_SYNTAX_EMACS;
   const char *err=re_compile_pattern((char*)pattern,patlen,&rexp);
   if(err)
   {
      ErrMsg(err);
      return 0;
   }
   rexp_compiled=true;
   rexp.newline_anchor=1;
   rexp.fastmap=fastmap;
   return 1;
}

int   no_re_search_2(const char *str,const int slen,
		     const char *buf1,const int len1,
		     const char *buf2,const int len2,
		     const int start,const int range)
{
   const char *pos;
   char c0=str[0];

   if(range>0)
   {
      pos=buf1+start;
      while(pos<buf1+len1)
      {
	 if(pos>=buf1+start+range)
	    return -1;
	 pos=(char*)memchr(pos,c0,len1-(pos-buf1));
	 if(pos)
	 {
	    if(pos<=buf1+len1-slen)
	    {
	       if(!memcmp(pos,str,slen))
		  return pos-buf1;
	       pos++;
	    }
	    else if(pos<=buf1+len1+len2-slen)
	    {
	       if(!memcmp(pos,str,len1-(pos-buf1))
	       && !memcmp(buf2,str+len1-(pos-buf1),slen-(len1-(pos-buf1))))
		  return pos-buf1;
	       pos++;
	    }
	    else
	       pos=buf1+len1;
	 }
	 else
	    pos=buf1+len1;
      }
      pos=pos-buf1+buf2-len1;
      while(pos<buf2+len2)
      {
	 if(pos>=buf2+start-len1+range)
	    return -1;
	 pos=(char*)memchr(pos,c0,len2-(pos-buf2));
	 if(pos)
	 {
	    if(pos>=buf2+start-len1+range)
	       return -1;
	    if(pos<=buf2+len2-slen)
	    {
	       if(!memcmp(pos,str,slen))
		  return pos-buf2+len1;
	       pos++;
	    }
	    else
	       break;
	 }
	 else
	    break;
      }
   }
   else	/* range<=0 */
   {
      pos=buf2+start-len1;
      if(pos>buf2+len2-slen)
	 pos=buf2+len2-slen;
      while(pos>=buf2)
      {
	 if(pos<buf2+start-len1+range)
	    return -1;
	 pos=my_memrchr(buf2,c0,pos-buf2+1);
	 if(pos)
	 {
	    if(!memcmp(pos,str,slen))
	       return pos-buf2+len1;
	    pos--;
	 }
	 else
	    pos=buf2-1;
      }
      pos=buf1+len1-(buf2-pos);
      while(pos>=buf1)
      {
	 if(pos<buf1+start+range)
	    return -1;
	 pos=my_memrchr(buf1,c0,pos-buf1+1);
	 if(pos)
	 {
	    if(pos<=buf1+len1-slen)
	    {
	       if(!memcmp(pos,str,slen))
		  return pos-buf1;
	       pos--;
	    }
	    else //if(pos<=buf1+len1+len2-slen)
	    {
	       if(!memcmp(pos,str,len1-(pos-buf1))
	       && !memcmp(buf2,str+len1-(pos-buf1),slen-(len1-(pos-buf1))))
		  return pos-buf1;
	       pos--;
	    }
	 }
	 else
	    break;
      }
   }
   return -1;
}

int    Search(int dir,offs offslim)
{
   Message(dir==FORWARD?"Searching forwards...":"Searching backwards...");

   offs srchpos=CurrentPos;

   if(dir==FORWARD && srchpos>=offslim
   || dir==BACKWARD && srchpos<=offslim)
      return FALSE;

   int res;
   if(noreg)
      res=no_re_search_2((char*)pattern,patlen,buffer,ptr1,
			 buffer+ptr2,BufferSize-ptr2,srchpos,offslim-srchpos);
   else
      res=re_search_2(&rexp,buffer,ptr1,buffer+ptr2,BufferSize-ptr2,
		      srchpos,offslim-srchpos,&regs,offslim);
   if(res==-1)
      return FALSE;

   fndind=res;
   if(noreg)
      fndlen=patlen;
   else
      fndlen=regs.end[0]-res;
   CurrentPos=fndind;
   return(TRUE);
}

void  ReplaceFound()
{
   register int  i;
   offs	 o=0;

   if(!noreg)
   {
      for(i=0; i<replen; i++)
      {
         switch(replace[i])
         {
         case('\\'):
	    if(i < (replen-1))
            {
	       i++;
	       char ch=replace[i];
	       if(ch=='&')
		  ch='0';
	       if(ch>='0' && ch<='9')
	       {
		  unsigned n=ch-'0';
		  if(n<regs.num_regs)
		  {
		     CopyBlock(regs.start[n]+o,regs.end[n]-regs.start[n]);
		     o+=regs.end[n]-regs.start[n];
		  }
		  break;
	       }
	    }
         default:
            InsertChar(replace[i]);
            o++;
	    break;
         case('&'):
            CopyBlock(fndind+o,fndlen);
	    o+=fndlen;
	    break;
         }
      }
   }
   else
   {
      InsertBlock((char*)replace,replen);
   }
   DeleteBlock(0,fndlen);   /* delete found substitute */
}

void  Replace()
{
   int    key=0,action=NO_ACTION;
   int    rcnt=0;
   int    first=TRUE;
   int    i;
   char  str[80];
   num    pline,pcol;

   if(View)
      return;

   back_tp=CurrentPos;

   do
   {
      if(!Search(FORWARD,(key!='#'?TextEnd:BlockEnd)))
      {
         if(first)
            NotFound();
         else if(key=='*' || key=='#')
         {
            if(rcnt==0)
               strcpy(str,"No replasements.");
            else if(rcnt==1)
               strcpy(str,"1 replacement.");
            else
               sprintf(str,"%d replacements.",rcnt);
            if(key!='#')
               CurrentPos=back_tp;
            stdcol=GetCol();
	    CenterView();
            RedisplayAll();
            Message(str);
            SetCursor();
            while(WaitForKey()==ERR);
            return;
         }
         else
            CurrentPos=back_tp;
         stdcol=GetCol();
         flag=TRUE;
         return;
      }
      first=FALSE;
      flag=REDISPLAY_ALL;
      if(key!='*' && key!='#')
      {
         int    oldr=rblock;
         TextPoint   OldBlockBegin=BlockBegin,OldBlockEnd=BlockEnd;
         int    oldh=hide;
         BlockBegin=BlockEnd=CurrentPos;
         BlockEnd+=fndlen;
         hide=0;
         rblock=0;
         CenterView();
         SyncTextWin();
         rblock=oldr;
         BlockBegin=OldBlockBegin;
         BlockEnd=OldBlockEnd;
         hide=oldh;
         Message("Replace? Y,R-replace L-Last replace N,D-do not replace *-replace all #-in block");
         SetCursor();
         action=GetNextAction();
         key=toupper((byte)(StringTyped[0]));
      }
      if(key=='Y' || key=='R' || action==NEWLINE || key=='*' || key=='L')
      {
         ReplaceFound();
         if(key=='*')
            rcnt++;
         if(key=='L')
         {
            flag=TRUE;
            return;
         }
         if(key!='*')
         {
            int oldh=hide;
            hide=1;
            RedisplayAll();
            hide=oldh;
         }
      }
      else if(key=='#')
      {
         if(hide)
            goto ret;
         if(rblock)
         {
            if(GetLine()<BlockBegin.Line())
            {
               GoToLineNum(BlockBegin.Line());
               continue;
            }
            pcol=GetCol();
            pline=GetLine();
            for(i=0; i<fndlen; i++)
            {
               if(pcol<BlockBegin.Col()
                  || (BlockEnd.Col()>BlockBegin.Col()
                      && pcol>=BlockEnd.Col() && CharRel(i)!='\n')
                  || pline>BlockEnd.Line())
                  break;
               if(CharRel(i)=='\t')
                  pcol=Tabulate(pcol);
               else if(CharRel(i)=='\n')
                  pcol=0,pline++;
               else
                  pcol++;
            }
            if(i==fndlen)
               ReplaceFound(),rcnt++;
            else
               MoveRight();
         }
         else
         {
            if(CurrentPos<BlockBegin)
            {
               CurrentPos=BlockBegin;
               continue;
            }
            if(CurrentPos+fndlen<=BlockEnd)
               ReplaceFound(),rcnt++;
            else
               MoveRight();
         }
      }
      else if(key=='N' || key=='D')
         MoveRight();
      else
      {
ret:     flag=TRUE;
         if(action!=CANCEL)
            CurrentPos=back_tp;
         stdcol=GetCol();
         return;
      }
   }
   while(TRUE);
}

void  ContSearch()
{
   if(LastOp==REPLACE && !View)
   {
      ContReplace();
      return;
   }
   back_tp=CurrentPos;
   if(LastDir==FORWARD)
   {
      if(patlen==0)
      {
         StartSearch();
         return;
      }
      if(Eof())
      {
         NotFound();
         return;
      }
      MoveRight();
      if(!Search(FORWARD,TextEnd))
      {
         MoveLeft();
         GetCol();
         NotFound();
      }
      else
         CenterView();
   }
   else
   {
      if(patlen==0)
      {
         StartSearchBackward();
         return;
      }
      if(Bof())
      {
         NotFound();
         return;
      }
      MoveLeft();
      if(!Search(BACKWARD,0))
      {
         MoveRight();
         GetCol();
         NotFound();
      }
      else
         CenterView();
   }
   stdcol=GetCol();
}

void  StartSearch()
{
   if(getstring("Search forwards: ",(char*)pattern,sizeof(pattern)-1,
                                  &SearchHistory,&patlen,NULL)<1)
     return;
   LastDir=FORWARD;
   LastOp=SEARCH;
   if(!CompilePattern())
      return;
   ContSearch();
}
void  StartSearchBackward()
{
   if(getstring("Search backwards: ",(char*)pattern,sizeof(pattern)-1,
                                  &SearchHistory,&patlen,NULL)<1)
     return;
   LastDir=BACKWARD;
   LastOp=SEARCH;
   if(!CompilePattern())
      return;
   ContSearch();
}

void  ContReplace()
{
   if(patlen==0)
   {
      StartReplace();
      return;
   }
   Replace();
}

void  StartReplace()
{
   if(View)
      return;
   if(getstring("Search: ",(char*)pattern,sizeof(pattern)-1,&SearchHistory,&patlen,NULL)<1)
      return;
   if(getstring("Replace: ",(char*)replace,sizeof(replace)-1,&SearchHistory,&replen,NULL)<0)
      return;
   LastOp=REPLACE;
   if(!CompilePattern())
      return;
   ContReplace();
}

void  FindMatch()
{
   byte  op,cl;
   int    dir;
   offs  ptr=CurrentPos;
   int    level = 0;

   if(Eof())
      return;
   op=Char();
   switch(op)
   {
   case '[':
      cl = ']';
      dir = 1;
      break;
   case ']':
      cl = '[';
      dir = -1;
      break;
   case '{':
      cl = '}';
      dir = 1;
      break;
   case '}':
      cl = '{';
      dir = -1;
      break;
   case '(':
      cl = ')';
      dir = 1;
      break;
   case ')':
      cl = '(';
      dir = -1;
      break;
   case '<':
      cl = '>';
      dir = 1;
      break;
   case '>':
      cl = '<';
      dir = -1;
      break;
   default:
      return;
   }
   while(!((dir>0)?EofAt(ptr):BofAt(ptr)))
   {
      ptr+=dir;
      if(CharAt(ptr)==op)
         level++;
      else if(CharAt(ptr)==cl)
      {
         if(level==0)
         {
            CurrentPos=ptr;
            stdcol=GetCol();
            return;
         }
         else
            level--;
      }
   }
   Message("Matching bracket not found");
   SetCursor();
   WaitForKey();
}
