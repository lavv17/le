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

#include <config.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"

#define SEARCH  1
#define REPLACE 2

#define FORWARD    1
#define BACKWARD  2

#define  BITS_PER_WORD  (sizeof(unsigned)*8)

History  SearchHistory;

byte  pattern[256];
int   patlen=0;
struct srch_char
{
   int	 repeat;
   unsigned cmap[256/BITS_PER_WORD];
}
      compiled_pattern[256],
      *compiled_patlen;

char  first_char[256];

byte  replace[256];
int   replen=0;
byte  found[256];
int   fndlen=0;

int   LastOp=0;
int   LastDir=FORWARD;

offs  offslim;
num   linelim;

TextPoint   back_tp;

void  NotFound()
{
   Message("Search string not found.");
   CurrentPos=back_tp;
   stdcol=GetCol();
   SetCursor();
   WaitForKey();
}

void  CompilePattern()
{
   int	 j=0,k,lim1,lim2,i;
   int   NOT,start;

   for(i=0; i<patlen; i++)
   {
      compiled_pattern[j].repeat=0;
      memset(&(compiled_pattern[j].cmap),0,32);

      if(noreg)
         goto _default;

      switch(pattern[i])
      {
      case('.'):
         memset(&(compiled_pattern[j].cmap),255,32);
         j++;
         break;
      case('*'):
         if(j<1)
            goto _default;
         compiled_pattern[j-1].repeat=-1;
         break;
      case('\\'):
         i++;
         goto _default;
      case('['):
	 for(k=i+1; k<patlen; k++)
	 {
	    if(pattern[k]=='\\')
	       k++;
	    else if(pattern[k]==']')
	       break;
	 }
	 if(k>=patlen)
            goto _default;
	 i++;
	 if(pattern[i]=='^')
	    NOT=TRUE,i++;
	 else
	    NOT=FALSE;
	 start=i;
	 i=k;
	 for(k=start; pattern[k]!=']'; k++)
	 {
	    if(pattern[k]=='-' && k>start && pattern[k+1]!=']')
	    {
	       lim1=pattern[k-1];
	       k++;
	       if(pattern[k]=='\\')
		  k++;
	       lim2=pattern[k];
               while(lim1<=lim2)
               {
                  compiled_pattern[j].cmap[lim1/BITS_PER_WORD]|=(1<<(lim1%BITS_PER_WORD));
                  lim1++;
               }
	    }
	    else
	    {
	       if(pattern[k]=='\\')
		  k++;
	       if(pattern[k+1]!='-' || pattern[k+2]==']')
                  compiled_pattern[j].cmap[pattern[k]/BITS_PER_WORD]|=(1<<(pattern[k]%BITS_PER_WORD));
	    }
         }
         if(NOT)
         {
            for(unsigned q=0; q<256/BITS_PER_WORD; q++)
               compiled_pattern[j].cmap[q]^=(unsigned)-1;
         }
         j++;
         break;
      default:
      _default:
	 compiled_pattern[j].cmap[pattern[i]/BITS_PER_WORD]|=(1<<(pattern[i]%BITS_PER_WORD));
         j++;
         break;
      }
   }
   compiled_patlen=compiled_pattern+j;

   for(k=0; k<256; k++)
      first_char[k]=(compiled_pattern[0].cmap[k/BITS_PER_WORD]>>(k%BITS_PER_WORD))&1;
   for(i=1; i<j && compiled_pattern[i-1].repeat; i++)
      for(k=0; k<256; k++)
         first_char[k]|=(compiled_pattern[i].cmap[k/BITS_PER_WORD]>>(k%BITS_PER_WORD))&1;
}

static inline
int   CharMatch(register struct srch_char *pch,register byte tch)
{
   return(pch->cmap[tch/BITS_PER_WORD]&(1<<(tch%BITS_PER_WORD)));
}

inline
offs  FindCharForward(register char *sch,register offs start,offs lim)
{
   while(start<lim)
   {
      if(sch[CharAt_NoCheck(start)])
         return(start);
      start++;
   }
   return(-1);
}
inline
offs  FindCharBackward(register char *sch,register offs start,offs lim)
{
   while(start>=lim)
   {
      if(sch[CharAt_NoCheck(start)])
         return(start);
      start--;
   }
   return(-1);
}

static inline
int    Match(register offs ptr,register struct srch_char *pch)
{
   register int  ch;

   for(;;)
   {
      if(!pch->repeat)
      {
	 if(EofAt(ptr))
	    return(FALSE);
	 ch=CharAt_NoCheck(ptr++);
	 if(CharMatch(pch,ch) && fndlen<256)
         {
   	    found[fndlen++]=ch;
   	    if(++pch<compiled_patlen)
	       continue;
            return(TRUE);
         }
         return(FALSE);
      }
      else
      {
	 int oldfndlen=fndlen;
	 if(fndlen>0 && (pch+1>=compiled_patlen || Match(ptr,pch+1)))
	    return(TRUE);
	 fndlen=oldfndlen;

	 if(EofAt(ptr))
	    return(FALSE);
	 ch=CharAt_NoCheck(ptr++);
	 if(CharMatch(pch,ch) && fndlen<256)
         {
   	    found[fndlen++]=ch;
            continue;
         }
         return(FALSE);
      }
   }
}

int    Search(int dir)
{
   Message(dir==FORWARD?"Searching forwards...":"Searching backwards...");

   register offs srchpos=CurrentPos;

   if(dir==FORWARD)
   {
      if(rblock)
	 offslim=TextPoint(linelim+1,0);
      if(offslim>Size())
         offslim=Size();

      for(;;)
      {
	 srchpos=FindCharForward(first_char,srchpos,offslim);
         if(srchpos==-1)
            break;
	 fndlen=0;
         if(Match(srchpos,compiled_pattern))
         {
            CurrentPos=srchpos;
            stdcol=GetCol();
            return(TRUE);
         }
         else
            srchpos++;
      }
   }
   else
   {
      if(rblock)
	 offslim=TextPoint(linelim,0);
      if(offslim<0)
         offslim=0;

      for(;;)
      {
         srchpos=FindCharBackward(first_char,srchpos,offslim);
         if(srchpos==-1)
            break;
	 fndlen=0;
         if(Match(srchpos,compiled_pattern))
         {
            CurrentPos=srchpos;
            stdcol=GetCol();
            return(TRUE);
         }
         else
            srchpos--;
      }
   }
   return(FALSE);
}

void  ReplaceFound()
{
   register int  i;

   if(!noreg)
   {
      for(i=0; i<replen; i++)
      {
         switch(replace[i])
         {
         case('\\'):
            if(i < (replen-1))
               i++;
         default:
            InsertChar(replace[i]);
            break;
         case('&'):
            InsertBlock((char*)found,fndlen);
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
      if(key!='#')
      {
         linelim=TextEnd.Line();
         offslim=TextEnd.Offset();
      }
      else
      {
         linelim=BlockEnd.Line();
         offslim=BlockEnd.Offset();
      }
      if(!Search(FORWARD))
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
      offslim=Size();
      linelim=TextEnd.Line();
      if(Eof())
      {
         NotFound();
         return;
      }
      MoveRight();
      if(!Search(FORWARD))
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
      offslim=0;
      linelim=0;
      if(Bof())
      {
         NotFound();
         return;
      }
      MoveLeft();
      if(!Search(BACKWARD))
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
   CompilePattern();
   ContSearch();
}
void  StartSearchBackward()
{
   if(getstring("Search backwards: ",(char*)pattern,sizeof(pattern)-1,
                                  &SearchHistory,&patlen,NULL)<1)
     return;
   LastDir=BACKWARD;
   LastOp=SEARCH;
   CompilePattern();
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
   if(getstring("Search: ",(char*)pattern,sizeof(pattern)-1,&SearchHistory,&patlen,NULL)<1)
      return;
   if(getstring("Replace: ",(char*)replace,sizeof(replace)-1,&SearchHistory,&replen,NULL)<0)
      return;
   LastOp=REPLACE;
   CompilePattern();
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
