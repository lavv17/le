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
#include   <assert.h>
#include   <limits.h>
#include   "keymap.h"
#include   "edit.h"
#include   "mb.h"
#include   "undo.h"

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
num       stdcol;    /* column to put cursor at in "Text" mode */
num       ScrShift=0;
int       modified=0;
bool      newfile=false;

int       EolSize=1;
char	  EolStr[3]="\n";

InodeInfo   FileInfo;
InodeHistory PositionHistory;
History    LoadHistory;

mode_t   FileMode;

int       file=(-1);  /* file descriptor */

char      FileName[256];

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

char  bak[BACKUP_SUFFIX_LEN]=".~%d~";

bool  buffer_mmapped=false;

/*____________________________________________________________________________
*/

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
   if(Text && stdcol!=NO_POS && Eol() && !in_hex_mode)
   {
      num i=GetCol();
      num j=stdcol;
      int oldmod=modified;
      if(UseTabs && i<j)
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
   if(todelete>0)
   {
      DeleteBlock(0,todelete);
      oldptr2=ptr2;
      flag|=REDISPLAY_LINE;
   }
   modified=om;
   SetStdCol();
}

void   MoveLeftOverEOL()
{
   if(in_hex_mode)
   {
      CurrentPos-=1;
      return;
   }
   if(Bol())
      CurrentPos-=EolSize;
   else
   {
      CurrentPos-=CharSizeLeft();
   }
}

void   MoveRightOverEOL()
{
   if(in_hex_mode)
   {
      CurrentPos+=1;
      return;
   }
   if(Eol())
      CurrentPos+=EolSize;
   else
      CurrentPos+=CharSize();
}

void   MoveUp()
{
   if(CurrentPos.Line()==0)
      CurrentPos=TextBegin;
   else
      CurrentPos=TextPoint(CurrentPos.Line()-1,GetStdCol());
}

void   MoveDown()
{
   CurrentPos=TextPoint(CurrentPos.Line()+1,GetStdCol());
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
   CalculateLine - determines line in the target point based
   on the line number in the source point
*/
void  CalculateLine(num *line_ptr,offs source,offs target)
{
   num line=*line_ptr;
   while(source>target)
   {
      if(BolAt(source))
	 --line;
      --source;
   }
   while(source<target)
   {
      ++source;
      if(BolAt(source))
	 ++line;
   }
   *line_ptr=line;
}

int   InsertBlock(const char *block_left,num size_left,const char *block_right,num size_right)
{
   if(buffer_mmapped)
   {
      return ERR;
   }

   num   i;
   offs  oldoffset;
   num   num_of_lines,oldline;
   num	 num_of_lines_curr;
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

   if(undo.Enabled())
      undo.AddChange(new Undo::Insert(block_left,size_left,block_right,size_right));

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
   oldline=GetLine();
   ptr1+=size_left;
   ptr2-=size_right;
   GapSize-=size;

   CalculateLine(&num_of_lines,oldoffset,oldoffset+size_left);
   num_of_lines_curr=num_of_lines;
   CalculateLine(&num_of_lines,oldoffset+size_left,oldoffset+size);

   join_at=-1;
   for(i=1; i<EolSize; i++)
   {
      if(BolAt(oldoffset+size+i))
      {
         join_at=i;
         break;
      }
   }
   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
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
	 if(!(scan->flags&LINEUNDEFINED))
	 {
	    scan->line+=num_of_lines_curr;
	    scan->flags|=COLUNDEFINED;
	 }
         scan->offset+=size_left;
      }
   }
   TextPoint::CheckSplit(Offset()-size_left-MB_LEN_MAX+1,Offset()+size_right+MB_LEN_MAX-1);

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

int   ReplaceTextFromFile(int fd,num size,num *act_read)
{
   // buffer should be clear
   assert(Size()==0);

   if(buffer_mmapped)
      return ERR;

   if(size==0)
   {
      *act_read=0;
      return(OK);
   }

   if(GetSpace(size)!=OK)
      return(ERR);

   *act_read=read(fd,buffer+ptr1,size);
   if(*act_read==-1)
      return(ERR);
   if(*act_read==0)
      return(OK);

   ptr1+=*act_read;
   GapSize-=*act_read;

   TextEnd=TextPoint(Size());

   return(OK);
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

int   GetBlock(char *copy,offs from,num size)
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
      return 0;

   if(from>=ptr1)
   {
      memcpy(copy,buffer+from+ptr2-ptr1,size);
      return(size);
   }
   if(from+size<=ptr1)
   {
      memcpy(copy,buffer+from,size);
      return(size);
   }
   num leftsize=ptr1-from;
   memcpy(copy,buffer+from,leftsize);
   memcpy(copy+leftsize,buffer+ptr2,size-leftsize);
   return(size);
}

// fight partial writes, count written bytes.
int write_loop(int fd,const char *ptr,num size,num *written)
{
   errno=0;
   while(size>0) {
      int res=write(fd,ptr,size);
      if(res==-1)
	 return ERR;
      if(res==0)
	 return ERR;
      ptr+=res;
      size-=res;
      *written+=res;
   }
   return OK;
}

int   WriteBlock(int fd,offs from,num size,num *act_written)
{
   *act_written=0;

   if(from<0)
   {
     size+=from;
     from=0;
   }
   if(from+size>Size())
     size=Size()-from;

   if(size<=0)
     return(OK);

   if(from>=ptr1)    // the region is completely on the right side
      return write_loop(fd,buffer+from+ptr2-ptr1,size,act_written);

   if(from+size<=ptr1)	// the region is completely on the left side
      return write_loop(fd,buffer+from,size,act_written);

   // the region is split by the gap
   num leftsize=ptr1-from;
   if(write_loop(fd,buffer+from,leftsize,act_written)!=OK)
      return ERR;
   return write_loop(fd,buffer+ptr2,size-leftsize,act_written);
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
   num   newline;
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

   if(undo.Enabled())
      undo.AddChange(new Undo::Delete(buffer+ptr1-left,left,buffer+ptr2,right));

   size=left+right;
   base=Offset()-left;

   TextPoint base_point(base);
   newline=base_point.Line();

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
   TextPoint::CheckSplit(Offset()-MB_LEN_MAX+1,Offset()+MB_LEN_MAX-1);

   modified=1;

   return(OK);
}

int   ReplaceBlock(const char *block,num size)
{
   if(!buffer_mmapped && !undo.Enabled() && !undo.Locked())
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

   num oldsize=size;
   if(base>Size()-oldsize)
      oldsize=Size()-base;

   if(buffer_mmapped)
   {
      if(size>oldsize)
	 size=oldsize;
      if(size<=0)
	 return ERR;
   }
   else
   {
      if(size>oldsize && GetSpace(size-oldsize)!=OK)
	 return ERR;
   }
   PreModify();

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

   int mb_size0=CharSizeLeftAt(base);

   num num_of_lines=0;
   offs o;
   for(o=base+1; o<=base+oldsize; o++)
      if(BolAt(o))
	 num_of_lines++;

   if(undo.Enabled())
      undo.AddChange(new Undo::Replace(buffer+ptr2,oldsize,block,size));

   memmove(buffer+ptr2-(size-oldsize),block,size);
   oldptr2=ptr2;
   ptr2-=(size-oldsize);

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

   int mb_size1=CharSizeLeftAt(base);
   int mb_size_max=(mb_size0>mb_size1?mb_size0:mb_size1);

   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
   {
      if(scan->offset>=base+size)
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
      else if(scan->offset+mb_size_max>base)
	 scan->flags|=COLUNDEFINED;
   }
   TextPoint::CheckSplit(Offset()-MB_LEN_MAX+1,Offset()+size+MB_LEN_MAX-1);

   // when mmapped, changes are committed to disk automatically.
   if(!buffer_mmapped)
      modified=1;

   return OK;
}

int   ReplaceCharMove(byte ch)
{
   int res=ReplaceChar(ch);
   if(res!=OK)
      return res;
   MoveRight();
   return OK;
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
   if(Eol() && !in_hex_mode)
     DeleteBlock(0,EolSize);
   else
     DeleteBlock(0,1);
}

void   DeleteLine()
{
   DeleteBlock(Offset()-LineBegin(Offset()),NextLine(Offset())-Offset());
   SetStdCol();
}

void   DeleteToEOL()
{
   DeleteBlock(0,LineEnd(Offset())-Offset());
}

void   DeleteToBOL()
{
   DeleteBlock(Offset()-LineBegin(Offset()),0);
}

offs   NextLine(offs ptr)
{
   char eol=EolStr[EolSize-1];
   const char *found;
   while(ptr<ptr1)
   {
      found=(const char*)memchr(buffer+ptr,eol,ptr1-ptr);
      if(!found)
      {
	 ptr=ptr1;
	 break;
      }
      ptr=found+1-buffer;
      if(BolAt(ptr))
	 return ptr;
   }
   offs size=Size();
   for(;;)
   {
      found=(const char*)memchr(buffer+ptr+GapSize,eol,size-ptr);
      if(!found)
	 return size;
      ptr=found+1-buffer-GapSize;
      if(BolAt(ptr))
	 return ptr;
   }
   /*NOTREACHED*/
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
   if(BofAt(ptr))
      return ptr;
   return LineBegin(ptr-1);
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
   undo.Clear();
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
   SetStdCol();
   ScrShift=0;
   View&=~TMP_RO_MODE;

   SetEolStr(EOL_UNIX);
}

bool IsAlNumAt(num o)
{
#if USE_MULTIBYTE_CHARS
   if(mb_mode)
      return iswalnum(WCharAt(o));
#endif
   return isalnum(CharAt(o)) || isrussian(CharAt(o));
}

char  *GetWord()
{
   static   char  word[256];
   num         shift=(-1);
   int         i;

   while(IsAlNumRel(shift) || CharRel(shift)=='.' || CharRel(shift)=='_')
     shift--;
   shift++;
   i=0;
   while(i<255 && (IsAlNumRel(shift) || CharRel(shift)=='.' || CharRel(shift)=='_'))
     word[i++]=CharRel(shift++);
   word[i]=0;
   return(word);
}

int   CountNewLines(offs start,num size,num *unix_nl,num *dos_nl,num *mac_nl)
{
   num	 i;
   num   unix_nl_store,dos_nl_store,mac_nl_store;

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
   if(!mac_nl)
      mac_nl=&mac_nl_store;

   *unix_nl=*dos_nl=*mac_nl=0;
   for(i=0; i<size; i++)
   {
      byte ch=CharAt_NoCheck(start+i);
      if(ch=='\r') {
	 (*mac_nl)++;
	 if(i+1<size && CharAt_NoCheck(start+i+1)=='\n') {
	    (*dos_nl)++;
	    (*unix_nl)++;
	    i++;
	 }
      } else if(ch=='\n') {
	 (*unix_nl)++;
      }
   }

   if(EolIs(EOL_UNIX))
      return *unix_nl;
   if(EolIs(EOL_MAC))
      return *mac_nl;
   return *dos_nl;
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

void  SeekStdCol()
{
   if(in_hex_mode || stdcol==NO_POS)
      return;
   CurrentPos=TextPoint(GetLine(),stdcol);
}

offs  ScanForCharForward(offs start,byte ch)
{
   char *pos;
   if(start<ptr1)
   {
      pos=(char*)memchr(buffer+start,ch,ptr1-start);
      if(pos)
	 return pos-buffer;
      start=ptr1;
   }
   if(start<Size())
   {
      pos=(char*)memchr(buffer+GapSize+start,ch,Size()-start);
      if(pos)
	 return pos-buffer-GapSize;
   }
   return -1;
}

void  InsertAutoindent(num oldcol)
{
   int   UseTabsNow=UseTabs;
   num   cnt;
   num   oldmargin;
   num   newmargin=0;

   if(View || !Bol())
      return;

   offs o=Offset()-EolSize;
   for(;;)
   {
      oldmargin=MarginSizeAt(o);
      if(BofAt(o) || oldmargin!=-1)
	 break;
      o=PrevLine(o);
   }
   if(TabsInMargin)
      UseTabsNow=1;

   if(oldmargin==oldcol)
      newmargin=oldmargin;
   else if(oldcol==0)
      newmargin=0;
   else if(oldmargin==-1)
      newmargin=oldcol/IndentSize*IndentSize;
   else
      newmargin=oldmargin;

   cnt=newmargin;
   if(Text && Eol() && !(!UseTabs && UseTabsNow))
   {
      stdcol=cnt;
      return;
   }
   if(UseTabsNow)
   {
       while(cnt>=TabSize)
       {
           cnt-=TabSize;
           InsertChar('\t');
       }
   }
   while(cnt>0)
   {
       cnt--;
       InsertChar(' ');
   }
   SetStdCol();
}

num GetCol()
{
   if(in_hex_mode)
      return 0;
   return(CurrentPos.Col());
}
bool EolAt(offs o)
{
   if(o>Size()-EolSize)
      return EofAt(o);
   if(EolSize==1)
      return CharAt_NoCheck(o)==EolStr[0];
   // assert(EolSize==2)
   return CharAt_NoCheck(o)==EolStr[0] && CharAt_NoCheck(o+1)==EolStr[1];
}
bool BolAt(offs o)
{
   if(o<EolSize)
      return BofAt(o);
   if(EolSize==1)
      return CharAt_NoCheck(o-1)==EolStr[0];
   // assert(EolSize==2)
   return CharAt_NoCheck(o-1)==EolStr[1] && CharAt_NoCheck(o-2)==EolStr[0];
}
bool Eol()
{
   return(EolAt(CurrentPos));
}
bool Bol()
{
   return(BolAt(CurrentPos));
}
void DeleteChar()
{
   DeleteBlock(0,CharSize());
}
void BackSpace()
{
   DeleteBlock(CharSizeLeft(),0);
}
int InsertChar(char ch)
{
   return(InsertBlock(&ch,1));
}
int ReplaceChar(char ch)
{
   return ReplaceBlock(&ch,1);
}

void SetEolStr(const char *n)
{
   EolSize=strlen(n);
   if(EolSize>int(sizeof(EolStr)-1))
      EolSize=sizeof(EolStr)-1;
   memcpy(EolStr,n,EolSize+1);
}

bool BlockEqAt(offs o,const char *s,int len)
{
   if(o+len>Size() || o<0)
      return false;
   if(o>=ptr1)
      return !memcmp(buffer+GapSize+o,s,len);
   if(o+len<=ptr1)
      return !memcmp(buffer+o,s,len);
   int len1=ptr1-o;
   return !memcmp(buffer+o,s,len1)
       && !memcmp(buffer+GapSize,s+len1,len-len1);
}

offs FindMatch(const char op)
{
   byte cl;
   int dir;
   offs ptr=CurrentPos;
   int level = 0;

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
      return -2;
   }
   while(!((dir>0)?EofAt(ptr):BofAt(ptr)))
   {
      ptr+=dir;
      if(CharAt(ptr)==op)
         level++;
      else if(CharAt(ptr)==cl)
      {
         if(level==0)
            return ptr;
         else
            level--;
      }
   }
   return -1;
}
