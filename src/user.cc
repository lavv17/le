/*
 * Copyright (c) 1993-2021 by Alexander V. Lukyanov (lav@yars.free.net)
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include "edit.h"
#include "block.h"
#include "keymap.h"
#include "clipbrd.h"
#include "getch.h"
#include "format.h"
#include "about.h"
#include "bm.h"
#include "undo.h"
#include "highli.h"

void  UserDeleteToEol()
{
   if(View || hex)
      return;
   DeleteToEOL();
   if(!Text)
      SetStdCol();
   flag|=REDISPLAY_LINE;
}
void  UserDeleteLine()
{
   if(View || hex)
      return;
   DeleteLine();
   flag|=REDISPLAY_AFTER;
}

void  UserLineUp()
{
   if(hex)
   {
      CurrentPos-=16;
   }
   else
   {
      MoveUp();
   }
}
void  UserLineDown()
{
   if(hex)
   {
      CurrentPos+=16;
   }
   else
   {
      num line=GetLine();
      MoveDown();
      if(line>=GetLine())
      {
	 if(Text)
	 {
	    if(!Bol())
	    {
	       num old_stdcol=SaveStdCol();
	       int old_modified=modified;
	       NewLine();
	       RestoreStdCol(old_stdcol);
	       modified=old_modified;
	    }
	 }
	 else
	    SetStdCol();
      }
   }
}

void  UserCharLeft()
{
   if(hex)
   {
      if(ascii)
         MoveLeft();
      else
      {
         if(right)
            right=0;   /* shift cursor from the rigth hex digit to the left one */
         else
         {
            MoveLeft();
            right=1;
         }
      }
   }
   else
   {
      if(Text && Eol() && stdcol>GetCol())
      {
         stdcol--;
         return;
      }
      MoveLeftOverEOL();
   }
   SetStdCol();
}
void  UserCharRight()
{
   if(hex)
   {
      if(ascii)
	 MoveRight();
      else
      {
         if(right)
         {
            MoveRight();
            right=0;
         }
         else
            right=1;
      }
   }
   else
   {
      if(Text && Eol())
      {
         AddStdCol(1);
      }
      else
      {
         MoveRightOverEOL();
         SetStdCol();
      }
   }
}

void  UserCopyFromDown()
{
   if(View || hex)
      return;

   num   oc=GetCol();
   if(Text && stdcol>oc && Eol())
      oc=stdcol;

   TextPoint tp=CurrentPos;

   for(;;)
   {
      tp=TextPoint(tp.Line()+1,oc);
      if(EofAt(tp.Offset()))
         break;
      if(tp.Col()>oc)
      {
         PreUserEdit();
         InsertChar('\t');
	 SetStdCol();
         flag|=REDISPLAY_LINE;
         return;
      }
      if(tp.Col()==oc && !EolAt(tp.Offset()))
      {
         wchar_t ch=WCharAt(tp.Offset());
         PreUserEdit();
         if(insert)
            InsertWChar(ch);
         else
            ReplaceWCharExtMove(ch);
	 SetStdCol();
         flag|=REDISPLAY_LINE;
         return;
      }
   }
}
void  UserCopyFromUp()
{
   if(View || hex)
      return;

   num   oc=GetCol();
   if(Text && stdcol>oc && Eol())
      oc=stdcol;

   TextPoint tp=CurrentPos;

   while(tp.Line()>0)
   {
      tp=TextPoint(tp.Line()-1,oc);
      if(tp.Col()>oc)
      {
         PreUserEdit();
         InsertChar('\t');
	 SetStdCol();
         flag|=REDISPLAY_LINE;
         return;
      }
      if(tp.Col()==oc && !EolAt(tp.Offset()))
      {
         wchar_t ch=WCharAt(tp.Offset());
         PreUserEdit();
         if(insert)
            InsertWChar(ch);
         else
            ReplaceWCharExtMove(ch);
	 SetStdCol();
         flag|=REDISPLAY_LINE;
         return;
      }
   }
}

void  UserDeleteBlock()
{
   if(DragMark)
      UserStopDragMark();

   if(View)
      return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      Delete();
   }
}
void  UserCopyBlock()
{
   if(DragMark)
      UserStopDragMark();

   if(View)
      return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      PreUserEdit();
      Copy();
   }
}
void  UserMoveBlock()
{
   if(DragMark)
      UserStopDragMark();

   if(View)
       return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      PreUserEdit();
      Move();
   }
}

void  UserBackwardDeleteWord()
{
   if(View || hex)
      return;
   if(!IsAlNumLeft() && CharRel(-1)!=' ' && CharRel(-1)!='\t')
   {
      UserBackSpace();
   }
   else
   {
      PreUserEdit();
      if(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
      {
	 while(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
	    DeleteBlock(1,0);
      }
      else
      {
	 while(!Bol() && IsAlNumLeft())
	    DeleteBlock(MBCharSize,0);
      }
   }
   SetStdCol();
   flag|=REDISPLAY_LINE;
}

void  UserForwardDeleteWord()
{
   if(View || hex)
      return;
   if(!IsAlNumRel(0) && Char()!=' ' && Char()!='\t')
      UserDeleteChar();
   else
   {
      PreUserEdit();
      if(!Eol() && (Char()==' ' || Char()=='\t'))
      {
	 while(!Eol() && (Char()==' ' || Char()=='\t'))
	    DeleteBlock(0,1);
      }
      else
      {
	 while(!Eol() && IsAlNumRel(0))
            DeleteBlock(0,MBCharSize);
      }
   }
   SetStdCol();
   flag|=REDISPLAY_LINE;
}

void  UserDeleteWord()
{
   if(View || hex)
      return;
   if(!IsAlNumRel(0))
      UserForwardDeleteWord();
   else
   {
      PreUserEdit();
      while(!Eol() && IsAlNumRel(0))
         DeleteBlock(0,MBCharSize);
      while(!Bol() && IsAlNumLeft())
	 DeleteBlock(MBCharSize,0);
   }
   SetStdCol();
   flag|=REDISPLAY_LINE;
}

void  UserMarkWord()
{
   if(DragMark)
      UserStopDragMark();

   offs word_begin=CurrentPos;
   offs word_end=CurrentPos;
   while(!BofAt(word_begin))
   {
      (void)MBCheckLeftAt(word_begin);
      if(IsAlNumAt(word_begin-MBCharSize))
	 word_begin--;
      else
	 break;
   }
   while(!EofAt(word_end) && IsAlNumAt(word_end))
      word_end+=MBCharSize;
   if(word_end==word_begin)
      word_end+=MBCharSize;
   BlockBegin=word_begin;
   BlockEnd=word_end;
   hide=FALSE;
   flag=REDISPLAY_ALL;
}
void  UserMarkLine()
{
   if(DragMark)
      UserStopDragMark();

   BlockBegin=LineBegin(Offset());
   if(rblock)
      BlockEnd=BlockBegin;
   else
      BlockEnd=NextLine(Offset());
   hide=FALSE;
   flag=REDISPLAY_ALL;
}
void  UserMarkToEol()
{
   if(DragMark)
      UserStopDragMark();

   SetStdCol();
   BlockBegin=CurrentPos;
   BlockEnd=LineEnd(CurrentPos.Offset());
   hide=(BlockEnd.Col()<=BlockBegin.Col());
   flag=REDISPLAY_ALL;
}
void  UserMarkAll()
{
   if(DragMark)
      UserStopDragMark();

   BlockBegin=TextBegin;
   if(rblock)
      BlockEnd=LineBegin(TextEnd);
   else
      BlockEnd=TextEnd;
   hide=FALSE;
   flag=REDISPLAY_ALL;
}

void  UserPageTop()
{
   if(hex)
   {
      if((CurrentPos&~15)==ScreenTop)
         CurrentPos-=(TextWinHeight-1)*16;
      else
         CurrentPos=ScreenTop+(CurrentPos&15);
   }
   else
   {
      if(Text)
      {
	 num oldstdcol=SaveStdCol();
         ToLineEnd();	// clear spaces at the line end
	 RestoreStdCol(oldstdcol);
      }

      if(GetLine()==ScreenTop.Line())
      {
	 CurrentPos=TextPoint(ScreenTop.Line()-(TextWinHeight-1),GetStdCol());
	 ScreenTop=LineBegin(CurrentPos);
	 flag=REDISPLAY_ALL;
      }
      else
	 CurrentPos=ScreenTop;
   }
}
void UserScrollUp()
{
   if(hex) {
      ScreenTop-=16;
      if((CurrentPos-ScreenTop)/16>=TextWinHeight)
	 CurrentPos-=16;
   } else {
      ScreenTop=PrevLine(ScreenTop);
      if(GetLine()>=ScreenTop.Line()+TextWinHeight)
	 UserLineUp();
   }
   flag=REDISPLAY_ALL;
}
void UserScrollDown()
{
   if(hex) {
      if((TextEnd-ScreenTop)/16>=TextWinHeight) {
	 ScreenTop+=16;
	 if(CurrentPos<ScreenTop)
	    CurrentPos+=16;
      }
   } else {
      if(TextEnd.Line()-ScreenTop.Line()>=TextWinHeight) {
	 ScreenTop=NextLine(ScreenTop);
	 if(CurrentPos<ScreenTop)
	    UserLineDown();
      }
   }
   flag=REDISPLAY_ALL;
}
void  UserPageUp()
{
   if(PreferPageTop)
   {
      UserPageTop();
      return;
   }

   if(hex)
   {
      int page_size=(TextWinHeight-1)*16;
      CurrentPos-=page_size;
      ScreenTop-=page_size;
   }
   else
   {
      num oldstdcol=SaveStdCol();
      if(Text)
	 ToLineEnd();
      CurrentPos=PrevNLines(CurrentPos,TextWinHeight-1);
      ScreenTop=PrevNLines(ScreenTop,TextWinHeight-1);
      RestoreStdCol(oldstdcol);
   }
   flag=REDISPLAY_ALL;
}
void  UserPageBottom()
{
   if(hex)
   {
      int pgsize=(TextWinHeight-1)*16;
      if(CurrentPos>=ScreenTop+pgsize)
	 CurrentPos+=pgsize;
      else
	 CurrentPos+=pgsize-((CurrentPos&~15)-ScreenTop);
   }
   else
   {
      if(Text)
      {
	 num oldstdcol=SaveStdCol();
         ToLineEnd();
	 RestoreStdCol(oldstdcol);
      }

      if(GetLine()==ScreenTop.Line()+TextWinHeight-1)
      {
	 CurrentPos=TextPoint(GetLine()+TextWinHeight-1,GetStdCol());
	 ScreenTop=TextPoint(GetLine()-(TextWinHeight-1),0);
	 flag=REDISPLAY_ALL;
      }
      else
	 CurrentPos=TextPoint(ScreenTop.Line()+TextWinHeight-1,GetStdCol());
   }
}
void  UserPageDown()
{
   if(PreferPageTop)
   {
      UserPageBottom();
      return;
   }

   if(hex)
   {
      int page_size=(TextWinHeight*16-16);
      CurrentPos+=page_size;
      if(TextEnd-ScreenTop>=2*page_size)
	 ScreenTop+=page_size;
      else if(TextEnd>=page_size)
	 ScreenTop=(TextEnd-page_size)&~15;
   }
   else
   {
      num oldstdcol=SaveStdCol();

      if(Text)
	 ToLineEnd();

      CurrentPos=NextNLines(CurrentPos,TextWinHeight-1);
      if(TextEnd.Line()>=ScreenTop.Line()+2*TextWinHeight-2)
	 ScreenTop=NextNLines(ScreenTop,TextWinHeight-1);
      else
      {
	 offs NewScreenTop=PrevNLines(TextEnd,TextWinHeight-1);
	 if(NewScreenTop>ScreenTop)
	    ScreenTop=NewScreenTop;
      }

      RestoreStdCol(oldstdcol);
   }
   flag=REDISPLAY_ALL;
}

void  UserWordLeft()
{
   if(hex && !ascii)
      MoveLeft();
   else
   {
      while(!Bof() && !IsAlNumLeft())
         CurrentPos-=MBCharSize;
      while(!Bof() && IsAlNumLeft())
         CurrentPos-=MBCharSize;
   }
   SetStdCol();
}
void  UserWordRight()
{
   if(hex && !ascii)
      MoveRight();
   else
   {
      while(!Eof() && !IsAlNumRel(0))
         CurrentPos+=MBCharSize;
      while(!Eof() && IsAlNumRel(0))
         CurrentPos+=MBCharSize;
   }
   SetStdCol();
}

void  UserMenu()
{
   ActivateMainMenu();
}

void  UserCommentLine()
{
   int unc=0;
   TextPoint   op=CurrentPos;

   if(View || hex)
      return;

   ToLineBegin();
   if(Suffix(FileName,".cc")
   || Suffix(FileName,".cpp")
   || Suffix(FileName,".cxx")
   || Suffix(FileName,".java"))
   {
      if(BlockEq("//",2))
      {
	 DeleteBlock(0,2);
	 if(Char()==' ')
	    DeleteBlock(0,1);
      }
      else
      {
	 InsertBlock("// ",3);
      }
   }
   else if(Suffix(FileName,".sql"))
   {
      if(BlockEq("--",2))
      {
	 DeleteBlock(0,2);
	 if(Char()==' ')
	    DeleteBlock(0,1);
      }
      else
      {
	 InsertBlock("-- ",3);
      }
   }
   else if(Suffix(FileName,".c") || Suffix(FileName,".h")
   || Suffix(FileName,".css"))
   {
      if(BlockEq("//",2))
      {
	 DeleteBlock(0,2);
	 if(Char()==' ')
	    DeleteBlock(0,1);
	 goto done;
      }
      if(BlockEq("/*",2))
      {
	 unc=1;
	 DeleteBlock(0,2);
      }
      ToLineEnd();
      if(BlockEqLeft("*/",2))
      {
	 unc=1;
	 DeleteBlock(2,0);
      }
      if(!unc)
      {
	 InsertBlock("*/",2);
	 ToLineBegin();
	 InsertBlock("/*",2);
      }
   }
   else if(Suffix(FileName,".html") || Suffix(FileName,".htm")
   || Suffix(FileName,".shtml"))
   {
      if(BlockEq("<!--",4))
      {
	 unc=1;
	 DeleteBlock(0,4);
      }
      ToLineEnd();
      if(BlockEqLeft("-->",3))
      {
	 unc=1;
	 DeleteBlock(3,0);
      }
      if(!unc)
      {
	 InsertBlock("-->",3);
	 ToLineBegin();
	 InsertBlock("<!--",4);
      }
   }
   else // default
   {
      if(Char()=='#')
      {
	 DeleteChar();
	 if(Char()==' ')
	    DeleteChar();
      }
      else
      {
	 InsertBlock("# ",2);
      }
   }
done:
   CurrentPos=op;
   SetStdCol();
   flag|=REDISPLAY_LINE;
}

void  UserSetBlockBegin()
{
   PreUserEdit();
   flag=REDISPLAY_ALL;
   if(hide)
   {
      BlockBegin=BlockEnd=CurrentPos;
      hide=FALSE;
      return;
   }
   if(rblock?(CurrentPos.Line()<=BlockEnd.Line()
              && CurrentPos.Col()<=BlockEnd.Col())
            :(CurrentPos.Offset()<=BlockEnd.Offset()))
   //then
      BlockBegin=CurrentPos;
   else
   {
      BlockBegin=/*BlockEnd;*/
      BlockEnd=CurrentPos;
   }
   if(DragMark)
   {
      if(*DragMark < BlockBegin)
	 *DragMark = BlockBegin;
   }
}
void  UserSetBlockEnd()
{
   PreUserEdit();
   flag=REDISPLAY_ALL;
   if(hide)
   {
      BlockBegin=BlockEnd=CurrentPos;
      hide=FALSE;
      return;
   }
   if(rblock?(CurrentPos.Line()>=BlockBegin.Line()
              && CurrentPos.Col()>=BlockBegin.Col())
            :(CurrentPos.Offset()>=BlockBegin.Offset()))
   //then
      BlockEnd=CurrentPos;
   else
   {
      BlockEnd=/*BlockBegin;*/
      BlockBegin=CurrentPos;
   }
   if(DragMark)
   {
      if(*DragMark > BlockEnd)
	 *DragMark = BlockEnd;
   }
}

void  UserFindBlockBegin()
{
   if(hide)
      return;
   CurrentPos=BlockBegin;
   SetStdCol();
}
void  UserFindBlockEnd()
{
   if(hide)
      return;
   CurrentPos=BlockEnd;
   SetStdCol();
}

void  UserLineBegin()
{
   if(Text && !View)
      ToLineEnd();
   ToLineBegin();
   SetStdCol();
}
void  UserLineEnd()
{
   ToLineEnd();
   SetStdCol();
   if(autoindent && Text && Bol() && !Bof())
   {
      bool old_modified=modified;
      InsertAutoindent(TextPoint(CurrentPos-EolSize).Col());
      modified=old_modified;
   }
}
void  UserFileBegin()
{
   CurrentPos=TextBegin;
   SetStdCol();
}
void  UserFileEnd()
{
   CurrentPos=TextEnd;
   SetStdCol();
}

void  UserPreviousEdit()
{
   if(!modified)
      return;
   CurrentPos=ptr1;
   SetStdCol();
}

void  UserUnindent()
{
   num newmargin;
   num oldmargin;
   offs pos;
   int sz;
   num   curpos=GetCol();

   if(Text && curpos<stdcol && Eol())
      curpos=stdcol;

   pos=LineBegin(Offset());
   oldmargin=MarginSizeAt(pos);

   if(oldmargin==-1)
   {
      oldmargin=GetCol();
      if(Text && oldmargin<stdcol)
	 oldmargin=stdcol;
   }

   if(oldmargin!=curpos || oldmargin==0)
   {
      if(Text && Eol() && stdcol>GetCol())
      {
         UserLineEnd();
	 return;
      }
      BackSpace();
   }
   else
   {
      for(;;)
      {
         pos=PrevLine(pos);
         newmargin=MarginSizeAt(pos);
         if(newmargin>=0 && newmargin<oldmargin)
            break;
         if(BofAt(pos))
         {
            newmargin=((oldmargin-1)/IndentSize)*IndentSize;
            break;
         }
      }
      if(Text && Eol())
      {
         DeleteToBOL();
         if(newmargin<curpos)
	    stdcol=newmargin;
	 else
	    stdcol=0;
	 flag|=REDISPLAY_LINE;
         return;
      }
      while(GetCol()>newmargin)
      {
         if(CharRel(-1)=='\t')
         {
            MoveLeft();
            if(newmargin<=GetCol())
               DeleteChar();
            else
            {
               sz=newmargin-GetCol();
               while(sz-->0)
                  InsertChar(' ');
               DeleteChar();
            }
         }
         else
            BackSpace();
      }
   }
   flag|=REDISPLAY_LINE;
   SetStdCol();
}

void  UserBackSpace()
{
   if(View)
      return;
   if(Bof() && (!Text || GetStdCol()==0))
      return;
   if(hex)
   {
      BackSpace();
      flag|=REDISPLAY_AFTER;
      return;
   }
   if(Bol() && (!Text || GetStdCol()==0))
   {
      DeleteBlock(EolSize,0);
      flag|=REDISPLAY_AFTER;
   }
   else
   {
      if(!BackspaceUnindents)
      {
	 if(Text && Eol() && stdcol>GetCol())
	 {
	    //UserLineEnd();
	    AddStdCol(-1);
	    return;
	 }
         BackSpace();
         flag|=REDISPLAY_LINE;
      }
      else
      {
         UserUnindent();
         return;
      }
   }
   SetStdCol();
}

void  UserDeleteChar()
{
   if(View)
      return;
   if(Eof())
      return;
   if(hex)
   {
      DeleteChar();
      flag=REDISPLAY_AFTER;
   }
   else
   {
      PreUserEdit();
      if(Eol())
      {
         DeleteEOL();
         flag=REDISPLAY_AFTER;
      }
      else
      {
         DeleteChar();
         flag=REDISPLAY_LINE;
      }
   }
   SetStdCol();
}

int   UserSave()
{
   if(FileName[0] && !View)
      return(SaveFile(FileName));
   else
      return(UserSaveAs());
}

int   file_check(const char *fn)
{
   char	 dir[256];
   char	 *slash;
   char	 msg[1024];

   if(buffer_mmapped) {
      char *open_name1=(char*)alloca(strlen(fn)+1);
      unsigned long long mmap_begin=0;
      unsigned long mmap_len=0;
      if(sscanf(fn,"%[^:]:%lli:%li",open_name1,&mmap_begin,&mmap_len)==3)
	 fn=open_name1;
   }

   if(access(fn,R_OK)==-1)
   {
      if(access(fn,F_OK)==0)
      {
	 sprintf(msg,"File: %s\nThe specified file is not readable",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      if((View&RO_MODE) || buffer_mmapped)  // view mode or mmap mode
      {
	 sprintf(msg,"File: %s\nThe specified file does not exist",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      strcpy(dir,fn);
      slash=dir+strlen(dir);
      while(slash>dir && !isslash(*--slash));
      if(slash>dir)
	 *slash=0;
      else
	 strcpy(dir,".");
      if(access(dir,F_OK)==-1)
      {
	 sprintf(msg,"File: %s\nThe specified directory does not exist",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      if(access(dir,W_OK|X_OK)==-1)
      {
	 sprintf(msg,"File: %s\nThe specified file does not exist\n"
		"and the directory does not permit creating",fn);
	 ErrMsg(msg);
	 return ERR;
      }

      struct menu CreateOrNot[]=
      {
	 {" C&reate ",MIDDLE-6,4},
	 {" &Cancel ",MIDDLE+6,4},
	 {NULL}
      };
      sprintf(msg,"The file `%s' does not exist. Create?",fn);
      switch(ReadMenuBox(CreateOrNot,HORIZ,msg,
	 " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
      {
      case('R'):
	 return OK;
      default:
	 return ERR;
      }
   }
   return OK;
}

void    UserLoad()
{
   char  newname[256];
   newname[0]=0;

   if(getstring("Load: ",newname,sizeof(newname)-1,&LoadHistory)>0)
   {
      if(ChooseFileName(newname)<0)
         return;
      if(file_check(newname)==ERR)
      {
	 LoadHistory.Push();
	 return;
      }

      if(modified)
      {
         if(!AskToSave())
         {
            LoadHistory.Push();
            return;
         }
      }
      LoadFile(newname);
   }
}

int   UserSaveAs()
{
   char  newname[256];
   newname[0]=0;

   if(getstring("Save as: ",newname,sizeof(newname)-1,&LoadHistory,NULL,NULL)>0)
   {
      if(ChooseFileName(newname)<0)
         return(ERR);
      if(SaveFile(newname)!=OK)
      {
         LoadHistory.Push();
         return(ERR);
      }
      return(OK);
   }
   return(ERR);
}
void  UserSwitch()
{
   LoadHistory.Open();
   LoadHistory.Prev();
   const HistoryLine *prev=LoadHistory.Prev();
   if(prev==NULL)
   {
      UserLoad();
      return;
   }

   char newname[256];
   strncpy(newname,prev->get_line(),255);
   newname[255]=0;

   if(ChooseFileName(newname)<0)
      return;

   if(access(newname,R_OK)==-1)
   {
      UserLoad();
      return;
   }

   if(modified)
      if(!AskToSave())
         return;

   LoadHistory+=newname;

   LoadFile(newname);
}

void  UserInfo()
{
   WIN   *InfoWin;
   char  cwd[1024];
   char  s[256];
   int   cl;
   time_t t;
   uid_t uid=geteuid();
   gid_t gid=getegid();
   struct passwd  *pw;
   struct group   *gr;

   DisplayWin(InfoWin=CreateWin(MIDDLE,MIDDLE,50,20,DIALOGUE_WIN_ATTR," Info ",0));

   pw=getpwuid(uid);
   gr=getgrgid(gid);

   strcpy(cwd,"Unknown");
   if (!getcwd(cwd,sizeof(cwd)))
      /*ignore*/;

   do
   {
      sprintf(s,"File: %.40s",FileName);
      PutStr(3,cl=2,s);

      sprintf(s,"Line=%-6ld Col=%-6ld\nSize:%-6ld Offset:%-6ld",(long)GetLine(),
             (long)(Text&&Eol()?GetStdCol():GetCol()),(long)Size(),(long)Offset());
      PutStr(3,cl+=2,s);

      sprintf(s,"CWD:  %.40s",cwd);
      PutStr(3,cl+=3,s);

      time(&t);
      sprintf(s,"Date: %s",ctime(&t));
      PutStr(3,cl+=1,s);

      sprintf(s,"User: %s(%ld), Group: %s(%ld)",pw?pw->pw_name:"",(long)uid,
                                              gr?gr->gr_name:"",(long)gid);
      PutStr(3,cl+=2,s);

      if(syntax_hl::selector) {
	 PutStr(3,cl+=2,"Syntax selector:");
	 PutStr(3,++cl,syntax_hl::selector);
      }

      refresh();
   }
   while(WaitForKey(1000)==ERR);

   flushinp();

   CloseWin();
   DestroyWin(InfoWin);
}

void  UserToLineNumber()
{
   static char nl[10]="";
   if(getstring("Move to line: ",nl,sizeof(nl)-1,NULL,NULL,NULL)<1)
      return;
   GoToLineNum(strtol(nl,0,0)-1);
   SetStdCol();
}
void  UserToOffset()
{
   static char no[40]="";
   if(getstring("Move to offset: ",no,sizeof(no)-1,NULL,NULL,NULL)<1)
      return;
   CurrentPos=strtol(no,0,0);
   SetStdCol();
}

void  UserIndent()
{
   /* #### what exactly needs to be done when !insert ? */
   if(Text && stdcol>=GetCol() && Eol())
   {
      stdcol=(stdcol/IndentSize+1)*IndentSize;
      return;
   }
   num addcol=0;
   num newcol=(GetCol()/IndentSize+1)*IndentSize;
   offs ptr;
   for(ptr=Offset(); !EolAt(ptr) && (CharAt(ptr)==' ' || CharAt(ptr)=='\t'); ptr++);
   if(EolAt(ptr))
   {
      // space after cursor up to line end -- delete it
      DeleteBlock(0,ptr-Offset());
   }
   else if(insert)
   {
      // delete the space anyway, but remember how much needs to be reinserted
      addcol=TextPoint(ptr).Col()-GetCol();
      DeleteBlock(0,ptr-Offset());
   }
   if(Text && stdcol>=GetCol() && Eol())
   {
      stdcol=(stdcol/IndentSize+1)*IndentSize;
      return;
   }
   PreUserEdit();
   if(insert)
   {
      while(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
	 BackSpace();
   }
   while(GetCol()<newcol)
   {
      if(insert || Eol())
      {
         if(UseTabs && Tabulate(GetCol())<=newcol)
            InsertChar('\t');
         else
            InsertChar(' ');
      }
      else
      {
          MoveRight();
      }
   }
   TextPoint old=CurrentPos;
   while(addcol>0)
   {
      if(UseTabs && addcol>=TabSize-GetCol()%TabSize)
      {
	 addcol-=TabSize-GetCol()%TabSize;
	 InsertChar('\t');
      }
      else
      {
	 addcol--;
	 InsertChar(' ');
      }
   }
   CurrentPos=old;
   if(insert)
      flag|=REDISPLAY_LINE;
   SetStdCol();
}

void  UserNewLine()
{
   if(View)
      return;

   if(autoindent && !CheckPending())
      UserAutoindent();
   else
   {
      NewLine();
      SetStdCol();
      flag|=REDISPLAY_AFTER;
   }
}

void  UserAutoindent()
{
   if(View)
      return;

   num oldcol=GetCol();
   if(Text && Eol() && oldcol<stdcol)
      oldcol=stdcol;

   bool do_indent=true;

   if(MarginSizeAt(Offset())==-1)
      DeleteToBOL();
   else
   {
      offs ptr;
      for(ptr=Offset(); !EolAt(ptr) && (CharAt(ptr)==' ' || CharAt(ptr)=='\t'); ptr++)
	 ;
      if(EolAt(ptr))
         DeleteToEOL();
      else
         do_indent=false;
   }

   NewLine();
   SetStdCol();
   flag|=REDISPLAY_AFTER;
   if(do_indent)
      InsertAutoindent(oldcol);
}

void  UserUndelete()
{
   if(View)
      return;
   Undelete();
   flag=REDISPLAY_ALL;
   SetStdCol();
}
void  UserUndo()
{
   if(View)
      return;
   if(!undo.Enabled())
   {
      UserUndelete();
      return;
   }
   undo.UndoGroup();
   flag=REDISPLAY_ALL;
}
void  UserRedo()
{
   if(View)
      return;
   undo.RedoGroup();
   flag=REDISPLAY_ALL;
   SetStdCol();
}
void  UserUndoStep()
{
   if(View)
      return;
   undo.UndoOne();
   flag=REDISPLAY_ALL;
}
void  UserRedoStep()
{
   if(View)
      return;
   undo.RedoOne();
   flag=REDISPLAY_ALL;
   SetStdCol();
}

void  UserInsertChar(char ch)
{
   if(View)
      return;
   if(Text && autoindent && ch=='}' && MarginSizeAt(Offset())==-1 && MarginSizeAt(PrevLine(Offset()))==stdcol)
   {
      const offs match = FindMatch(ch);
      const num indent = match>=0 ? MarginSizeAt(match) : stdcol-IndentSize;
      DeleteToBOL();
      stdcol=indent;
   }
   PreUserEdit();
   InsertChar(ch);

   if(wordwrap)
      WordWrapInsertHook();

   if(hex || Bol())
      flag|=REDISPLAY_AFTER;
   else
      flag|=REDISPLAY_LINE;
   SetStdCol();
}
void  UserReplaceChar(char ch)
{
   if(View)
      return;
   PreUserEdit();

   if(!hex && Eol())
      flag|=REDISPLAY_AFTER;

   if(buffer_mmapped || hex || !mb_mode)
      ReplaceCharMove(ch);
   else
   {
      InsertChar(ch);
      (void)MBCheckLeft();
      if(!MBCharInvalid)
	 DeleteChar();
   }

   if(!hex && Bol())
      flag|=REDISPLAY_AFTER;
   else
      flag|=REDISPLAY_LINE;
   SetStdCol();
}

void  UserInsertControlChar(char ch)
{
   if(View)
      return;
   PreUserEdit();
   if((hex && !insert) || buffer_mmapped)
   {
      if(!hex && (Eol() || Char()=='\n'))
	 flag|=REDISPLAY_AFTER;
      ReplaceCharMove(ch);
      if(!hex && Bol())
	 flag|=REDISPLAY_AFTER;
      else
	 flag|=REDISPLAY_LINE;
   }
   else
   {
      InsertChar(ch);
      if(hex || Bol())
	 flag|=REDISPLAY_AFTER;
      else
	 flag|=REDISPLAY_LINE;
   }
   SetStdCol();
}
void  UserInsertString(const char *s,int len)
{
   while(len-->0)
      UserInsertControlChar(*s++);
}
void UserInsertWChar(wchar_t ch)
{
   char buf[MB_CUR_MAX+1];
   int len=wctomb(buf,ch);
   if(len<=0)
      return;
   UserInsertString(buf,len);
}

void  UserEnterControlChar()
{
   int   key;

   if(View)
      return;

   attrset(STATUS_LINE_ATTR->n_attr);
   mvaddch(StatusLineY,COLS-2,'^');
   SetCursor();
   key=GetRawKey();
   if(key==ERR)
      return;
   UserInsertControlChar((char)key);
}

void  UserWordHelp()
{
   if(*GetWord())
      cmd(HelpCmd,0,1);
}

void  UserKeysHelp()
{
   Help("MainHelp"," Help on Keys ");
}

void  UserAbout()
{
   ShowAbout();
   move(LINES-1,COLS-1);
   GetNextAction();
   HideAbout();
}

void  UserRefreshScreen()
{
   reset_prog_mode();
   flushinp();
   RedisplayAll();
   refresh();
}

void  UserChooseChar()
{
   if(mb_mode && !hex)
      UserChooseWChar();
   else
      UserChooseByte();
}
void  UserChooseByte()
{
   int   ch=choose_ch();
   if(ch!=-1)
      UserInsertControlChar(ch);
}
void  UserChooseWChar()
{
   wchar_t ch=choose_wch();
   if(ch!=-1)
      UserInsertWChar(ch);
}

void  UserInsertCharCode()
{
   if(mb_mode && !hex)
      UserInsertWCharCode();
   else
      UserInsertByteCode();
}
void  UserInsertByteCode()
{
   if(View)
      return;
   int ch=getcode_char();
   if(ch!=-1)
      UserInsertControlChar(ch);
}
void  UserInsertWCharCode()
{
   wchar_t ch=getcode_wchar();
   if(ch!=-1)
      UserInsertWChar(ch);
}

static int base_editmode=-1;

void  UserSwitchInsertMode()
{
   insert=!insert;
}
void  UserSwitchHexMode()
{
   if(editmode==HEXM)
   {
      if(base_editmode==HEXM)
         editmode=EXACT;
      else
         editmode=base_editmode;
      SetStdCol();
   }
   else
   {
      base_editmode=editmode;
      editmode=HEXM;
   }
   if(editmode==-1)
      editmode=EXACT;
   flag=REDISPLAY_ALL;
   if(editmode==HEXM)
      ScreenTop=ScreenTop&~15;
   else
      ScreenTop=LineBegin(ScreenTop);
}
void  UserSwitchTextMode()
{
   if(editmode==TEXT)
   {
      if(base_editmode==TEXT)
         editmode=EXACT;
      else
         editmode=base_editmode;
   }
   else
   {
      base_editmode=editmode;
      editmode=TEXT;
   }
   if(editmode==-1)
      editmode=EXACT;
   flag=REDISPLAY_ALL;
   if(editmode==HEXM)
      ScreenTop=ScreenTop&~15;
   else
      ScreenTop=LineBegin(ScreenTop);
}

void  UserSwitchRussianMode()
{
   if(inputmode==RUSS)
      inputmode=LATIN;
   else
      inputmode=RUSS;
}
void  UserSwitchGraphMode()
{
   if(inputmode==GRAPH)
      inputmode=LATIN;
   else
      inputmode=GRAPH;
}
void  UserSwitchAutoindentMode()
{
   autoindent=!autoindent;
}

void  UserBlockPrefixIndent()
{
   if(View)
      return;

   if(DragMark)
      UserStopDragMark();

   if(!GetActionArgument("Prefix: "))
      return;

   PrefixIndent(ActionArgument,ActionArgumentLen);
   flag=REDISPLAY_ALL;
}

History	 ShellHistory;
History	 PipeHistory;

void  UserShellCommand()
{
   if(!GetActionArgument("Shell-Command: ",&ShellHistory))
      return;
   cmd(ActionArgument,/*save*/false,/*pause*/true);
}

void  UserPipeBlock()
{
   if(DragMark)
      UserStopDragMark();

   CheckBlock();
   if(hide || rblock || View)
      return;

   const char *filter=GetActionArgument("Pipe through: ",&PipeHistory);
   if(!filter)
      return;

   MessageSync("Piping...");

   PipeBlock(filter,TRUE,TRUE);
   flag=REDISPLAY_ALL;
}

void  UserYankBlock()
{
   if(DragMark)
      UserStopDragMark();

   if(View)
      return;
   MainClipBoard.PasteAndMark();
   OptionallyConvertBlockNewLines("yanked");
   flag=REDISPLAY_ALL;
}

void  UserStartDragMark()
{
   if(DragMark)
   {
      UserStopDragMark();
      return;
   }
   PreUserEdit();
   DragMark=new TextPoint(CurrentPos);
   if(hide)
      UserSetBlockBegin();
}
void  UserStopDragMark()
{
   if(!DragMark)
      return;
   delete DragMark;
   DragMark=0;
}

static TextPoint *mark_move_point;
static bool mark_move_top,mark_move_left;
static void pre_mark_move()
{
   if(hide)
   {
   was_hidden:
      flag=REDISPLAY_ALL;
      BlockEnd=BlockBegin=CurrentPos;
      mark_move_point=&BlockEnd;
      mark_move_top=mark_move_left=false;
      return;
   }
   if(rblock)
   {
      mark_move_top=(CurrentPos.Line()==BlockBegin.Line());
      if(!mark_move_top && CurrentPos.Line()!=BlockEnd.Line())
	 goto was_hidden;
      mark_move_left=(CurrentPos.Col()==BlockBegin.Col());
      if(!mark_move_left && CurrentPos.Col()!=BlockEnd.Col())
	 goto was_hidden;
   }
   else
   {
      if(CurrentPos==BlockBegin)
	 mark_move_point=&BlockBegin;
      else if(CurrentPos==BlockEnd)
	 mark_move_point=&BlockEnd;
      else
	 goto was_hidden;  // just do the same.
   }
}
static void post_mark_move()
{
   hide=false;
   PreUserEdit();
   if(rblock)
   {
      if(mark_move_left)
      {
	 if(mark_move_top)
	    BlockBegin=CurrentPos;
	 else
	 {
	    BlockBegin=TextPoint::ForcedLineCol(BlockBegin.Line(),CurrentPos.Col());
	    BlockEnd  =TextPoint::ForcedLineCol(CurrentPos.Line(),BlockEnd.Col());
	 }
      }
      else
      {
	 if(mark_move_top)
	 {
	    BlockBegin=TextPoint::ForcedLineCol(CurrentPos.Line(),BlockBegin.Col());
	    BlockEnd  =TextPoint::ForcedLineCol(BlockEnd.Line(),CurrentPos.Col());
	 }
	 else
	    BlockEnd=CurrentPos;
      }
      // swap the points if needed.
      if(BlockBegin.Col()>BlockEnd.Col() && BlockBegin.Line()>=BlockEnd.Line())
	 goto swap_marks;
      if(BlockBegin.Col()>BlockEnd.Col())
      {
	 TextPoint tmp=BlockBegin;
	 BlockBegin=TextPoint::ForcedLineCol(BlockBegin.Line(),BlockEnd.Col());
	 BlockEnd  =TextPoint::ForcedLineCol(BlockEnd.Line(),tmp.Col());
      }
      else if(BlockBegin.Line()>BlockEnd.Line())
      {
	 TextPoint tmp=BlockBegin;
	 BlockBegin=TextPoint::ForcedLineCol(BlockEnd.Line(),BlockBegin.Col());
	 BlockEnd  =TextPoint::ForcedLineCol(tmp.Line(),BlockEnd.Col());
      }
   }
   else
   {
      flag=REDISPLAY_ALL;
      *mark_move_point=CurrentPos;
      if(BlockBegin>BlockEnd)
      {
      swap_marks:
	 TextPoint tmp=BlockBegin;
	 BlockBegin=BlockEnd;
	 BlockEnd=tmp;
      }
   }
}

#define MarkMove(move)	   \
   void UserMark##move()   \
   {			   \
      pre_mark_move();	   \
      hide=1;		   \
      User##move();	   \
      SeekStdCol();	   \
      post_mark_move();	   \
   }
MarkMove(CharLeft);
MarkMove(CharRight);
MarkMove(WordLeft);
MarkMove(WordRight);
MarkMove(LineBegin);
MarkMove(LineEnd);
MarkMove(FileBegin);
MarkMove(FileEnd);
MarkMove(PageDown);
MarkMove(PageUp);
MarkMove(PageTop);
MarkMove(PageBottom);
MarkMove(LineUp);
MarkMove(LineDown);

void UserOptimizeText()
{
   if(View || buffer_mmapped)
      return;

   offs     ptr;
   TextPoint  tp=CurrentPos;

   MessageSync("Optimizing...");
   bool at_indent=true;
   num col=0;
   for(ptr=0; !EofAt(ptr); ptr++)
   {
      byte ch=CharAt_NoCheck(ptr);
      if(ch!=' ' && ch!='\t')
	 at_indent=false;
      if(at_indent && ch=='\t') {
	 // optimize indentation (space+tab) => (tab)
	 while(ptr>0 && CharAt_NoCheck(ptr-1)==' ') {
	    int spaces_in_the_tab=col%TabSize;
	    int spaces_to_remove=spaces_in_the_tab?spaces_in_the_tab:TabSize;
	    CurrentPos=ptr;
	    DeleteBlock(spaces_to_remove,0);
	    ptr-=spaces_to_remove;
	    col-=spaces_to_remove;
	    if(spaces_in_the_tab==0) {
	       // we need to insert a tab to compensate for the spaces
	       InsertChar('\t');
	       MoveLeft();
	    }
// 	    assert(col==GetCol());
// 	    assert(ptr==CurrentPos);
	 }
      }
      if(EolAt(ptr))
      {
         CurrentPos=ptr;
         while(!Bol() && (CharRel_NoCheck(-1)==' ' || CharRel_NoCheck(-1)=='\t'))
	 {
            BackSpace();
	    ptr--;
	 }
	 ptr+=EolSize-1; // skip EOL at once
	 at_indent=true;
	 col=0;
      } else {
	 if(ch=='\t')
	    col=Tabulate(col);
	 else
	    col++;
      }
   }
   CurrentPos=TextEnd;
   while(!Bof() && Bol() && BolAt(Offset()-EolSize))
      DeleteBlock(EolSize,0);
   if(!Bol())
      NewLine();

   CurrentPos=tp;
   SetStdCol();
   flag=REDISPLAY_ALL;
}

void UserRememberBlock()
{
   if(DragMark)
      UserStopDragMark();

   MainClipBoard.Copy();
}

void UserSetBookmark()
{
   Message("Mark: ");
   move(LINES-1,6);
   curs_set(1);
   int key=getch();
   if(key<256 && key>=0)
      SetBookmark(key);
   else
      beep();
   ClearMessage();
}

void UserGoBookmark()
{
   Message("Go to mark: ");
   move(LINES-1,12);
   curs_set(1);
   int key=getch();
   if(key<256 && key>=0)
      GoBookmark(key);
   else
      beep();
   ClearMessage();
}

#define S(n) void UserSetBookmark##n() { SetBookmark('0'+n); }
S(0) S(1) S(2) S(3) S(4) S(5) S(6) S(7) S(8) S(9)
#define G(n) void UserGoBookmark##n() { GoBookmark('0'+n); }
G(0) G(1) G(2) G(3) G(4) G(5) G(6) G(7) G(8) G(9)
