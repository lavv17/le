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

/* block.cc : block operations */
/* $Id$ */

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

extern  char    **BlockHelp[];

char    BlockFile[256];

int     hide=TRUE;

int     InBlock(offs ptr,num line,num col)
{
    if(hide)
        return(FALSE);
    if(!rblock || hex)
    {
        return(ptr>=BlockBegin.Offset() && ptr<BlockEnd.Offset());
    }
    else
    {
        if(line>=BlockBegin.Line()
        && line<=BlockEnd.Line())
        {
            if(BlockBegin.Col()>BlockEnd.Col())
            {
                TextPoint tp=CurrentPos;
                HardMove(BlockEnd.Line(),BlockBegin.Col());
                BlockEnd=CurrentPos;
                CurrentPos=tp;
            }
            if(BlockBegin.Col()==BlockEnd.Col())
                return(col>=BlockBegin.Col());
            return(col>=BlockBegin.Col() && col<BlockEnd.Col());
        }
        return(FALSE);
    }
}

void    MoveLineCol(num l,num c)
{
   CurrentPos=TextPoint(l,c);
}
void    HideDisplay()
{
   hide=!hide;
   CheckBlock();
   flag=1;
}
char    CharAtLC(num l,num c)
{
    static  TextPoint   Last;
    Last=TextPoint(l,c);
    return((EolAt(Last)||Last.Col()!=c||Last.Line()!=l)?' ':CharAt(Last));
}
void    NewLine()
{
    InsertBlock(EolStr,EolSize);
}

/* Moves exactly to specified line/column, edits the text as needed */
void    HardMove(num l,num c)
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
    stdcol=GetCol();
}

void    ExpandTab()
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
   ClipBoard cb;

   if(!cb.Copy())
      return;

   cb.PasteAndMark();
   CurrentPos=BlockBegin;
   stdcol=GetCol();
}

void    Copy()
{
    CheckBlock();
    if(View || hide)
        return;

    PreUserEdit();

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
   num   i,j;
   num   h=BlockEnd.Line()-BlockBegin.Line()+1;
   num   oldcol2=BlockEnd.Col();

   lines_deleted=0;

   if(!MainClipBoard.Copy())
      return 0;

   for(i=BlockBegin.Line(); i<=BlockEnd.Line(); i++)
   {
      j=BlockBegin.Col();
      HardMove(i,j);
      while(!Eol() && (BlockBegin.Col()==oldcol2
		   || j<oldcol2))
      {
	 if(Char()=='\t')
	    ExpandTab();
	 j++;
	 DeleteChar();
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

   hide=TRUE;
   return 1;
}
int   Delete()
{
   CheckBlock();
   if(View || hide)
      return 1;
   flag=REDISPLAY_ALL;
   if(rblock)
      return RDelete();
   if(!MainClipBoard.Copy())
      return 0;
   if(!InBlock(Offset(),GetLine(),GetCol()))
      CurrentPos=BlockBegin;
   DeleteBlock(CurrentPos-BlockBegin,BlockEnd-CurrentPos);
   stdcol=GetCol();
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
   stdcol=GetCol();
   hide=0;
}
void    Move()
{
    num     size=BlockEnd-BlockBegin;

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

    TextPoint tp=CurrentPos;

    if(CopyBlock(BlockBegin,size)!=OK)
       return;

    Delete();

    CurrentPos=BlockBegin=BlockEnd=tp;
    stdcol=GetCol();
    BlockEnd+=size;
    hide=0;
}

void    Write()
{
   int         fd;
   struct stat st;
   num         act_written;

   CheckBlock();
   if(hide)
       return;

   if(getstring("Write to: ",BlockFile,sizeof(BlockFile)-1,&LoadHistory,NULL,NULL)<1 || ChooseFileName(BlockFile)<0)
       return;
   LoadHistory.Push();

   if(BlockFile[0]=='|')
   {
      /* write to pipe */
      Message("Piping to...");
      PipeBlock(BlockFile+1,/*IN*/FALSE,/*OUT*/TRUE);
      return;
   }

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
       if(LockFile(fd)==-1)    /* check if the file is already locked */
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
   errno=0;
   Message("Writing...");
   if(rblock)
   {
      ClipBoard cb;
      if(!cb.Copy())
      {
         close(fd);
         return;
      }
      cb.Write(fd);
   }
   else
   {
      WriteBlock(fd,BlockBegin,BlockEnd-BlockBegin,&act_written);
   }
   if(errno)
      FError(BlockFile);
   close(fd);
}

void    Read()
{
   int             fd;
   struct stat     st;
   num   act_read;

   if(View)
      return;
   if(getstring("Read from: ",BlockFile,sizeof(BlockFile)-1,&LoadHistory,NULL,NULL)<1)
      return;
   if(BlockFile[0]=='|')
   {
      LoadHistory.Push();
      /* read from pipe */
      Message("Piping in...");
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
   if(GetSpace(st.st_size)!=OK)
       return;
   Message("Reading...");
   PreUserEdit();
   if(ReadBlock(fd,st.st_size,&act_read)!=OK)
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
   flag=REDISPLAY_ALL;
   rblock=hide=FALSE;

after_read:
   act_read=BlockEnd-BlockBegin;
   if(!hex)
   {
      num   dos_nl,unix_nl;
      CountNewLines(BlockBegin,act_read,&unix_nl,&dos_nl);
      struct   menu  ynMenu[]={{"  &Yes  "},{"   &No   "},{NULL}};

      if(DosEol && dos_nl*2<unix_nl)
      {
	 switch(ReadMenuBox(ynMenu,HORIZ,"The read block looks like it is in UNIX format,\n"
					 "whereas the current text is in DOS format\n"
					 "Do you wish to convert the block?",
		  " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
	 {
	 case(0):
	 case('N'):
	    break;
	 case('Y'):
	    ConvertFromUnixToDos(BlockBegin,act_read);
	    break;
	 }
      }
      if(!DosEol && dos_nl*2>unix_nl)
      {
	 switch(ReadMenuBox(ynMenu,HORIZ,"The read block looks like it is in DOS format,\n"
					 "whereas the current text is in UNIX format\n"
					 "Do you wish to convert the block?",
		  " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
	 {
	 case(0):
	 case('N'):
	    break;
	 case('Y'):
	    ConvertFromDosToUnix(BlockBegin,act_read);
	    break;
	 }
      }
   }
   CurrentPos=BlockEnd;
   stdcol=GetCol();
}

void    DoIndent(int i)
{
    int j;
    num space;

    flag=1;

    if(rblock)
    {
        num     li;
        for(li=BlockBegin.Line(); li<=BlockEnd.Line(); li++)
        {
            HardMove(li,BlockBegin.Col());
            while(!Eol() && isspace(Char()))
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
        while(!Eol() && isspace(Char()))
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
        MoveDown();
        ToLineBegin();
    }
    while(CurrentPos<BlockEnd);
    BlockEnd=CurrentPos;
    CheckPoint();
}
void    DoUnindent(int i)
{
    int j;
    num space;

    flag=1;
    if(rblock)
    {
        num     li;
        for(li=BlockBegin.Line(); li<=BlockEnd.Line(); li++)
        {
            HardMove(li,BlockBegin.Col());
            while(!Eol() && isspace(Char()))
            {
                ExpandTab();
                MoveRight();
            }
            HardMove(li,BlockBegin.Col());
            for(j=i; j>0 && !Eol() && isspace(Char()); j--)
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
        while(!Eol() && isspace(Char()))
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
        MoveDown();
        ToLineBegin();
    }
    while(CurrentPos<BlockEnd);
    BlockEnd=CurrentPos;
    CheckPoint();
}

char    is[64]="";
void    Indent()
{
    int     i;
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
void    Unindent()
{
    int     i;
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
byte    Toupper(byte ch)
{
    if(islowerrus(ch))
        ch=toupperrus(ch);
    else if(islower(ch))
        ch=toupper(ch);
    return(ch);
}
byte    Tolower(byte ch)
{
    if(isupperrus(ch))
        ch=tolowerrus(ch);
    else if(isupper(ch))
        ch=tolower(ch);
    return(ch);
}
byte    Inverse(byte ch)
{
    if(Islower(ch))
        return(Toupper(ch));
    else
        return(Tolower(ch));
}

void    BlockFunc()
{
   int   h=FALSE;
   int   action;
next:
   if((!rblock && BlockBegin>=BlockEnd) || hide)
      h=TRUE;

   if(!h && !View)
      Message("Block: C-Copy M-Move D-Delete W-Write H-Hide I-Indent U-Unindent R-Read BETXLP");
   else if(h && !View)
      Message("Block: B-Begin E-End T-Type L-to Lower P-to uPper X-exchange H-display");
   else if(h && View)
      Message("Block: B-Begin E-End T-Type H-display");
   else
      Message("Block: W-Write B-Begin E-End T-Type H-Hide");
   SetCursor();
   action=GetNextAction();
   flag=REDISPLAY_ALL;
   switch(action)
   {
   case(EDITOR_HELP):
      Help(BlockHelp," Block Help ");
      goto next;
   case(REFRESH_SCREEN):
      UserRefreshScreen();
      break;
   default:
      if(StringTypedLen!=1)
         break;
      switch(toupper(StringTyped[0]))
      {
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
      case('|'):
#ifndef MSDOS
         UserPipeBlock();
#else
         Message("Piping is not supported in MS-DOS. Press any key");
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
          ReplaceChar1(func(Char()));
       flag|=REDISPLAY_LINE;
   }
   else
   {
      if(rblock)
      {
         num i,j;
         num line1=BlockBegin.Line();
         num line2=BlockEnd.Line();
         num col1 =BlockBegin.Col();
         num col2 =BlockEnd.Col();

         for(i=line1; i<=line2; i++)
         {
            if(col2>col1)
            {
                for(j=col1; j<col2; j++)
                {
                    MoveLineCol(i,j);
                    if(!isspace(Char()))
                        ReplaceChar1(func(Char()));
                }
            }
            else
            {
                GoToLineNum(i);
                while(!Eol())
                    ReplaceChar1(func(Char()));
            }
         }
      }
      else
      {
         CurrentPos=BlockBegin;
         while(CurrentPos<BlockEnd)
	 {
	    InsertChar(func(Char()));
	    DeleteChar();
	 }
      }
      flag=REDISPLAY_ALL;
   }
   CurrentPos=oldpos;
   stdcol=GetCol();
}

void    ConvertToUpper()
{
    Transform(Toupper);
}
void    ConvertToLower()
{
    Transform(Tolower);
}
void    ExchangeCases()
{
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

void  PrefixIndent(char *prefix,num len)
{
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
