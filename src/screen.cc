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
/*____________________________________________________________________________
**
**  File:         screen.c
**  Description:  text visualization for the editor
**____________________________________________________________________________
*/
#include <config.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <xalloca.h>

#include "edit.h"
#include "block.h"
#include "highli.h"

#define   HexPos    10
#define   AsciiPos   (HexPos+3*16+2)

int       ShowScrollBar=SHOW_RIGHT;
int       ShowStatusLine=SHOW_BOTTOM;

int   TextWinX;
int   TextWinY;
int   TextWinWidth;
int   TextWinHeight;
int   ScrollBarX;
int   StatusLineY;

WINDOW *text_w=0;

/* when there_message==1 there is a message on the screen */
int    there_message=0;

void  TestPosition()
{
   if(hex)
   {
      if(ScreenTop&15)
	 ScreenTop=ScreenTop&~15;
      if(Offset()<ScreenTop.Offset())
      {
         flag=REDISPLAY_ALL;
         if(Offset()/16<Scroll)
         {
            ScreenTop=0;
         }
         else
         {
            ScreenTop=(Offset()&~15)-Scroll*16+16;
         }
      }
      if(Offset()>=(ScreenTop.Offset()+16*TextWinHeight))
      {
         flag=REDISPLAY_ALL;
         ScreenTop=(Offset()&~15)-(TextWinHeight-Scroll)*16;
      }
      return;
   }
   if(GetLine()-ScreenTop.Line()>TextWinHeight-1)
   {
      ScreenTop=PrevNLines(Offset(),TextWinHeight-Scroll);
      flag=REDISPLAY_ALL;
   }
   else if(GetLine()-ScreenTop.Line()<0)
   {
      ScreenTop=PrevNLines(Offset(),Scroll-1);
      flag=REDISPLAY_ALL;
   }
   num   col=(Text?max(stdcol,GetCol()):GetCol());
   if(ScrShift>col)
   {
      ScrShift=(col/hscroll)*hscroll;
      flag=REDISPLAY_ALL;
   }
   else if(col-ScrShift>=TextWinWidth)
   {
      ScrShift=((col-TextWinWidth)/hscroll+1)*hscroll;
      flag=REDISPLAY_ALL;
   }
}

void  SyncTextWin()
{
   TestPosition();
   if(flag&REDISPLAY_ALL)
     RedisplayAll();
   else if(flag&REDISPLAY_AFTER)
     RedisplayAfter();
   else if(flag&REDISPLAY_LINE)
     RedisplayLine();
}

void  LocateCursor()
{
   StatusLine();
   SetCursor();
}
void  ScrollBar(int check)
{
   static  int OldPos;
   int       NewPos;
   int       i;

   NewPos=(Size()==0)?0:((Offset()*(TextWinHeight-1)+Size()/2)/Size());


   if(ShowScrollBar==SHOW_NONE)
   {
      OldPos=NewPos;
      return;
   }

   attrset(SCROLL_BAR_ATTR->attr);

   if(check && NewPos==OldPos)
   {
      if(!there_message && ShowStatusLine!=SHOW_BOTTOM)
      {
         mvaddch(TextWinY+TextWinHeight-1,ScrollBarX,
            (NewPos==(TextWinHeight-1)?' '|A_REVERSE:ACS_CKBOARD));
      }
      return;
   }

   for(i=TextWinY; i<TextWinY+TextWinHeight; i++)
      mvaddch(i,ScrollBarX,ACS_CKBOARD);
   OldPos=NewPos;
   mvaddch(NewPos+TextWinY,ScrollBarX,' '|A_REVERSE);
}
void  SetCursor()
{
   SyncTextWin();
   ScrollBar(TRUE);
   if(hex)
   {
      move((Offset()-ScreenTop.Offset())/16+TextWinY,
          ((ascii?AsciiPos+(Offset()-ScreenTop.Offset())%16
               :HexPos+(Offset()-ScreenTop.Offset())%16*3+right)
           +TextWinX));
   }
   else
   {
      move(GetLine()-ScrLine+TextWinY,
          ((Text&&Eol())?stdcol:GetCol())-ScrShift+TextWinX);
   }
   curs_set(1);
   curs_set(insert?1:2);
}

void  Message(const char *s)
{
   attrset(STATUS_LINE_ATTR->attr);
   mvaddstr(LINES-1,0,s);
   for(int x=strlen(s); x<COLS; x++)
      addch(' ');
   refresh();
   there_message=1;
}
void  ClearMessage()
{
   if(there_message && ShowStatusLine!=SHOW_BOTTOM)
   {
      Redisplay(TextWinHeight-1,
              hex?ScreenTop.Offset()+16*(TextWinHeight-1)
                :NextNLines(ScreenTop.Offset(),TextWinHeight-1),
              TextWinHeight);
      there_message=0;
   }
}

void  StatusLine()
{
   char  status[512];
   char  *bn;
   char  name[20];
   char  chr[4];
   int   l;
   char  flags[8];

   ClearMessage();

   if(ShowStatusLine==SHOW_NONE)
     return;

   if(Eof())
     strcpy(chr,"EOF");
   else
   {
      if(Eol())
	 strcpy(chr,"EOL");
      else
	 sprintf(chr,"%-3d",Char());
   }

   if(View)
     sprintf(flags,"R/O %c %c",
       rblock    ?'B':' ',
       DosEol    ?'D':' ');
   else
     sprintf(flags,"%c%c%c%c%c%c%c",
       modified   ?'*':' ',
       inputmode   ?(inputmode==2?'G':'R'):' ',
       insert    ?'I':'O',
       autoindent  ?'A':' ',
       rblock    ?'B':' ',
       (oldptr1>ptr1 || oldptr2<ptr2) ? 'U'
	 : ( (oldptr1<ptr1 || oldptr2>ptr2) ? 'u':' '),
       DosEol    ?'D':' ');

   if(FileName[0])
   {
     bn=le_basename(FileName);
     l=strlen(bn);
     if(l>14)
       sprintf(name,"\"%.*s..%.*s\"",6,bn,6,bn+l-6);
     else
       sprintf(name,"\"%s\"",bn);
   }
   else
     sprintf(name,"NewFile");

   sprintf(status,
     "Line=%-5lu Col=%-4lu Size:%-6lu Ch:%3s %s %s Offs:%lu (%d%%)",
         GetLine()+1,((Text&&Eol())?stdcol:GetCol())+1,Size(),chr,flags,name,Offset(),
         (int)(Size()?(Offset()*100+Size()/2)/Size():100));

   l=strlen(status);
   if(l<COLS)
     memset(status+l,' ',COLS-l);
   status[COLS]=0;

   SetAttr(STATUS_LINE_ATTR);
   move(StatusLineY,0);
   for(bn=status; *bn; bn++)
      addch_visual((byte)*bn);
}

void  Redisplay(num line,offs ptr,num limit)
{
   extern  ShowMatchPos;
   num    col;
   int    i;
   char  s[64],*sp;
   offs  lptr;
   int    x;

   struct attr *norm_attr=NORMAL_TEXT_ATTR,*blk_attr=BLOCK_TEXT_ATTR;

   static  num   OldBlockBeginLine=-1,OldBlockBeginCol=-1,
                 OldBlockEndLine=-1,OldBlockEndCol=-1;

   if(!hex)
      ScreenTop=LineBegin(ScreenTop);
   else
      ScreenTop=ScreenTop&~15;

   if(line<0)
   {
      ptr=ScreenTop.Offset();
      line=0;
   }
   if(limit>TextWinHeight)
      limit=TextWinHeight;

   TestPosition();

   if(rblock && !hide)
   {
      /* This is a dirty trick, but it works */
      /* The need for it is that editing of single line can change
	    block marking on several lines, when the block is a rectangle */
      if(BlockBegin.Col()!=OldBlockBeginCol
      || BlockBegin.Line()!=OldBlockBeginLine)
      {
         OldBlockBeginLine=BlockBegin.Line();
         OldBlockBeginCol =BlockBegin.Col();
         flag=REDISPLAY_ALL;
      }
      if(BlockEnd.Col() !=OldBlockEndCol
      || BlockEnd.Line()!=OldBlockEndLine)
      {
         OldBlockEndLine=BlockEnd.Line();
         OldBlockEndCol =BlockEnd.Col();
         flag=REDISPLAY_ALL;
      }
   }

   if(flag&REDISPLAY_ALL)
   {
      ScrollBar(FALSE);	/* redraw all the scrollbar */
      line=0;
      ptr=ScreenTop.Offset();
      limit=TextWinHeight;
      flag=0;
   }

   chtype *cl=(chtype*)alloca(TextWinWidth*sizeof(chtype));
   chtype *clp=cl;
   struct attr *ca=norm_attr;

   if(hex)
   {
      /* here goes drawing the text in HEX mode */
      rblock=0;
      for( ; line<limit; line++)
      {
         lptr=ptr;

         clp=cl;

	 if(!EofAt(ptr) || line==(Size()-ScreenTop.Offset()+15)/16)
         {
            sprintf(s,"%07lX   ",ptr);
            for(sp=s; *sp; sp++)
	       *clp++=norm_attr->attr|*sp;
         }
         for(i=0; i<16 && !EofAt(ptr); i++)
         {
            if(EofAt(ptr))
               break;
            sprintf(s,"%02X",CharAt(ptr));
            ca=(InBlock(ptr)?blk_attr:norm_attr);
            if(ascii && ptr==Offset() && ShowMatchPos)
               ca=SHADOW_ATTR;
	    *clp++=ca->attr|s[0];
	    *clp++=ca->attr|s[1];
            ptr++;
            if(InBlock(ptr-1) && InBlock(ptr) && (ptr&15))
               *clp++=blk_attr->attr|' ';
            else
               *clp++=norm_attr->attr|' ';
         }
	 *clp++=norm_attr->attr|' ';
	 *clp++=norm_attr->attr|' ';
         ptr=lptr;
         for(i=0; i<16 && !EofAt(ptr); i++,ptr++)
         {
            ca=(InBlock(ptr)?blk_attr:norm_attr);
            if(!ascii && ptr==Offset() && ShowMatchPos)
               ca=SHADOW_ATTR;
	    *clp++=visualize(ca,CharAt_NoCheck(ptr)|ca->attr);
         }
	 // clear the rest of line
	 for(i=TextWinWidth-(clp-cl); i>0; i--)
            *clp++=norm_attr->attr|' ';

	 wmove(text_w,line,0);
      	 clp=cl;
	 attrset(0);
	 for(x=0; x<TextWinWidth; x++)
	    waddch(text_w,*clp++);
      }
      touchwin(stdscr);
      wnoutrefresh(text_w);
   }
   else
   {
      static byte *hl=0;
      offs  next_line_ptr;
      for(; line<limit; line++,ptr=next_line_ptr)
      {
	 /* TODO: build highlight map here */
	 next_line_ptr=NextLine(ptr);
	 int ll=next_line_ptr-ptr;
	 hl=(byte*)realloc(hl,ll);
	 if(!hl)
	    goto after_hl;
	 memset(hl,'\0',ll);
	 if(curr_highlight)
	 {
	    char *buf1="",*buf2="";
	    int	  len1=0,len2=0;

	    if(ptr1<=ptr)
	    {
	       buf1=buffer+ptr2+ptr-ptr1;
	       len1=ll;
	    }
	    else if(ptr1>=ptr+ll)
	    {
	       buf1=buffer+ptr;
	       len1=ll;
	    }
	    else
	    {
	       buf1=buffer+ptr;
	       len1=ptr1-ptr;
	       buf2=buffer+ptr2;
	       len2=ll-len1;
	    }

	    int pos=0;
	    for(;;)
	    {
	       pos=re_search_2(&hl_compiled,buf1,len1,buf2,len2,pos,ll-pos,
			       &hl_regs,ll-pos);
	       if(pos==-1)
		  break;
	       for(unsigned r=1; r<hl_regs.num_regs; r++)
	       {
		  for(i=hl_regs.start[r]; i<hl_regs.end[r]; i++)
		     hl[i]=r;
	       }
	       pos++;
	    }
	 }

      after_hl:
	 clp=cl;
	 byte *hlp=hl;

         for(col=(-ScrShift);
            col<TextWinWidth && !EolAt(ptr); ptr++,hlp++)
         {
	    if(InBlock(ptr,line+ScrLine,col+ScrShift))
	       ca=blk_attr;
	    else if(hlp && *hlp)
	    {
	       ca=blk_attr;
	    }
	    else
	       ca=norm_attr;

	    byte ch=CharAt_NoCheck(ptr);
            if(ch=='\t')
            {
               i=col;
               col=Tabulate(col+ScrShift)-ScrShift;
               if(i<0)
                  i=0;
               while(i<col && i<TextWinWidth)
               {
		  ca=(InBlock(ptr,line+ScrLine,col+ScrShift)?blk_attr:norm_attr);
                  *clp++=ca->attr|' ';
                  i++;
               }
            }
            else
            {
               if(col>=0)
		  *clp++=visualize(ca,ch|ca->attr);
	       col++;
            }
         }
	 if(col<0)
	    col=0;
         for( ; col<TextWinWidth; col++)
         {
	    ca=(InBlock(ptr,line+ScrLine,col+ScrShift)?blk_attr:norm_attr);
            *clp++=ca->attr|' ';
         }

	 wmove(text_w,line,0);
      	 clp=cl;
	 attrset(0);
	 for(x=0; x<TextWinWidth; x++)
	    waddch(text_w,*clp++);
      }
   }
   touchwin(stdscr);
   wnoutrefresh(text_w);
}
void  RedisplayAll()
{
   ScrollBar(FALSE);
   flag=0;
   Redisplay(0,ScreenTop.Offset(),TextWinHeight);
}
void  RedisplayAfter()
{
   flag&=~REDISPLAY_AFTER;
   if(hex)
      Redisplay((Offset()-ScreenTop.Offset())/16-1,(Offset()&~15)-16,TextWinHeight);
   else
      Redisplay(GetLine()-ScreenTop.Line()-1,PrevLine(Offset()),TextWinHeight);
}
void  RedisplayLine()
{
   flag&=~REDISPLAY_LINE;
   if(hex)
      Redisplay((Offset()-ScreenTop.Offset())/16-1,(Offset()&~15)-16,(Offset()-ScrPtr)/16+2);
   else
      Redisplay(GetLine()-ScreenTop.Line()-1,PrevLine(Offset()),GetLine()-ScrLine+1);
}

void  CenterView()
{
   TestPosition();
   if(hex)
   {
      if((Offset()-ScreenTop)/16 > TextWinHeight*2/3
      || (Offset()-ScreenTop)/16 < TextWinHeight/3)
      {
	 ScreenTop=(Offset()-TextWinHeight*16/2)&~15;
	 flag=REDISPLAY_ALL;
      }
      offs max_top=(TextEnd-TextWinHeight*16+15)&~15;
      if(ScreenTop>max_top)
	 ScreenTop=max_top;
      return;
   }
   if(GetLine()-ScreenTop.Line()>TextWinHeight*2/3)
   {
      ScreenTop=NextNLines(ScreenTop.Offset(),GetLine()-ScreenTop.Line()-TextWinHeight/2);
      flag=REDISPLAY_ALL;
   }
   else if(GetLine()-ScreenTop.Line()<TextWinHeight/3)
   {
      ScreenTop=PrevNLines(ScreenTop.Offset(),-(GetLine()-ScreenTop.Line()-TextWinHeight/2));
      flag=REDISPLAY_ALL;
   }
   offs max_top=PrevNLines(TextEnd,TextWinHeight-1);
   if(max_top<ScreenTop)
      ScreenTop=max_top;
}

struct  menu   OkMenu[]={
{"   &Ok   ",MIDDLE,FDOWN-2},
{NULL}};

void  ErrMsg(const char *s)
{
   ReadMenuBox(OkMenu,HORIZ,s," Error ",ERROR_WIN_ATTR,CURR_BUTTON_ATTR);
}

void  FError(char *s)
{
   char  msg[256];

   if(strlen(s)>50)
      sprintf(msg,"File: ...%s\n",s+strlen(s)-47);
   else
      sprintf(msg,"File: %s\n",s);
   if(errno>0)
   {
      if(errno==EAGAIN)
         strcpy(msg,"File is already locked or no more processes.");
      else
         strcat(msg,strerror(errno));
   }
   else
      strcat(msg,"The device is full or ulimit is too low,\nI cannot write");
   ErrMsg(msg);
}

void  NotMemory()
{
   ErrMsg("Not enough memory");
}
