/*
 * Copyright (c) 1993-2013 by Alexander V. Lukyanov (lav@yars.free.net)
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

/* block.cc : block operations */

#include <config.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "edit.h"
#include "block.h"
#include "keymap.h"
#include "clipbrd.h"

TextPoint *DragMark=0;

char    BlockFile[256];

int     hide=TRUE;

int rInBlock(num line,num col)
{
   if(hide)
      return(FALSE);
   if(line>=BlockBegin.Line()
   && line<=BlockEnd.Line())
   {
#if 0
      if(BlockBegin.Col()>BlockEnd.Col())
      {
	 TextPoint tp=CurrentPos;
	 HardMove(BlockEnd.Line(),BlockBegin.Col());
	 BlockEnd=CurrentPos;
	 CurrentPos=tp;
      }
#endif
      if(BlockBegin.Col()==BlockEnd.Col())
	 return(col>=BlockBegin.Col());
      return(col>=BlockBegin.Col() && col<BlockEnd.Col());
   }
   return(FALSE);
}

void   MoveLineCol(num l,num c)
{
   CurrentPos=TextPoint(l,c);
}
void   HideDisplay()
{
   if(DragMark)
      UserStopDragMark();
   hide=!hide;
   CheckBlock();
   flag=1;
}
char   CharAtLC(num l,num c)
{
   static  TextPoint   Last;
   Last=TextPoint(l,c);
   return((EolAt(Last)||Last.Col()!=c||Last.Line()!=l)?' ':CharAt(Last));
}
#if USE_MULTIBYTE_CHARS
wchar_t WCharAtLC(num l,num c)
{
   static TextPoint Last;
   Last=TextPoint(l,c);
   return((EolAt(Last)||Last.Col()!=c||Last.Line()!=l)?' ':WCharAt(Last));
}
#endif
void   NewLine()
{
   InsertBlock(EolStr,EolSize);
}

/* Moves exactly to specified line/column, edits the text as needed */
void   HardMove(num l,num c)
{
   MoveLineCol(l,c);
   if(Eof())
      while(GetLine()<l)
        NewLine();
   if(Eol())
      while(GetCol()<c)
         InsertChar(' ');
   if(GetCol()>c)
   {
      MoveLeft();
      ExpandTab();
      while(GetCol()<c)
         MoveRight();
   }
   SetStdCol();
}

void   ExpandTab()
{
   int i,size;
   if(Char()!='\t')
      return;
   size=TabSize-GetCol()%TabSize;
   for(i=size; i>0; i--)
      InsertChar(' ');
   DeleteChar();
   CurrentPos-=size;
}

void    RCopy()
{
   MessageSync("Copying...");

   ClipBoard cb;
   TextPoint tp=CurrentPos;
   num old_stdcol=SaveStdCol();

   if(!cb.Copy())
      return;

   CurrentPos=tp;
   RestoreStdCol(old_stdcol);
   cb.PasteAndMark();
   CurrentPos=BlockBegin;
   SetStdCol();
}

void    Copy()
{
   if(DragMark)
      UserStopDragMark();

   CheckBlock();
   if(View || hide)
      return;

   PreUserEdit();

   if(buffer_mmapped || (hex && !insert))
   {
      CopyBlockOver(BlockBegin,BlockEnd-BlockBegin);
      return;
   }

   if(rblock)
   {
      RCopy();
      return;
   }

   if(CopyBlock(BlockBegin,BlockEnd-BlockBegin)!=OK)
      return;

   flag=REDISPLAY_ALL;
   if(!InBlock(Offset(),GetLine(),GetCol()))
   {
      num size=BlockEnd-BlockBegin;
      BlockBegin=CurrentPos;
      BlockBegin-=size;
      BlockEnd=CurrentPos;
   }
}

/* when lines_deleted!=0 then last RDelete deleted block lines */
static int lines_deleted;

int   RDelete()
{
   MessageSync("Deleting...");

   num   i,j;
   num   h=BlockEnd.Line()-BlockBegin.Line()+1;
   num   oldcol2=BlockEnd.Col();

   lines_deleted=0;

   CurrentPos=BlockBegin;
   num oldline=GetLine(),oldcol=GetCol();

   if(!MainClipBoard.Copy())
      return 0;

   for(i=BlockBegin.Line(); i<=BlockEnd.Line(); i++)
   {
      j=BlockBegin.Col();
      HardMove(i,j);
      if(BlockBegin.Col()==oldcol2)
      {
	 DeleteToEOL();
      }
      else
      {
	 offs o=CurrentPos;
	 while(GetCol()<oldcol2 && !Eol())
	 {
	    if(Char()=='\t')
	       ExpandTab();
	    MoveRightOverEOL();
	 }
	 DeleteBlock(CurrentPos-o,0);
      }
   }

   if(BlockBegin.Col()==oldcol2)
   {
      bool space=true;
      GoToLineNum(BlockBegin.Line());
      while(GetLine()<=BlockEnd.Line() && !Eof())
      {
	 if(Eol())
	    MoveRightOverEOL();
	 else if(Char()==' ' || Char()=='\t')
	    MoveRight();
	 else
	 {
	    space=false;
	    break;
	 }
      }
      if(space)
      {
	 lines_deleted=1;
	 CurrentPos=BlockBegin;
	 for(i=0; i<h; i++)
	    DeleteLine();
      }
   }
   HardMove(oldline,oldcol);
   SetStdCol();

   hide=TRUE;
   return 1;
}
int   Delete()
{
   if(DragMark)
      UserStopDragMark();

   CheckBlock();
   if(View || hide)
      return 1;
   if(buffer_mmapped)
      return 0;
   flag=REDISPLAY_ALL;
   if(rblock)
      return RDelete();
   if(!MainClipBoard.Copy())
      return 0;
   if(!InBlock(Offset(),GetLine(),GetCol()))
      CurrentPos=BlockBegin;
   DeleteBlock(CurrentPos-BlockBegin,BlockEnd-CurrentPos);
   SetStdCol();
   hide=TRUE;
   return 1;
}
void    RMove()
{
   num    oldcol,oldline;
   num    oldline2=BlockEnd.Line();
   num    h=BlockEnd.Line()-BlockBegin.Line()+1;
            /* height of the block */

   if(GetCol()==BlockBegin.Col() && GetLine()==BlockBegin.Line())
      return;
   if(BlockBegin.Col()==BlockEnd.Col()
   && GetLine()>=BlockBegin.Line() && GetLine()<=BlockEnd.Line())
   {
      if(GetCol()==BlockBegin.Col())
         return;
      MoveLineCol(BlockBegin.Line(),GetCol());
   }

   oldline=GetLine();
   oldcol=GetCol();

   if(!RDelete())
     return;

   if(lines_deleted && oldline>oldline2)
      oldline-=h;

   HardMove(oldline,oldcol);
   MainClipBoard.PasteAndMark();

   CurrentPos=BlockBegin;
   SetStdCol();
   hide=0;
}
void    Move()
{
   if(DragMark)
      UserStopDragMark();

   num	 size=BlockEnd-BlockBegin;

   CheckBlock();
   if(View || hide)
        return;

   flag=REDISPLAY_ALL;
   if(rblock)
   {
      RMove();
      return;
   }
   if(InBlock(CurrentPos))
      return;

   if(buffer_mmapped)
   {
      // FIXME
      return;
   }

   TextPoint tp=CurrentPos;

   if(CopyBlock(BlockBegin,size)!=OK)
      return;

   Delete();

   CurrentPos=BlockBegin=BlockEnd=tp;
   SetStdCol();
   BlockEnd+=size;
   hide=0;
}

void    Write()
{
   if(DragMark)
      UserStopDragMark();

   int         fd;
   struct stat st;
   num         act_written;

   CheckBlock();
   if(hide)
       return;

   if(getstring("Write to: ",BlockFile,sizeof(BlockFile)-1,&LoadHistory,0,
	 "WriteBlockHelp"," Write Block Help ")<1)
       return;
   if(BlockFile[0]=='|')
   {
      LoadHistory.Push();
      /* write to pipe */
      MessageSync("Piping to...");
      PipeBlock(BlockFile+1,/*IN*/FALSE,/*OUT*/TRUE);
      return;
   }
   if(ChooseFileName(BlockFile)<0)
      return;
   LoadHistory.Push();

   if(stat(BlockFile,&st)==-1)
   {
       if(errno!=ENOENT)
       {
           FError(BlockFile);
           return;
       }
       st.st_mode=0;
   }
   else
   {
       if(!(st.st_mode&S_IFCHR) && !CheckMode(st.st_mode))
           return;
       if(FileInfo.SameFile(InodeInfo(&st)))
       {
           ErrMsg("This is the editing file");
           return;
       }
   }
   fd=open(BlockFile,O_CREAT|((st.st_mode&S_IFCHR)?0:O_EXCL)|O_WRONLY,0666);
   if(fd==-1 && errno==EEXIST)
   {
       fd=open(BlockFile,O_WRONLY);
       if(fd==-1)
       {
           FError(BlockFile);
           return;
       }
       if(LockFile(fd,false)==-1)    /* check if the file is already locked */
       {
           close(fd);
           return;
       }
       close(fd);

       static struct menu OverwrMenu[]={
       { " &Overwrite "},{ "  &Append  "},{ "  &Cancel  "},
       {NULL}};
       switch(ReadMenuBox(OverwrMenu,HORIZ,"The file to write already exists",
	 " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
       {
       case('C'):
       case(0):
           return;
       case('O'):
           fd=open(BlockFile,O_WRONLY|O_TRUNC);
           break;
       case('A'):
           fd=open(BlockFile,O_WRONLY|O_APPEND);
       }
   }
   if(fd==-1)
   {
      FError(BlockFile);
      return;
   }
   MessageSync("Writing...");
   int res=OK;
   if(rblock)
   {
      ClipBoard cb;
      if(!cb.Copy())
      {
         close(fd);
         return;
      }
      errno=0;
      res=cb.Write(fd);
   }
   else
   {
      errno=0;
      res=WriteBlock(fd,BlockBegin,BlockEnd-BlockBegin,&act_written);
   }
   if(res<0)
      FError(BlockFile);
   close(fd);
}

int OptionallyConvertBlockNewLines(const char *bname)
{
   TextPoint old=CurrentPos;

   int block_size=BlockEnd-BlockBegin;
   if(buffer_mmapped || rblock || hex || block_size<EolSize)
      return 1;

   num   dos_nl,unix_nl,mac_nl;
   CountNewLines(BlockBegin,block_size,&unix_nl,&dos_nl,&mac_nl);
   static struct menu ynMenu[]={{"  &Yes  "},{"   &No   "},{NULL}};
   char msg[512];

   if(EolIs(EOL_DOS) && dos_nl*2<unix_nl)
   {
//       SyncTextWin();
      sprintf(msg,"The %s block looks like it is in UNIX format,\n"
		  "whereas the current text is in DOS format\n"
		  "Do you wish to convert the block?",bname);
      switch(ReadMenuBox(ynMenu,HORIZ,msg,
	       " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
      {
      case(0):
      case('N'):
	 break;
      case('Y'):
	 ConvertFromUnixToDos(BlockBegin,block_size);
	 flag=REDISPLAY_ALL;
	 break;
      }
   }
   else if(EolIs(EOL_UNIX) && dos_nl*2>unix_nl)
   {
//       SyncTextWin();
      sprintf(msg,"The %s block looks like it is in DOS format,\n"
		  "whereas the current text is in UNIX format\n"
		  "Do you wish to convert the block?",bname);
      switch(ReadMenuBox(ynMenu,HORIZ,msg,
	       " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
      {
      case(0):
      case('N'):
	 break;
      case('Y'):
	 ConvertFromDosToUnix(BlockBegin,block_size);
	 flag=REDISPLAY_ALL;
	 break;
      }
   }
   CurrentPos=old;
   SetStdCol();
   return 1;
}

void    Read()
{
   if(DragMark)
      UserStopDragMark();

   int             fd;
   struct stat     st;
   num   act_read;
   int res=OK;

   if(View)
      return;
   if(getstring("Read from: ",BlockFile,sizeof(BlockFile)-1,&LoadHistory,0,
	 "ReadBlockHelp"," Read Block Help ")<1)
      return;
   if(BlockFile[0]=='|')
   {
      LoadHistory.Push();
      /* read from pipe */
      MessageSync("Piping in...");
      if(PipeBlock(BlockFile+1,TRUE,FALSE)==OK)
	 goto after_read;
      return;
   }
   if(ChooseFileName(BlockFile)<0)
      return;
   LoadHistory.Push();

   fd=open(BlockFile,O_RDONLY);
   if(fd==-1)
   {
       FError(BlockFile);
       return;
   }
   errno=0;
   if(fstat(fd,&st)==-1)
   {
       close(fd);
       FError(BlockFile);
       return;
   }
   if(!CheckMode(st.st_mode))
   {
       close(fd);
       return;
   }
   MessageSync("Reading...");
   PreUserEdit();
   if(buffer_mmapped || (hex && !insert))
      res=ReadBlockOver(fd,st.st_size,&act_read);
   else
      res=ReadBlock(fd,st.st_size,&act_read);
   if(res!=OK)
   {
      close(fd);
      if(errno)
         FError(BlockFile);
      return;
   }
   close(fd);
   if(act_read==0)
      return;
   BlockBegin=BlockEnd=CurrentPos;
   BlockBegin-=act_read;
   rblock=hide=FALSE;

after_read:
   flag=REDISPLAY_ALL;
   OptionallyConvertBlockNewLines("read");
   CurrentPos=BlockEnd;
   SetStdCol();
}

void    DoIndent(int i)
{
   int j;
   num space;

   flag=1;

   if(rblock)
   {
      num    li;
      for(li=BlockBegin.Line(); li<=BlockEnd.Line(); li++)
      {
         HardMove(li,BlockBegin.Col());
         while(!Eol() && Space())
         {
            ExpandTab();
            MoveRight();
         }
         HardMove(li,BlockBegin.Col());
         for(j=i; j>0; j--)
            InsertChar(' ');
      }
      return;
   }

   CurrentPos=BlockBegin;
   ToLineBegin();
   BlockBegin=CurrentPos;
   do
   {
      space=0;
      while(!Eol() && Space())
      {
         if(Char()=='\t')
            space=Tabulate(space);
         else
            space++;
         DeleteChar();
      }
      j=space+i;
      if(UseTabs)
         for(; j>=TabSize; j-=TabSize)
            InsertChar('\t');
      for(; j>0; j--)
         InsertChar(' ');
      MoveRight();
      while(!Bol() && !Eof())
	 MoveRight();
   }
   while(CurrentPos<BlockEnd);
   BlockEnd=CurrentPos;
   SetStdCol();
   CheckPoint();
}
void    DoUnindent(int i)
{
   int j;
   num space;

   flag=1;
   if(rblock)
   {
      num    li;
      for(li=BlockBegin.Line(); li<=BlockEnd.Line(); li++)
      {
         HardMove(li,BlockBegin.Col());
         while(!Eol() && Space())
         {
            ExpandTab();
            MoveRight();
         }
         HardMove(li,BlockBegin.Col());
         for(j=i; j>0 && !Eol() && Space(); j--)
            DeleteChar();
      }
      return;
   }

   CurrentPos=BlockBegin;
   ToLineBegin();
   BlockBegin=CurrentPos;
   do
   {
      space=0;
      while(!Eol() && Space())
      {
         if(Char()=='\t')
            space=Tabulate(space);
         else
            space++;
         DeleteChar();
      }
      j=space-i;
      if(UseTabs)
         for(; j>=TabSize; j-=TabSize)
            InsertChar('\t');
      for(; j>0; j--)
         InsertChar(' ');
      MoveRight();
      while(!Bol() && !Eof())
	 MoveRight();
   }
   while(CurrentPos<BlockEnd);
   BlockEnd=CurrentPos;
   SetStdCol();
   CheckPoint();
}

char   is[64]="";
void   Indent()
{
   if(DragMark)
      UserStopDragMark();

   int    i;
   CheckBlock();
   if(View || hide)
      return;
   if(is[0]==0)
      sprintf(is,"%d",IndentSize);
   if(getstring("Indent size: ",is,sizeof(is)-1,NULL,NULL,NULL)<1)
      return;
   if(sscanf(is,"%d",&i)==0 || i==0 || abs(i)>1024)
   {
      is[0]=0;
      return;
   }
   if(i>0)
      DoIndent(i);
   else
      DoUnindent(-i);
}
void   Unindent()
{
   if(DragMark)
      UserStopDragMark();

   int    i;
   CheckBlock();
   if(View || hide)
      return;
   if(is[0]==0)
      sprintf(is,"%d",IndentSize);
   if(getstring("Unindent size: ",is,sizeof(is)-1,NULL,NULL,NULL)<1)
      return;
   if(sscanf(is,"%d",&i)==0 || i==0 || abs(i)>1024)
   {
      is[0]=0;
      return;
   }
   if(i>0)
      DoUnindent(i);
   else
      DoIndent(-i);
}

int Islower(byte ch)
{
   return(islower(ch) || islowerrus(ch));
}
int Isupper(byte ch)
{
   return(isupper(ch) || isupperrus(ch));
}
byte   Toupper(byte ch)
{
   if(islowerrus(ch))
      ch=toupperrus(ch);
   else if(islower(ch))
      ch=toupper(ch);
   return(ch);
}
byte   Tolower(byte ch)
{
   if(isupperrus(ch))
      ch=tolowerrus(ch);
   else if(isupper(ch))
      ch=tolower(ch);
   return(ch);
}
byte   Inverse(byte ch)
{
   if(Islower(ch))
      return(Toupper(ch));
   else
      return(Tolower(ch));
}

void    BlockFunc()
{
   if(DragMark)
   {
      UserStopDragMark();
      beep();
      return;
   }

   int   h=FALSE;
   int   action;
next:
   if((!rblock && BlockBegin>=BlockEnd) || hide)
      h=TRUE;

   if(!h && !View)
      MessageSync("Block: C-Copy M-Move D-Delete W-Write H-Hide I-Indent U-Unindent R-Read BETXLPA");
   else if(h && !View)
      MessageSync("Block: B-Begin E-End T-Type L-to Lower P-to uPper X-exchange H-display A-mark All");
   else if(h && View)
      MessageSync("Block: B-Begin E-End T-Type H-display A-mark All");
   else
      MessageSync("Block: W-Write B-Begin E-End T-Type H-Hide A-mark All");
   SetCursor();
   action=GetNextAction();
   flag=REDISPLAY_ALL;
   switch(action)
   {
   case(EDITOR_HELP):
      Help("BlockHelp"," Block Help ");
      goto next;
   case(REFRESH_SCREEN):
      UserRefreshScreen();
      break;
   default:
      if(StringTypedLen!=1)
         break;
      switch(toupper(StringTyped[0]))
      {
      case('V'):
	 UserStartDragMark();
	 break;
      case('C'):
         if(!h)
            UserCopyBlock();
         break;
      case('M'):
         if(!h)
            UserMoveBlock();
         break;
      case('D'):
         if(!h)
            UserDeleteBlock();
         break;
      case('W'):
         if(!h)
            Write();
         break;
      case('R'):
         Read();
         break;
      case('H'):
         HideDisplay();
         break;
      case('I'):
         if(!h)
             Indent();
         break;
      case('U'):
         if(!h)
             Unindent();
         break;
      case('B'):
         UserSetBlockBegin();
         break;
      case('E'):
         UserSetBlockEnd();
         break;
      case('B'-'@'):
         if(!h)
             CurrentPos=BlockBegin;
         break;
      case('E'-'@'):
         if(!h)
             CurrentPos=BlockEnd;
         break;
      case('T'):
         BlockType();
         break;
      case('P'):
         ConvertToUpper();
         break;
      case('L'):
         ConvertToLower();
         break;
      case('X'):
         ExchangeCases();
         break;
      case('Y'):
	 UserYankBlock();
	 break;
      case('A'):
	 UserMarkAll();
	 break;
      case('|'):
#ifndef MSDOS
         UserPipeBlock();
#else
         Message("Piping is not supported on MS-DOS. Press any key");
         GetNextAction();
#endif
         break;
      case('>'):
	 UserBlockPrefixIndent();
	 break;
      default:
         flag=FALSE;
         return;
      }
   }
}

void  Transform(byte (*func)(byte))
{
   TextPoint oldpos=CurrentPos;

   if(hide || (!rblock && BlockBegin==BlockEnd))
   {
      while(!Eol())
         ReplaceCharMove(func(Char()));
      flag|=REDISPLAY_LINE;
   }
   else
   {
      if(rblock)
      {
         num i;
         num line1=BlockBegin.Line();
         num line2=BlockEnd.Line();
         num col1 =BlockBegin.Col();
         num col2 =BlockEnd.Col();

         for(i=line1; i<=line2; i++)
         {
	    HardMove(i,col1);
            if(col2>col1)
            {
	       while(GetCol()<col2 && !Eol())
                  ReplaceCharMove(func(Char()));
            }
            else
            {
               while(!Eol())
                  ReplaceCharMove(func(Char()));
	    }
         }
      }
      else
      {
         CurrentPos=BlockBegin;
         while(CurrentPos<BlockEnd)
	    ReplaceCharMove(func(Char()));
      }
      flag=REDISPLAY_ALL;
   }
   CurrentPos=oldpos;
   SetStdCol();
}
#if USE_MULTIBYTE_CHARS
wctrans_t trans_toupper;
wchar_t ToupperW(wchar_t wc)
{
   return towctrans(wc,trans_toupper);
}
wctrans_t trans_tolower;
wchar_t TolowerW(wchar_t wc)
{
   return towctrans(wc,trans_tolower);
}
wchar_t InverseW(wchar_t wc)
{
   if(iswupper(wc))
      return towctrans(wc,trans_tolower);
   else
      return towctrans(wc,trans_toupper);
}
void  TransformW(wchar_t (*func)(wchar_t))
{
   TextPoint oldpos=CurrentPos;

   if(hide || (!rblock && BlockBegin==BlockEnd))
   {
      while(!Eol())
         ReplaceWCharMove(func(WChar()));
      flag|=REDISPLAY_LINE;
   }
   else
   {
      if(rblock)
      {
         num i;
         num line1=BlockBegin.Line();
         num line2=BlockEnd.Line();
         num col1 =BlockBegin.Col();
         num col2 =BlockEnd.Col();

         for(i=line1; i<=line2; i++)
         {
	    HardMove(i,col1);
            if(col2>col1)
            {
	       while(GetCol()<col2 && !Eol())
                  ReplaceWCharMove(func(WChar()));
            }
            else
            {
               while(!Eol())
                  ReplaceWCharMove(func(WChar()));
	    }
         }
      }
      else
      {
         CurrentPos=BlockBegin;
         while(CurrentPos<BlockEnd)
	    ReplaceWCharMove(func(WChar()));
      }
      flag=REDISPLAY_ALL;
   }
   CurrentPos=oldpos;
   SetStdCol();
}
#endif

void    ConvertToUpper()
{
   if(DragMark)
      UserStopDragMark();

#if USE_MULTIBYTE_CHARS
   if(mb_mode)
   {
      trans_toupper=wctrans("toupper");
      TransformW(ToupperW);
   }
   else
#endif
      Transform(Toupper);
}
void    ConvertToLower()
{
   if(DragMark)
      UserStopDragMark();

#if USE_MULTIBYTE_CHARS
   if(mb_mode)
   {
      trans_tolower=wctrans("tolower");
      TransformW(TolowerW);
   }
   else
#endif
      Transform(Tolower);
}
void    ExchangeCases()
{
   if(DragMark)
      UserStopDragMark();

#if USE_MULTIBYTE_CHARS
   if(mb_mode)
   {
      trans_toupper=wctrans("toupper");
      trans_tolower=wctrans("tolower");
      TransformW(InverseW);
   }
   else
#endif
      Transform(Inverse);
}
void    BlockType()
{
   rblock=!rblock;
   flag=!hide;
}

void    CheckBlock()
{
    if(rblock)
    {
        if(BlockBegin.Col()>BlockEnd.Col()
        || BlockBegin.Line()>BlockEnd.Line())
            hide=1;
    }
    else
    {
        if(BlockBegin.Offset()>=BlockEnd.Offset())
            hide=1;
    }
}

void  PrefixIndent(const char *prefix,num len)
{
   if(DragMark)
      UserStopDragMark();

   CheckBlock();
   if(hide)
      return;

   num l1,l2,c1;

   if(rblock)
   {
      l2=BlockEnd.Line()+1;
      c1=BlockBegin.Col();
   }
   else
   {
      BlockBegin=LineBegin(BlockBegin);
      if(BlockEnd.Col()!=0)
         BlockEnd=NextLine(BlockEnd);

      l2=BlockEnd.Line();
      c1=0;
   }
   l1=BlockBegin.Line();
   for(num i=l1; i<l2; i++)
   {
      HardMove(i,c1);
      InsertBlock(prefix,len);
   }
}
