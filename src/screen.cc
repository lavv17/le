/*
 * Copyright (c) 1993-2017 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "edit.h"
#include "block.h"
#include "highli.h"
#include "getch.h"
#include "mb.h"

#ifndef max
# define max(a,b) ((a)>(b)?(a):(b))
#endif

int       ShowScrollBar=SHOW_NONE;
int       ShowStatusLine=SHOW_BOTTOM;

int   ShowMatchPos=TRUE;

int   TextWinX;
int   TextWinY;
int   TextWinWidth;
int   TextWinHeight;
int   ScrollBarX;
int   StatusLineY;

int    message_sp=0; // number of messages on bottom of screen

int   range_begin;
int   range_end;

#if defined(__NCURSES_H) && (!defined(mvaddchnstr) || defined(mvadd_wchnstr))
#undef mvaddchnstr
#define mvaddchnstr le_mvaddchnstr

int le_mvaddchnstr(int line, int col, chtype *s, int n)
{
   if(move(line,col)==ERR)
      return ERR;
   while(n-->0)
      addch(*s++);
   return OK;
}
#endif

void  TestPosition()
{
   int m=message_sp;
   if(ShowStatusLine==SHOW_BOTTOM && m>0)
      m--;  // one message is over status.

   if(in_hex_mode)
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
      if(Offset()>=(ScreenTop.Offset()+16*(TextWinHeight-m)))
      {
         flag=REDISPLAY_ALL;
         ScreenTop=(Offset()&~15)-(TextWinHeight-m-Scroll)*16;
      }
      return;
   }
   num oldtop=ScreenTop.Line();
   if(GetLine()-oldtop>TextWinHeight-m-1)
   {
      ScreenTop=PrevNLines(Offset(),TextWinHeight-m-Scroll);
      flag=REDISPLAY_ALL;
   }
   else if(GetLine()-ScreenTop.Line()<0)
   {
      ScreenTop=PrevNLines(Offset(),Scroll-1);
      flag=REDISPLAY_ALL;
   }
   num   col=GetCol();
   if(Text && stdcol!=NO_POS)
      col=stdcol;
   if(ScrShift+hscroll>col && ScrShift>0)
   {
      ScrShift=(col/hscroll-1)*hscroll;
      if(ScrShift<0)
	 ScrShift=0;
      flag=REDISPLAY_ALL;
   }
   else if(col-ScrShift>=TextWinWidth)
   {
      ScrShift=((col-TextWinWidth)/hscroll+1)*hscroll;
      flag=REDISPLAY_ALL;
   }
}

static int skipped=0;	// number of times Sync skipped its work

void  SyncTextWin()
{
   if(CheckPending()>0)
   {
      if(++skipped<2000)
      {
         if(ShowStatusLine==SHOW_NONE)
         {
            flag=REDISPLAY_ALL;
            return;
         }
	 attrset(STATUS_LINE_ATTR->n_attr);
	 move(StatusLineY,COLS-3);
	 addch(' ');
	 addch("-\\|/"[time(0)%4]);
	 leaveok(stdscr,TRUE);
	 flag=REDISPLAY_ALL;
	 return;
      }
   }
   skipped=0;

   int m=message_sp;
   if(ShowStatusLine==SHOW_BOTTOM && m>0)
      m--;  // one message is over status.

   int line=TextWinHeight-m;
   int lim=-1;
   offs ptr;

   static  num   OldBlockBeginLine=-1,OldBlockBeginCol=-1,
                 OldBlockEndLine=-1,OldBlockEndCol=-1;

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
      line=0;
      lim=TextWinHeight-m;
   }
   else if(flag&REDISPLAY_AFTER)
   {
      line=in_hex_mode ? (CurrentPos-ScreenTop)/16-1 : GetLine()-ScreenTop.Line()-1;
      lim=TextWinHeight-m;
   }
   else if(flag&REDISPLAY_LINE)
   {
      line=in_hex_mode ? (CurrentPos-ScreenTop)/16-1 : GetLine()-ScreenTop.Line()-1;
      lim =line+2;
   }
   if(flag&REDISPLAY_RANGE)
   {
      if(line>range_begin)
	 line=range_begin;
      if(lim<range_end)
	 lim=range_end;
   }

   if(in_hex_mode)
      ptr=(ScreenTop&~15)+16*line;
   else
      ptr=NextNLines(ScreenTop,line);

   if(lim>TextWinHeight-m)
      lim=TextWinHeight-m;

   if(lim>=line)
      Redisplay(line,ptr,lim);
   flag=0;
   StatusLine();
}

int   ScrollBarPos=0;
void  ScrollBar(int check)
{
   int       NewPos;
   int       i;

   NewPos=(Size()==0)?0:((Offset()*(TextWinHeight-1)+Size()/2)/Size());

   if(ShowScrollBar==SHOW_NONE)
   {
      ScrollBarPos=NewPos;
      return;
   }

   attrset(SCROLL_BAR_ATTR->n_attr);

   if(check && NewPos==ScrollBarPos)
      return;

   for(i=TextWinY; i<TextWinY+TextWinHeight; i++)
   {
#if USE_MULTIBYTE_CHARS
      if(mb_mode)
	 mvadd_wch(i,ScrollBarX,WACS_CKBOARD);
      else
#endif
	 mvaddch(i,ScrollBarX,ACS_CKBOARD);
   }
   ScrollBarPos=NewPos;
   mvaddch(NewPos+TextWinY,ScrollBarX,' '|A_REVERSE);
}
void  SetCursor()
{
   if(skipped)
      return;

   ScrollBar(TRUE);
   if(in_hex_mode)
   {
      move((Offset()-ScreenTop.Offset())/16+TextWinY,
          ((ascii?AsciiPos+(Offset()-ScreenTop.Offset())%16
               :HexPos+(Offset()-ScreenTop.Offset())%16*3+right)
           +TextWinX));
   }
   else
   {
      move(GetLine()-ScrLine+TextWinY,
          ((Text&&Eol())?GetStdCol():GetCol())-ScrShift+TextWinX);
   }
   leaveok(stdscr,FALSE);
   if(insert)
      curs_set(1);
   else
   {
      if(curs_set(2)==ERR)
	 curs_set(1); /* in case it was invisible */
   }
}

void  AddMessage(const char *s)
{
   message_sp++;
   attrset(STATUS_LINE_ATTR->n_attr);
   mvaddstr(LINES-message_sp,0,(char*)s);
   for(int x=strlen(s); x<COLS; x++)
      addch(' ');
}
void  Message(const char *s)
{
   if(message_sp==1)
      message_sp=0;  // don't need to clear, as we'll add the line again
   else
      ClearMessage();
   AddMessage(s);
}
void  MessageSync(const char *s)
{
   Message(s);
   refresh();
}

static void ClearMessageOnTextWin()
{
   if(message_sp>0 && (ShowStatusLine!=SHOW_BOTTOM || message_sp>1))
   {
      int m=message_sp;
      message_sp=0;
      Redisplay(TextWinHeight-m,
              in_hex_mode?ScreenTop.Offset()+16*(TextWinHeight-m)
                :NextNLines(ScreenTop.Offset(),TextWinHeight-m),
              TextWinHeight);
      ScrollBar(false);
   }
}

void  ClearMessage()
{
   ClearMessageOnTextWin();
   if(message_sp>0 && ShowStatusLine==SHOW_BOTTOM)
      StatusLine();
   message_sp=0;
}

void  StatusLine()
{
   char  status[512];
   char  status_right[512];
   char  *bn;
   char  name[20];
   wchar_t *wname=0;
   char  chr[12];
   int   l;
   char  flags[16];

   ClearMessageOnTextWin();
   message_sp=0;

   if(ShowStatusLine==SHOW_NONE)
      return;

   if(Eof())
      strcpy(chr,"Ch:EOF");
   else
   {
      if(Eol())
	 strcpy(chr,"Ch:EOL");
      else
      {
	 if(MBCheckRight() && MBCharSize>1)
	    snprintf(chr,sizeof(chr),"UC:%04X",WChar());
	 else
	    snprintf(chr,sizeof(chr),"Ch:%-3d",Char());
      }
   }

   char eol=' ';
   if(!strcmp(EolStr,EOL_DOS))
      eol='D';
   else if(!strcmp(EolStr,EOL_MAC))
      eol='M';

   if(View)
   {
      if(buffer_mmapped)
	 snprintf(flags,sizeof(flags),"MM R/O %c",
	    rblock   ?'B':' ');
      else
	 snprintf(flags,sizeof(flags),"R/O %c %c",
	    rblock   ?'B':' ',eol);
   }
   else
   {
      if(buffer_mmapped)
	 snprintf(flags,sizeof(flags),"MM %c%c   ",
	    inputmode   ?(inputmode==2?'G':'R'):' ',
	    rblock	?'B':' ');
      else
	 snprintf(flags,sizeof(flags),"%c%c%c%c%c%c%c",
	    modified	?'*':' ',
	    inputmode   ?(inputmode==2?'G':'R'):' ',
	    insert	?'I':'O',
	    autoindent  ?'A':' ',
	    rblock	?'B':' ',
	    (oldptr1>ptr1 || oldptr2<ptr2) ? 'U'
	       : ( (oldptr1<ptr1 || oldptr2>ptr2) ? 'u':' '),
	    eol);
   }
   if(FileName[0])
   {
      bn=le_basename(FileName);
      l=strlen(bn);
#if USE_MULTIBYTE_CHARS
      wname=(wchar_t*)alloca((l+1)*sizeof(wchar_t));
      memset(wname,0,(l+1)*sizeof(wchar_t));
      mbstowcs(wname,bn,l);
      l=wcslen(wname);
      if(l>14)
      {
	 memmove(wname+8,wname+l-6,7*sizeof(wchar_t));
	 wname[6]=wname[7]=L'.';
      }
      if(l<=0)
	 wname=0;
#endif // USE_MULTIBYTE_CHARS
      if(wname==0)
      {
	 if(l>14)
	    snprintf(name,sizeof(name),"%.*s..%.*s",6,bn,6,bn+l-6);
	 else
	    strcpy(name,bn);
      }
   }
   else
      snprintf(name,sizeof(name),"NewFile");


   if(in_hex_mode)
      snprintf(status,sizeof(status),"OctOffs:0%-11lo",(unsigned long)(Offset()));
   else
      snprintf(status,sizeof(status),"Line=%-5lu Col=%-4lu",
	 (unsigned long)(GetLine()+1),
	 (unsigned long)(((Text&&Eol())?GetStdCol():GetCol())+1));

   unsigned status_len=strlen(status);
   snprintf(status+status_len,sizeof(status)-status_len," Sz:%-6lu %-7s %s",
         (unsigned long)(Size()),chr,flags);

   snprintf(status_right,sizeof(status_right)," Offs:%lu (%d%%)",(unsigned long)(Offset()),
         (int)(Size()?(Offset()*100.+Size()/2)/Size():100));

   move(StatusLineY,0);
   SetAttr(STATUS_LINE_ATTR);
   int prev_x=0;
   int x=0,y;
   for(bn=status; *bn; bn++)
   {
      prev_x=x;
      addch((byte)*bn);
      getyx(stdscr,y,x);
      (void)y;
      if(prev_x>x || (prev_x==x && x==COLS-1))
	 return;
   }

   if(x>=COLS-3)
      return;
   addch('"');
   getyx(stdscr,y,x);
#if USE_MULTIBYTE_CHARS
   if(wname)
   {
      for(wchar_t *w=wname; *w; w++)
      {
	 prev_x=x;
	 wchar_t wc=visualize_wchar(*w);
	 if(wc!=*w)
	    attrset(curr_attr->so_attr);
	 addnwstr(&wc,1);
	 attrset(curr_attr->n_attr);
	 getyx(stdscr,y,x);
	 if(prev_x>x || (prev_x==x && x==COLS-1))
	    return;
      }
   }
   else // note the following block
#endif
   {
      for(bn=name; *bn; bn++)
      {
	 prev_x=x;
	 addch((byte)*bn);
	 getyx(stdscr,y,x);
	 if(prev_x>x || (prev_x==x && x==COLS-1))
	    return;
      }
   }
   prev_x=x;
   addch('"');
   getyx(stdscr,y,x);
   if(prev_x>x || (prev_x==x && x==COLS-1))
      return;
   for(bn=status_right; *bn; bn++)
   {
      prev_x=x;
      addch((byte)*bn);
      getyx(stdscr,y,x);
      if(prev_x>x || (prev_x==x && x==COLS-1))
	 return;
   }
   for(int i=COLS-x; i>0; i--)
      addch(' ');
}

static unsigned mkhash(byte *data,int len)
{
   unsigned res=0;
   while(len-->0)
      res+=(res<<5)+*data++;
   return res;
}

static const attr *norm_attr,*blk_attr,*syntax[SYNTAX_COLORS];
static const attr *FindPosAttr(offs ptr,num line,num col,byte *hlp)
{
   if(col>=TextWinWidth-MBCharWidth && !EolAt(ptr) && !EolAt(ptr+MBCharSize))
      return SHADOW_ATTR;
   else if(InBlock(ptr,line+ScrLine,col+ScrShift))
      return blk_attr;
   else if(hlp && *hlp>0 && *hlp<=SYNTAX_COLORS)
      return syntax[*hlp-1];
   else
      return norm_attr;
}

#ifdef USE_MULTIBYTE_CHARS
static inline void le_add_cchar(cchar_t *&c,attr_t a,wchar_t wc)
{
# ifdef CURSES_CCHAR_MAX // NetBSD
   c->attributes=a;
   c->elements=1;
   c->vals[0]=wc;
# else // ncurses
   c->attr=a;
   c->chars[0]=wc;
//    for(int i=1; i<CCHARW_MAX; i++)
//       c->chars[i]=0;
# endif
   c++;
}
static inline void le_combine_cchar(cchar_t *c,wchar_t wc)
{
# ifdef CURSES_CCHAR_MAX // NetBSD
   if(c->elements>=CURSES_CCHAR_MAX)
      return;
   c->vals[c->elements++]=wc;
# else // ncurses
   int i=1;
   while(i<CCHARW_MAX && c->chars[i])
      i++;
   if(i>=CCHARW_MAX)
      return;
   c->chars[i]=wc;
# endif
}
#endif // USE_MULTIBYTE_CHARS

void  Redisplay(num line,offs ptr,num limit)
{
   num    col;
   int    i;
   char  s[64],*sp;
   offs  lptr;

   int m=message_sp;
   if(ShowStatusLine==SHOW_BOTTOM && m>0)
      m--;  // one message is over status.

   norm_attr=NORMAL_TEXT_ATTR;
   blk_attr=BLOCK_TEXT_ATTR;
   for(i=0; i<SYNTAX_COLORS; i++)
      syntax[i]=find_attr(SYNTAX1+i);

   if(!in_hex_mode)
   {
      if(!BolAt(ScreenTop))
	 ScreenTop=TextPoint(ScreenTop.Line(),0);
   }
   else
   {
      if(ScreenTop&15)
	 ScreenTop=ScreenTop&~15;
   }

   if(line<0)
   {
      ptr=ScreenTop.Offset();
      line=0;
   }
   if(limit>TextWinHeight-m)
      limit=TextWinHeight-m;

   if(flag&REDISPLAY_ALL)
   {
      ScrollBar(FALSE);	/* redraw all the scrollbar */
      line=0;
      ptr=ScreenTop.Offset();
      limit=TextWinHeight-m;
   }

   if(limit<=line)
      return;
   if(ptr<0)
      ptr=0;

   int ll=max(TextWinWidth,80);
   chtype *cl=(chtype*)alloca(ll*sizeof(chtype));
   chtype *clp;
   const attr *ca=norm_attr;

#ifdef USE_MULTIBYTE_CHARS
   cchar_t *clw=(cchar_t*)alloca(ll*sizeof(cchar_t));
   cchar_t *clwp;
#endif

   if(in_hex_mode)
   {
      /* here goes drawing the text in HEX mode */
      rblock=0;
      for( ; line<limit; line++)
      {
         lptr=ptr;

	 if(!mb_mode)
	 {
	    clp=cl;

	    if(!EofAt(ptr) || line==(Size()-ScreenTop.Offset()+15)/16)
	    {
	       snprintf(s,sizeof(s),"%08lX   ",(unsigned long)ptr);
	       for(sp=s; *sp; sp++)
		  *clp++=norm_attr->n_attr|*sp;
	    }
	    for(i=0; i<16 && !EofAt(ptr); i++)
	    {
	       if(EofAt(ptr))
		  break;
	       snprintf(s,sizeof(s),"%02X",CharAt(ptr));
	       ca=(InBlock(ptr)?blk_attr:norm_attr);
	       if(ascii && ptr==Offset() && ShowMatchPos)
		  ca=SHADOW_ATTR;
	       *clp++=ca->n_attr|s[0];
	       *clp++=ca->n_attr|s[1];
	       ptr++;
	       char b=' '; // ((ptr&15)!=8 ? ' ' : '-');
	       if(InBlock(ptr-1) && InBlock(ptr) && (ptr&15))
		  *clp++=blk_attr->n_attr|b;
	       else
		  *clp++=norm_attr->n_attr|b;
	    }
	    while(clp-cl<AsciiPos)
	       *clp++=norm_attr->n_attr|' ';
	    ptr=lptr;
	    for(i=0; i<16 && !EofAt(ptr); i++,ptr++)
	    {
	       ca=(InBlock(ptr)?blk_attr:norm_attr);
	       if(!ascii && ptr==Offset() && ShowMatchPos)
		  ca=SHADOW_ATTR;
	       *clp++=visualize(ca,CharAt_NoCheck(ptr)|ca->n_attr);
	    }
	    // clear the rest of line
	    for(i=TextWinWidth-(clp-cl); i>0; i--)
	       *clp++=norm_attr->n_attr|' ';

	    attrset(0);
	    mvaddchnstr(TextWinY+line,TextWinX,cl,TextWinWidth);
	 }
#ifdef USE_MULTIBYTE_CHARS
	 else // mb_mode
	 {
	    memset(clw,0,ll*sizeof(cchar_t));
	    clwp=clw;
	    if(!EofAt(ptr) || line==(Size()-ScreenTop.Offset()+15)/16)
	    {
	       snprintf(s,sizeof(s),"%08lX   ",(unsigned long)ptr);
	       for(sp=s; *sp; sp++)
		  le_add_cchar(clwp,norm_attr->n_attr,*sp);
	    }
	    for(i=0; i<16 && !EofAt(ptr); i++)
	    {
	       if(EofAt(ptr))
		  break;
	       snprintf(s,sizeof(s),"%02X",CharAt(ptr));
	       ca=(InBlock(ptr)?blk_attr:norm_attr);
	       if(ascii && ptr==Offset() && ShowMatchPos)
		  ca=SHADOW_ATTR;
	       le_add_cchar(clwp,ca->n_attr,s[0]);
	       le_add_cchar(clwp,ca->n_attr,s[1]);
	       ptr++;
	       char b=' '; // ((ptr&15)!=8 ? ' ' : '-');
	       ca=((InBlock(ptr-1) && InBlock(ptr) && (ptr&15))?blk_attr:norm_attr);
	       le_add_cchar(clwp,ca->n_attr,b);
	    }
	    while(clwp-clw<AsciiPos)
	       le_add_cchar(clwp,norm_attr->n_attr,' ');
	    ptr=lptr;
	    for(i=0; i<16 && !EofAt(ptr); i++,ptr++)
	    {
	       ca=(InBlock(ptr)?blk_attr:norm_attr);
	       if(!ascii && ptr==Offset() && ShowMatchPos)
		  ca=SHADOW_ATTR;
	       wchar_t ch=CharAt_NoCheck(ptr);
	       wchar_t vch=visualize_wchar(ch);
	       le_add_cchar(clwp,vch==ch?ca->n_attr:ca->so_attr,vch);
	    }
	    // clear the rest of line
	    for(i=TextWinWidth-(clwp-clw); i>0; i--)
	       le_add_cchar(clwp,norm_attr->n_attr,' ');
	    attrset(0);
	    mvadd_wchnstr(TextWinY+line,TextWinX,clw,TextWinWidth);
	 }
#endif // USE_MULTIBYTE_CHARS
      }
   }
   else /* !in_hex_mode */
   {
      offs  next_line_ptr;

      offs  start=0,end=0;
      byte *hl=0;
      byte *hlp=0;

      /* build highlight map */
      if(hl_option && hl_active)
      {
	 start=PrevNLines(ScreenTop,hl_lines-1);
	 end=NextNLines(ScreenTop,TextWinHeight-m+hl_lines);
	 int ll=end-start;
	 if(ll==0)
	    goto after_hl;
	 hl=(byte*)malloc(ll);
	 if(!hl)
	    goto after_hl;

	 char *buf1=0,*buf2=0;
	 int   len1=0,len2=0;

	 if(ptr1<=start)
	 {
	    buf1=buffer+ptr2+start-ptr1;
	    len1=ll;
	 }
	 else if(ptr1>=end)
	 {
	    buf1=buffer+start;
	    len1=ll;
	 }
	 else
	 {
	    buf1=buffer+start;
	    len1=ptr1-start;
	    buf2=buffer+ptr2;
	    len2=ll-len1;
	 }

	 syntax_hl::attrib_line(buf1,len1,buf2,len2,hl);

	 // try to find difference in highlight
	 static unsigned oldhash[1024];
	 unsigned newhash;

	 offs p=ScreenTop;
	 hlp=hl+(p-start);
	 int l;
	 for(l=0; l<TextWinHeight-m && l<1024; l++)
	 {
	    next_line_ptr=NextLine(p);
	    newhash=mkhash(hlp,next_line_ptr-p);

	    if(p<ptr && newhash!=oldhash[l])
	    {
	       ptr=p;
	       line=l;
	    }
	    else if(l>=limit && newhash!=oldhash[l])
	    {
	       limit=l+1;
	    }

	    oldhash[l]=newhash;
	    hlp+=next_line_ptr-p;
	    p=next_line_ptr;
	 }

	 hlp=hl+(ptr-start);
      }
   after_hl:

      for(;;)
      {
	 col=ScrShift-TabSize+1;
	 if(col<0)
	    col=0;

	 TextPoint n(ScreenTop.Line()+line,col);
	 next_line_ptr=n.Offset();

	 if(hlp)
	    hlp+=next_line_ptr-ptr;

	 ptr=next_line_ptr;

	 if(n.Line()!=ScreenTop.Line()+line)
	    col=0;
	 else
	    col=n.Col()-ScrShift;

	 if(!mb_mode)
	 {
	    clp=cl;
	    for( ; col<TextWinWidth && !EolAt(ptr); ptr++)
	    {
	       if(col>-TabSize)
		  ca=FindPosAttr(ptr,line,col,hlp);

	       byte ch=CharAt_NoCheck(ptr);
	       if(ch=='\t')
	       {
		  i=Tabulate(col+ScrShift)-ScrShift;
		  if(i>0)
		  {
		     if(col<0)
			col=0;
		     *clp++ = ca->n_attr|' ';
		     col++;
		     while(col<i && col<TextWinWidth)
		     {
			ca=FindPosAttr(ptr,line,col,hlp);
			*clp++ = ca->n_attr|' ';
			col++;
		     }
		  }
		  else
		     col=i;
	       }
	       else
	       {
		  if(col>=0)
		     *clp++=visualize(ca,ch|ca->n_attr);
		  col++;
	       }
	       if(hlp)
		  hlp++;
	    }
	    if(col<0)
	       col=0;

	    if(EofAt(ptr))
	       hlp=0;

	    for( ; col<TextWinWidth; col++)
	    {
	       ca=FindPosAttr(ptr,line,col,hlp);
	       *clp++ = ca->n_attr|' ';
	    }

	    attrset(0);
	    mvaddchnstr(TextWinY+line,TextWinX,cl,TextWinWidth);
	 }
#ifdef USE_MULTIBYTE_CHARS
	 else // mb_mode
	 {
	    memset(clw,0,ll*sizeof(cchar_t));
	    clwp=clw;
	    for( ; col<TextWinWidth && !EolAt(ptr); ptr+=MBCharSize)
	    {
	       wchar_t ch=WCharAt(ptr);

	       if(col>-TabSize)
		  ca=FindPosAttr(ptr,line,col,hlp);

	       if(ch=='\t')
	       {
		  i=Tabulate(col+ScrShift)-ScrShift;
		  if(i>0)
		  {
		     if(col<0)
			col=0;
		     le_add_cchar(clwp,ca->n_attr,' ');
		     col++;
		     while(col<i && col<TextWinWidth)
		     {
			ca=FindPosAttr(ptr,line,col,hlp);
			le_add_cchar(clwp,ca->n_attr,' ');
			col++;
		     }
		  }
		  else
		     col=i;
	       }
	       else
	       {
		  if(col>=0)
		  {
		     if(MBCharWidth==0 && clwp>clw)
			le_combine_cchar(clwp-1,ch);
		     else if(MBCharWidth>0)
		     {
			wchar_t vch=visualize_wchar(ch);
			le_add_cchar(clwp,vch==ch?ca->n_attr:ca->so_attr,vch);
		     }
		  }
		  col+=MBCharWidth;
	       }
	       if(hlp)
		  hlp+=MBCharSize;
	    }
	    if(col<0)
	       col=0;

	    if(EofAt(ptr))
	       hlp=0;

	    for( ; col<TextWinWidth; col++)
	    {
	       ca=FindPosAttr(ptr,line,col,hlp);
	       le_add_cchar(clwp,ca->n_attr,' ');
	    }

	    attrset(0);
	    mvadd_wchnstr(TextWinY+line,TextWinX,clw,TextWinWidth);
	 } // end of mb_mode
#endif

	 if(++line>=limit)
	    break;

      } // end for(;;)

      if(hl)
	 free(hl);
   }
}

void  RedisplayAll()
{
   flag=REDISPLAY_ALL;
   SyncTextWin();
}
void  RedisplayLine()
{
   flag|=REDISPLAY_LINE;
   SyncTextWin();
}

void  CenterView()
{
   TestPosition();
   if(in_hex_mode)
   {
      if((Offset()-ScreenTop)/16 > TextWinHeight*2/3
      || (Offset()-ScreenTop)/16 < TextWinHeight/3)
      {
	 offs o=Offset()-TextWinHeight*16/2;
	 if(o<0)
	    o=0;
	 ScreenTop=o&~15;
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
   {
      ScreenTop=max_top;
      flag=REDISPLAY_ALL;
   }
}

struct  menu   OkMenu[]={
{"   &Ok   ",MIDDLE,FDOWN-2},
{NULL}};

void  ErrMsg(const char *s)
{
   ReadMenuBox(OkMenu,HORIZ,s," Error ",ERROR_WIN_ATTR,CURR_BUTTON_ATTR);
}

void  FError(const char *s)
{
   const char *err=errno?strerror(errno):"The device is full or ulimit is too low,\nI cannot write";
   char  msg[256];

   if(strlen(s)>50)
      snprintf(msg,sizeof(msg),"File: ...%s\n",s+strlen(s)-47);
   else
      snprintf(msg,sizeof(msg),"File: %s\n",s);
   strcat(msg,err);

   ErrMsg(msg);
}

void  NoMemory()
{
   ErrMsg("Not enough memory");
}
