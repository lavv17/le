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

#include <config.h>
#include   <stdio.h>
#include   <ctype.h>
#include   <string.h>
#include   <errno.h>
#include   <fcntl.h>
#include   <sys/types.h>
#include   <sys/stat.h>
#ifdef HAVE_SYS_MMAN_H
#include   <sys/mman.h>
#endif
#include   <unistd.h>
#include   <time.h>
#include   <stdlib.h>
#include   "keymap.h"
#include   "edit.h"

#ifndef O_NDELAY
#define O_NDELAY 0
#endif

#define    START_GAP   (10*MemStep)

int       flag=TRUE;
char      *buffer=NULL;
offs      BufferSize,GapSize;
offs      ptr1,ptr2;
offs      oldptr1,oldptr2;
int       oldmodified;	/* status of text if ptr1 and ptr2 restored */
num       stdcol;
num       ScrShift=0;
int       modified=0;
int       newfile=0;

int       DosEol=0;
int       EolSize=1;
char      *EolStr="\n";

InodeInfo   FileInfo;
InodeHistory PositionHistory;
History    LoadHistory;

mode_t   FileMode;

int       file=(-1);  /* file descriptor */

char      FileName[256];
char      InitName[256];

int       TabSize=8;
int       autoindent=TRUE;
int       insert=TRUE;
int       makebak=FALSE;
int       Scroll=1;
int       hscroll=8;
int       rblock=FALSE;

int       right=FALSE;
int       ascii=FALSE;

int       editmode=EXACT,inputmode=LATIN;
int       noreg=FALSE;

int   FileTypeDetect=1;

int   TabsInMargin;

char  bak[5]="~%d~";

bool  buffer_mmapped=false;

/*____________________________________________________________________________
*/
int   StringCompare(offs o,char *str,num len)
{
/* this works only for len<=2 */
   return(CharAt(o)==str[0] && (len<2 || CharAt(o+1)==str[1]));
/* ... and this for any len, but it is slooower :-) */
/*   if(o<0 || o+len>Size())
      return(0);

   num   left=ptr1-o;
   if(left<=0)
      return(!memcmp(buffer+ptr2-left,str,len));
   num   right=len-left;
   if(right<=0)
      return(!memcmp(buffer+o,str,len));
   return(!memcmp(buffer+o,str,left) && !memcmp(buffer+ptr2,str+left,right));*/
}

void   PreModify(void)
{
   num   shift=Offset()-ptr1;
   if(shift>0)
   {
      memmove(buffer+ptr1,buffer+ptr2,shift);
      oldptr1=ptr1+=shift;
      oldptr2=ptr2+=shift;
   }
   else if(shift<0)
   {
      memmove(buffer+ptr2+shift,buffer+ptr1+shift,-shift);
      oldptr1=ptr1+=shift;
      oldptr2=ptr2+=shift;
   }
}

int PreUserEdit()
{
   if(buffer_mmapped)
      return 1;
   if(Text && Eol() && !hex)
   {
      num i=GetCol();
      num j=stdcol;
      int oldmod=modified;
      if(UseTabs)
   	 for( ; Tabulate(i)<=j; i=Tabulate(i))
            if(InsertChar('\t')!=OK)
	       return 0;
      while(i++<j)
	 if(InsertChar(' ')!=OK)
	    return 0;
      modified=oldmod;
   }
   return 1;
}

offs   LineBegin(offs ptr)
{
   while(!BolAt(ptr))
      ptr--;
   return(ptr);
}
offs   LineEnd(offs ptr)
{
   while(!EolAt(ptr))
      ptr++;
   return(ptr);
}

void   ToLineBegin()
{
   CurrentPos=LineBegin(CurrentPos.Offset());
}
void   ToLineEnd()
{
   offs   ptr=Offset();
   int    om;
   num  todelete;

   while(!EolAt(ptr))
      ptr++;
   todelete=ptr;
   if(Text)
   {
      while(!BolAt(ptr) && (CharAt(ptr-1)==' ' || CharAt(ptr-1)=='\t')
           && (hide || ptr!=BlockEnd.Offset()))
         ptr--;
   }
   todelete-=ptr;
   CurrentPos=ptr;
   om=modified;
   DeleteBlock(0,todelete);
   if(todelete>0)
      oldptr2=ptr2;
   modified=om;
   stdcol=GetCol();
}

void   MoveLeftOverEOL()
{
   if(Bol() && !hex)
      CurrentPos-=EolSize;
   else
      CurrentPos-=1;
}

void   MoveRightOverEOL()
{
   if(Eol() && !hex)
      CurrentPos+=EolSize;
   else
      CurrentPos+=1;
}

void   MoveUp()
{
   if(CurrentPos.Line()==0)
      CurrentPos=TextBegin;
   else
      CurrentPos=TextPoint(CurrentPos.Line()-1,stdcol);
}

void   MoveDown()
{
   CurrentPos=TextPoint(CurrentPos.Line()+1,stdcol);
}


/* MarginSizeAt returns column of the first non-blank character
  on the line specified by pos. If the line is empty (or consists
  of blanks only) -1 is returned. */
num    MarginSizeAt(offs pos)
{
   num    margin=0;

   TabsInMargin=0;

   pos=LineBegin(pos);

   while(!EolAt(pos))
   {
      if(CharAt(pos)=='\t')
      {
         TabsInMargin=1;
         margin=Tabulate(margin);
      }
      else if(CharAt(pos)==' ')
         margin++;
      else
         return(margin);
      pos++;
   }
   return(-1);
}

int    GetSpace(num s)
{
   if(buffer_mmapped)
      return ERR;

   char   *nb;
   num    _add;
   offs   nptr1=ptr1;
   offs   nptr2=ptr2;

   if(nptr1<oldptr1)
      nptr1=oldptr1;
   if(nptr2>oldptr2)
      nptr2=oldptr2;

   if(nptr2-nptr1>=s)
     return(OK);

   _add=((s-nptr2+nptr1+MemStep-1)/MemStep+1)*MemStep;

   if((nb=(char*)realloc(buffer,BufferSize+_add))==NULL)
   {
     NotMemory();
     return(ERR);
   }
   memmove(nb+nptr2+_add,nb+nptr2,BufferSize-nptr2);
   ptr2+=_add;
   oldptr2+=_add;
   BufferSize+=_add;
   GapSize+=_add;
   buffer=nb;
   return(OK);
}

/*
   CalculateLineCol - determines line and column in target point basing
   on line and col in source point
*/
void  CalculateLineCol(num *line,num *col,offs source,offs target)
{
   num	 bol_point;

   if(source>target)
   {
      for( ; source>target; source--)
      {
	 if(BolAt(source))
	 {
	    (*line)--;
	    *col=-1;
	 }
	 else if(*col!=-1)
	 {
	    if(CharAt_NoCheck(source-1)=='\t')
	       *col=-1;
	    else
	       (*col)--;
	 }
      }
   }
   if(*col==-1)
   {
      source=LineBegin(source);
      *col=0;
   }
   for(bol_point=source++; source<=target; source++)
   {
      if(BolAt(source))
      {
	 (*line)++;
	 bol_point=source;
	 *col=0;
      }
   }
   for(source=bol_point; source<target; source++)
   {
      if(CharAt_NoCheck(source)=='\t')
         *col=Tabulate(*col);
      else
         (*col)++;
   }
}

int   InsertBlock(char *block_left,register num size_left,char *block_right,num size_right)
{
   if(buffer_mmapped)
   {
      return ERR;
   }

   register num   i;
   register offs  oldoffset;
   num   num_of_lines,num_of_columns,oldline,oldcol;
   num	 num_of_lines_curr,num_of_columns_curr;
   int   break_at;
   int   join_at;
   num	 size;
   int	 new_oldmodified;

   size=size_left+size_right;

   if(size==0)
      return(OK);

   PreModify();

   if(size_left>0)
   {
      if(oldptr1>ptr1)
	 oldptr1=ptr1;
   }
   if(ptr2>oldptr2 && size_right>0)
   {
      if(ptr2-oldptr2>size_right || memcmp(buffer+ptr2-size_right,block_right,size_right))
   	 oldptr2=ptr2;
   }

   if(GetSpace(size_left+size_right)!=OK)
      return(ERR);

   if(oldptr1==ptr1 && oldptr2==ptr2)
      new_oldmodified=modified;
   else
      new_oldmodified=oldmodified;

   memmove(buffer+ptr1,block_left,size_left);
   memmove(buffer+ptr2-size_right,block_right,size_right);

   oldoffset=Offset();

   break_at=-1;
   for(i=1; i<EolSize; i++)
   {
      if(BolAt(oldoffset+i))
      {
         break_at=i;
         break;
      }
   }
   num_of_lines=0;
   oldcol=num_of_columns=GetCol();
   oldline=GetLine();
   ptr1+=size_left;
   ptr2-=size_right;
   GapSize-=size;

   CalculateLineCol(&num_of_lines,&num_of_columns,oldoffset,oldoffset+size_left);
   num_of_lines_curr=num_of_lines;
   num_of_columns_curr=num_of_columns;
   CalculateLineCol(&num_of_lines,&num_of_columns,oldoffset+size_left,oldoffset+size);

   join_at=-1;
   for(i=1; i<EolSize; i++)
   {
      if(BolAt(oldoffset+size+i))
      {
         join_at=i;
         break;
      }
   }
   for(register TextPoint *scan=TextPoint::base; scan; scan=scan->next)
   {
      if(scan->offset>oldoffset || scan==&TextEnd)
      {
         if(!(scan->flags&LINEUNDEFINED))
         {
            if(scan->line==oldline)
               scan->flags|=COLUNDEFINED;
            scan->line+=num_of_lines;
            if(break_at>0 && scan->offset>=oldoffset+break_at)
               scan->line--;
            if(join_at>0  && scan->offset>=oldoffset+join_at)
               scan->line++;
         }
         scan->offset+=size;
      }
      else if(scan==&CurrentPos)
      {
         scan->col=num_of_columns_curr;
         scan->line+=num_of_lines_curr;
         scan->offset+=size_left;
	 scan->flags&=~(COLUNDEFINED|LINEUNDEFINED);
      }
   }

   stdcol=GetCol();
   if(oldptr1!=ptr1 || oldptr2!=ptr2)
   {
      oldmodified=new_oldmodified;
      modified=1;
   }
   else
   {
      modified=oldmodified;
   }

   return(OK);
}

int   CopyBlock(offs from,num size)
{
   if(buffer_mmapped)
   {
      return ERR;
   }

   PreModify();

   if(from<0)
   {
     size+=from;
     from=0;
   }
   if(from+size>Size())
   {
     size=Size()-from;
   }
   if(size<=0)
     return(OK);

   if(GetSpace(size)!=OK)
      return(ERR);

   if(from>=ptr1)
     return(InsertBlock(buffer+from+ptr2-ptr1,size));
   if(from+size<=ptr1)
     return(InsertBlock(buffer+from,size));
   return(InsertBlock(buffer+from,ptr1-from,buffer+ptr2,from+size-ptr1));
}

int   CopyBlockOver(offs from,num size)
{
   if(from<0)
   {
     size+=from;
     from=0;
   }
   if(from+size>Size())
   {
     size=Size()-from;
   }
   if(size<=0)
     return(OK);

   if(buffer_mmapped)
   {
      int res=ReplaceBlock(buffer+from,size);
      if(res==OK)
	 CurrentPos+=size;
      return res;
   }
   if(CopyBlock(from,size)==OK)
      return DeleteBlock(0,size);
   return ERR;
}

int   ReadBlock(int fd,num size,num *act_read)
{
   if(buffer_mmapped)
   {
      return ERR;
   }

   if(size==0)
   {
      *act_read=0;
      return(OK);
   }

   PreModify();

   if(GetSpace(size)!=OK)
      return(ERR);

   *act_read=read(fd,buffer+ptr1,size);
   if(*act_read==-1)
      return(ERR);
   if(*act_read==0)
      return(OK);
   return(InsertBlock(buffer+ptr1,*act_read));
}

int   ReadBlockOver(int fd,num size,num *act_read)
{
   if(buffer_mmapped)
   {
      if(size>Size()-Offset())
	 size=Size()-Offset();
   }
   if(size==0)
   {
      *act_read=0;
      return(OK);
   }
   if(buffer_mmapped)
   {
      char *buf=(char*)malloc(size);
      if(buf==0)
      {
	 NotMemory();
	 return ERR;
      }
      *act_read=read(fd,buf,size);
      int res=OK;
      if(*act_read==-1)
	 res=ERR;
      if(res==OK && *act_read>0)
	 res=ReplaceBlock(buf,*act_read);
      if(res==OK)
	 CurrentPos+=*act_read;
      free(buf);
      return res;
   }
   if(ReadBlock(fd,size,act_read)==OK)
      return DeleteBlock(0,*act_read);
   return ERR;
}

int   WriteBlock(int fd,offs from,num size,num *act_written)
{
   num   leftsize;

   if(from<0)
   {
     size+=from;
     from=0;
   }
   if(from+size>Size())
   {
     size=Size()-from;
   }
   if(size<=0)
   {
     *act_written=0;
     return(OK);
   }

   if(from>=ptr1)
   {
      *act_written=write(fd,buffer+from+ptr2-ptr1,size);
      if(*act_written==-1)
      {
         *act_written=0;
         return(ERR);
      }
      return(OK);
   }
   if(from+size<=ptr1)
   {
      *act_written=write(fd,buffer+from,size);
      if(*act_written==-1)
      {
         *act_written=0;
         return(ERR);
      }
      return(OK);
   }
   leftsize=ptr1-from;
   *act_written=write(fd,buffer+from,leftsize);
   if(*act_written==-1)
   {
      *act_written=0;
      return(ERR);
   }
   if(*act_written<leftsize)
      return(OK);
   *act_written=write(fd,buffer+ptr2,size-leftsize);
   if(*act_written==-1)
   {
      *act_written=leftsize;
      return(ERR);
   }
   *act_written+=leftsize;
   return(OK);
}

int   DeleteBlock(num left,num right)
{
   if(buffer_mmapped)
   {
      return ERR;
   }

   offs  base;
   num   size;
   num   i;
   num   newcol,newline;
   num   num_of_lines;
   int   join_at;
   int   break_at;

   if(left>Offset())
      left=Offset();
   if(right>Size()-Offset())
      right=Size()-Offset();

   if(left==0 && right==0)
     return(OK);

   PreModify();

   if(oldptr1<ptr1 && left>0)
      oldptr1=ptr1;
   if(oldptr2>ptr2 && right>0)
      oldptr2=ptr2;

   if(oldptr1==ptr1 && oldptr2==ptr2)
      oldmodified=modified;

   size=left+right;
   base=Offset()-left;

   TextPoint base_point(base);
   newline=base_point.Line();
   newcol=base_point.Col();

   num_of_lines=TextPoint(base+size).Line()-newline;

   break_at=-1;
   for(i=1; i<EolSize; i++)
   {
      if(BolAt(base+size+i))
      {
         break_at=i;
         break;
      }
   }

   ptr1-=left;
   ptr2+=right;
   GapSize+=left+right;

   join_at=-1;
   for(i=1; i<EolSize; i++)
   {
     if(BolAt(base+i))
     {
       join_at=i;
       break;
     }
   }

   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
   {
      if(scan->offset>base+size)
      {
         scan->offset-=size;
         if(!(scan->flags&LINEUNDEFINED))
         {
            scan->line-=num_of_lines;
            if(break_at>0 && scan->offset>=base+break_at)
               scan->line--;
            if(join_at>0  && scan->offset>=base+join_at)
               scan->line++;
            if(scan->line==newline)
               scan->flags|=COLUNDEFINED;
         }
      }
      else if(scan->offset>=base)
      {
	 *scan=base_point;
      }
   }
   modified=1;

   return(OK);
}

int   ReplaceBlock(char *block,num size)
{
   if(!buffer_mmapped)
   {
      int res=InsertBlock(block,size);
      if(res==OK)
      {
	 res=DeleteBlock(0,size);
      	 CurrentPos-=size;
      }
      return res;
   }

   offs base=Offset();
   offs newline=GetLine();

   if(base>Size()-size)
   {
      size=Size()-base;
      if(size<=0)
	 return ERR;
   }

   int break_at=-1;
   int i;
   for(i=1; i<EolSize; i++)
   {
      if(BolAt(base+size+i))
      {
         break_at=i;
         break;
      }
   }

   num num_of_lines=0;
   offs o;
   for(o=base+1; o<=base+size; o++)
      if(BolAt(o))
	 num_of_lines++;

   memmove(buffer+base,block,size);

   for(o=base; o<base+size; o++)
      if(EolAt(o))
	 num_of_lines--;

   int join_at=-1;
   for(i=1; i<EolSize; i++)
   {
     if(BolAt(base+size+i))
     {
       join_at=i;
       break;
     }
   }

   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
   {
      if(scan->offset>base+size)
      {
         if(!(scan->flags&LINEUNDEFINED))
         {
            scan->line-=num_of_lines;
            if(break_at>0 && scan->offset>=base+size+break_at)
               scan->line--;
            if(join_at>0  && scan->offset>=base+size+join_at)
               scan->line++;
            if(scan->line==newline)
               scan->flags|=COLUNDEFINED;
         }
      }
      else if(scan->offset>base)
      {
	 scan->offset=base+size;
	 scan->flags|=COLUNDEFINED|LINEUNDEFINED;
      }
   }

   return OK;
}

int   ReplaceCharMove(byte ch)
{
   int res;
   if(!buffer_mmapped)
   {
      res=InsertChar(ch);
      if(res==OK)
	 DeleteChar();
   }
   else
   {
      res=ReplaceChar(ch);
      if(res==OK)
	 MoveRight();
   }
   return res;
}

int   Undelete()
{
   if(oldptr1==ptr1 && oldptr2==ptr2)
     return(OK);

   CurrentPos=ptr1;

   offs  oldoldptr1=oldptr1,oldoldptr2=oldptr2;
   int   newmodified=oldmodified;
   oldptr1=ptr1;
   oldptr2=ptr2;

   /* left delete, right delete */
   if(oldoldptr1>=ptr1 && oldoldptr2<=ptr2)
   {
      InsertBlock(buffer+ptr1,oldoldptr1-ptr1,
		  buffer+oldoldptr2,ptr2-oldoldptr2);
   }
   /* left insert, right insert */
   else if(oldoldptr1<=ptr1 && oldoldptr2>=ptr2)
   {
      DeleteBlock(ptr1-oldoldptr1,oldoldptr2-ptr2);
   }
   /* left delete, right insert */
   else if(oldoldptr1>ptr1 && oldoldptr2>ptr2)
   {
      InsertBlock(buffer+ptr1,oldoldptr1-ptr1,NULL,0);
//      DeleteBlock(0,oldoldptr2-ptr2);
      newmodified=1;
   }
   else if(oldoldptr1<ptr1 && oldoldptr2<ptr2)
   {
//      DeleteBlock(ptr1-oldoldptr1,0);
      newmodified=1;
      InsertBlock(NULL,0,buffer+oldoldptr2,ptr2-oldoldptr2);
   }

   modified=newmodified;

   return(OK);
}

void   DeleteEOL()
{
   if(Eol() && !hex)
     DeleteBlock(0,EolSize);
   else
     DeleteBlock(0,1);
}

void   DeleteLine()
{
   stdcol=0;
   DeleteBlock(Offset()-LineBegin(Offset()),NextLine(Offset())-Offset());
}

void   DeleteToEOL()
{
   DeleteBlock(0,LineEnd(Offset())-Offset());
}

offs   NextLine(offs ptr)
{
   while(!EofAt(ptr) && !BolAt(++ptr));
   return(ptr);
}
offs   NextNLines(offs ptr,num n)
{
   ptr=LineBegin(ptr);
   while(n-->0)
      ptr=NextLine(ptr);
   return(ptr);
}

offs   PrevLine(offs ptr)
{
   ptr=LineBegin(ptr);
   while(!BofAt(ptr) && !BolAt(--ptr));
   return(ptr);
}

offs   PrevNLines(offs ptr,num n)
{
   ptr=LineBegin(ptr);
   while(n--)
      ptr=PrevLine(ptr);
   return(ptr);
}

void  CheckPoint()
{
   oldptr1=ptr1;
   oldptr2=ptr2;
   oldmodified=modified;
}

void  EmptyText()
{
   remove(TmpFileName());

   if(FileName[0])
      LoadHistory+=HistoryLine(FileName,strlen(FileName));

   if(file!=-1)
   {
      struct stat st;
      close(file);
      file=-1;
      if(stat(FileName,&st)!=-1)
      {
	 if(newfile && st.st_size==0)
            remove(FileName);
	 else if(!modified)
	    SavePosition();
      }
   }
   FileName[0]=0;
   TextPoint::ResetTextPoints();

   if(buffer)
   {
#ifdef HAVE_MMAP
      if(buffer_mmapped)
      {
	 munmap(buffer,BufferSize);
      }
      else
#endif
      {
	 free(buffer);
      }
      buffer=NULL;
   }

   BufferSize=0;
   GapSize=0;
   oldptr1=ptr1=oldptr2=ptr2=0;
   oldmodified=modified=0;
   stdcol=0;
   ScrShift=0;

   DosEol=0;
   EolSize=1;
   EolStr="\n";
}

void   Optimize()
{
   offs     ptr;
   TextPoint  tp=CurrentPos;

   Message("Optimizing...");
   for(ptr=0; !EofAt(ptr); ptr++)
   {
      if(EolAt(ptr))
      {
         CurrentPos=ptr;
         while(!Bol() && (CharRel_NoCheck(-1)==' ' || CharRel_NoCheck(-1)=='\t'))
            BackSpace();
      }
   }
   CurrentPos=TextEnd;
   while(!Bof() && Bol() && BolAt(Offset()-EolSize))
      DeleteBlock(EolSize,0);
   if(!Bol())
      NewLine();

   CurrentPos=tp;
   stdcol=GetCol();
   flag=REDISPLAY_ALL;
}

char  *GetWord()
{
   static   char  word[256];
   num         shift=(-1);
   int         i;

   while(isalnum(CharRel(shift)) || CharRel(shift)=='.' || CharRel(shift)=='_')
     shift--;
   shift++;
   i=0;
   while(i<255 && (isalnum(CharRel(shift)) || CharRel(shift)=='.' || CharRel(shift)=='_'))
     word[i++]=CharRel(shift++);
   word[i]=0;
   return(word);
}

int   CountNewLines(offs start,num size,num *unix_nl,num *dos_nl)
{
   num	 i;
   num   unix_nl_store,dos_nl_store;

   if(start<0)
   {
      size+=start;
      start=0;
   }
   if(start>Size())
      start=Size();
   if(start+size>=Size())
      size=Size()-start;

   if(!unix_nl)
      unix_nl=&unix_nl_store;
   if(!dos_nl)
      dos_nl=&dos_nl_store;

   *unix_nl=*dos_nl=0;
   for(i=0; i<size; i++)
   {
      if(CharAt_NoCheck(start+i)=='\n')
      {
	 (*unix_nl)++;
	 if(i>0 && CharAt_NoCheck(start+i-1)=='\r')
	    (*dos_nl)++;
      }
   }

   return(DosEol?*dos_nl:*unix_nl);
}

void  ConvertFromDosToUnix(offs start,num size)
{
   while(size>1)
   {
      if(CharAt(start)=='\r' && CharAt(start+1)=='\n')
      {
	 CurrentPos=start;
	 DeleteChar();
	 size--;
      }
      size--;
      start++;
   }
}
void  ConvertFromUnixToDos(offs start,num size)
{
   while(size>0)
   {
      if(CharAt(start)=='\n')
      {
	 CurrentPos=start;
	 InsertChar('\r');
	 start++;
      }
      size--;
      start++;
   }
}
