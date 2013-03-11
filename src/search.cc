/*
 * Copyright (c) 1993-2000 by Alexander V. Lukyanov (lav@yars.free.net)
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

/* Search & replace for the editor */
/* $Id: search.cc,v 1.39 2008/12/22 06:45:57 lav Exp $ */

#include <config.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#include "getch.h"
#include "search.h"
#include <alloca.h>

extern "C" {
#include <regex.h>
}

#define SEARCH  1
#define REPLACE 2

#define FORWARD   1
#define BACKWARD  2

History  SearchHistory;

int   match_case=1;

byte  pattern[256];
int   patlen=0;

byte  replace[256];
int   replen=0;

offs  fndind=0;
num   fndlen=0;

static struct re_registers regs;
static struct re_pattern_buffer rexp;
static bool rexp_compiled=false;
static bool word_bounds;
static bool numeric_search;
static bool hex_search;

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
   CurrentPos=back_tp;
   SyncTextWin();
   Message("Search string not found.");
   stdcol=GetCol();
   SetCursor();
   WaitForKey();
}

unsigned char map_to_lower[256];
void map_to_lower_init()
{
   if(map_to_lower[(unsigned char)' '])  // assumes tolower(' ')!='\0' :-)
      return;

   unsigned i;
   /* Map uppercase characters to corresponding lowercase ones.  */
   for (i = 0; i < sizeof(map_to_lower); i++)
      map_to_lower[i] = isupper (i) ? tolower (i) : i;
}

static bool CompilePattern()
{
   if(noreg)
      return true;

   if(rexp_compiled)
   {
      regfree(&rexp);
      rexp_compiled=false;
      memset(&rexp,0,sizeof(rexp));
      if(regs.start)
	 free(regs.start);
      if(regs.end)
	 free(regs.end);
      memset(&regs,0,sizeof(regs));
   }

   re_syntax_options = RE_SYNTAX_EMACS |
      RE_BK_PLUS_QM | RE_CONTEXT_INDEP_ANCHORS | RE_UNMATCHED_RIGHT_PAREN_ORD;
   rexp.translate=0;
   word_bounds=false;
   hex_search=false;
   numeric_search=false;
   bool match_case_now=match_case;

   unsigned char *p=(unsigned char *)alloca(patlen+1);
   memcpy(p,pattern,patlen);
   int len=patlen;
   if(*p=='$') // search options
   {
      for(p++,len--; len>0 && *p!=' '; p++,len--)
      {
	 switch(*p)
	 {
	    case('i'): // ignore case
	       match_case_now=false;
	       break;
	    case('I'): // match case
	       match_case_now=true;
	       break;
	    case('w'):
	       word_bounds=true;
	       break;
	    case('n'):
	       numeric_search=true;
	       break;
	    case('x'):
	       hex_search=true;
	       break;
	    default:
	       goto so_out;
	 }
      }
      if(len>0 && *p==' ')
	 p++,len--;
      if(len==0)
      {
	 p=pattern;
	 len=patlen;
      }
   so_out:;
   }
   if(!match_case_now)
   {
      map_to_lower_init();
      rexp.translate=(RE_TRANSLATE_TYPE)malloc(256);
      memcpy(rexp.translate,map_to_lower,256);
   }

   if(numeric_search || hex_search)
   {
      int radix=0;
      if(hex_search)
	 radix=16;
      char *s=(char*)alloca(len+1);
      char *store=s;
      len=0;
      for(char *t=strtok((char*)p," "); t; t=strtok(0," "))
      {
	 int num=strtoul(t,0,radix);
	 unsigned char n=(unsigned char)num;
	 if(strchr("[]\\*.",n))
	    *store++='\\', len++;
	 *store++=n, len++;
      }
      *store=0;
      p=(unsigned char*)s;
   }

   const char *err=re_compile_pattern((char*)p,len,&rexp);
   if(err)
   {
      ErrMsg(err);
      return false;
   }
   rexp_compiled=true;
   rexp.fastmap=(char*)malloc(256);
   re_compile_fastmap(&rexp);
   return true;
}

int   no_re_search_2(const char *str,const int slen,
		     const char *buf1,const int len1,
		     const char *buf2,const int len2,
		     const int start,const int range)
{
   const char *pos;
   char c0=str[0];

   if(!buf2)
      buf2="";

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
   MessageSync(dir==FORWARD?"Searching forwards...":"Searching backwards...");

   offs srchpos=CurrentPos;

   if((dir==FORWARD  && srchpos>=offslim)
   || (dir==BACKWARD && srchpos<=offslim))
      return FALSE;

   if(!buffer)
      return FALSE;

   char *buf1=0,*buf2=0;
   int len1=0,len2=0;

   if(ptr1>0)
   {
      buf1=buffer;
      len1=ptr1;
   }
   if(BufferSize-ptr2>0)
   {
      buf2=buffer+ptr2;
      len2=BufferSize-ptr2;
   }
   if(buf2 && !buf1)
   {
      buf1=buf2; buf2=0;
      len1=len2; len2=0;
   }
search_again:
   int res;
   if(noreg)
      res=no_re_search_2((char*)pattern,patlen,buf1,len1,
			 buf2,len2,srchpos,offslim-srchpos);
   else
   {
      offs stop=len1+len2;
      if(dir==FORWARD)
	 stop=offslim;
      res=re_search_2(&rexp,buf1,len1,buf2,len2,
		      srchpos,offslim-srchpos,&regs,stop);
   }
   if(res==-1)
      return FALSE;
   if(res==-2)
   {
      ErrMsg("re_search_2 failed");
      return FALSE;
   }

   fndind=res;
   if(noreg)
      fndlen=patlen;
   else
      fndlen=regs.end[0]-res;
   CurrentPos=fndind;
   if(fndlen==0
   || (word_bounds && (isalnum(CharRel(-1)) || isalnum(CharRel(fndlen)))))
   {
      if(dir==FORWARD)
	 srchpos=CurrentPos+1;
      else // dir==BACKWARD
	 srchpos=CurrentPos-1;
      goto search_again;
   }
   return(TRUE);
}

void  ReplaceFound()
{
   offs	 o=0;

   if(!noreg)
   {
      char *scan=(char*)replace;
      int scan_len=replen;
      for(;;)
      {
	 if(scan_len==0)
	    break;
	 char *bslash=(char*)memchr(scan,'\\',scan_len);
	 int len=(bslash?bslash-scan:scan_len);
	 if(len==scan_len-1)
	    len=scan_len;  // backslash at eol
	 if(len>0)
	 {
	    if(buffer_mmapped)
	    {
	       ReplaceBlock(scan,len);
	       CurrentPos+=len;
	    }
	    else
	    {
	       InsertBlock(scan,len);
	       o+=len;
	    }
	    scan+=len;
	    scan_len-=len;
	 }
	 if(!bslash || scan_len==0)
	    break;
	 scan++;
	 scan_len--;
	 if(scan_len==0)
	    break;
	 char ch=*scan++;
	 scan_len--;
	 if(ch=='\\')
	 {
	 insert_ch:
	    if(buffer_mmapped)
	       ReplaceCharMove(ch);
	    else
	    {
	       InsertChar(ch);
	       o++;
	    }
	    continue;
	 }
	 if(ch=='&')
	    ch='0';
	 if(ch>='0' && ch<='9')
	 {
	    unsigned n=ch-'0';
	    if(n<regs.num_regs)
	    {
	       if(!buffer_mmapped)
	       {
		  CopyBlock(regs.start[n]+o,regs.end[n]-regs.start[n]);
		  o+=regs.end[n]-regs.start[n];
	       }
	       else
	       {
		  int rlen=regs.end[n]-regs.start[n];
		  ReplaceBlock(buffer+regs.start[n],rlen);
		  CurrentPos+=rlen;
	       }
	    }
	    continue;
	 }
	 // we've got an unknown backslashed sequence. Insert it verbatim.
	 scan--;
	 scan_len++;
	 ch='\\';
	 goto insert_ch;
      }
   }
   else
   {
      if(buffer_mmapped)
      {
	 ReplaceBlock((char*)replace,replen);
	 CurrentPos+=replen;
      }
      else
	 InsertBlock((char*)replace,replen);
   }
   if(!buffer_mmapped)
	DeleteBlock(0,fndlen);   /* delete found substitute */
   flag=REDISPLAY_ALL;
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

   TextPoint block_end=(rblock
      ?(BlockEnd.Col()==BlockBegin.Col()?NextLine(BlockEnd):BlockEnd.Offset())
      :BlockEnd.Offset());

   do
   {
      if(key!='*' && key!='#')
	 SyncTextWin(); // before possible long search
      if(!Search(FORWARD,(key!='#'?TextEnd:block_end)))
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
            SyncTextWin();
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
         int	     oldr=rblock;
         TextPoint   OldBlockBegin=BlockBegin;
	 TextPoint   OldBlockEnd  =BlockEnd;
         int	     oldh=hide;
         BlockBegin=BlockEnd=CurrentPos;
         BlockEnd+=fndlen;
         hide=0;
         rblock=0;
	 stdcol=GetCol();
         CenterView();
         SyncTextWin();
	 rblock=oldr;
         BlockBegin=OldBlockBegin;
         BlockEnd=OldBlockEnd;
         hide=oldh;
         Message("Replace? Y,R-replace L-Last replace N,D-do not replace *-replace all #-in block");
         SetCursor();
      next_action:
         action=GetNextAction();
	 switch(action)
	 {
	 case REFRESH_SCREEN:
	    refresh();
	    goto next_action;
	 default:
	    if(StringTypedLen!=1)
	       goto ret;
	 }
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
                                  &SearchHistory,&patlen,"SearchHelp"," Search Help ")<1)
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
                                  &SearchHistory,&patlen,"SearchHelp"," Search Help ")<1)
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
   if(getstring("Search: ",(char*)pattern,sizeof(pattern)-1,&SearchHistory,&patlen,"SearchHelp"," Search Help ")<1)
      return;
   if(getstring("Replace: ",(char*)replace,sizeof(replace)-1,&SearchHistory,&replen,"ReplaceHelp"," Replace Help ")<0)
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
   Message("Matching bracket not found.");
   SetCursor();
   WaitForKey();
}
