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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "edit.h"

History::History()
{
   curr=-1;
}
void  History::Open()
{
   curr=-1;
}
HistoryLine *History::Curr()
{
   if(curr==-1)
      return(NULL);
   else
   {
      if(lines[curr].len==0)
      {
         curr=-1;
         return(NULL);
      }
      return(lines+curr);
   }
}
HistoryLine *History::Prev()
{
   if(curr==-1)
      curr=0;
   else
   {
      curr++;
      if(curr==HISTORY_SIZE)
         curr=-1;
   }
   return(Curr());
}
HistoryLine *History::Next()
{
   if(curr==-1)
   {
      curr=HISTORY_SIZE;
      while(lines[--curr].len==0 && curr>0);
   }
   else
      curr--;
   return(Curr());
}
void  History::operator+=(const HistoryLine& hl)
{
   int   i;

   if(hl.len==0)
      return;

   for(i=0; i<HISTORY_SIZE-1 && hl!=lines[i]; i++);
   for( ; i>0; i--)
      lines[i]=lines[i-1];
   lines[0]=hl;
}
void  History::operator-=(const HistoryLine& hl)
{
   int   i;

   if(hl.len==0)
      return;

   for(i=0; hl!=lines[i]; i++)
      if(i>=HISTORY_SIZE-1)
         return;
   for( ; i<HISTORY_SIZE-1; i++)
      lines[i]=lines[i+1];
   lines[i]=HistoryLine();
}
void  History::Push()
{
   HistoryLine hl2=lines[2];
   HistoryLine hl1=lines[1];
   *this+=hl2;
   *this+=hl1;
}

HistoryLine::HistoryLine()
{
   len=0;
   memset(line,0,sizeof(line));
   cr_time=0;
}
HistoryLine::HistoryLine(char *s,unsigned short l)
{
   if(l==0)
      l=strlen(s);
   memcpy(line,s,len=l);
   memset(line+l,0,sizeof(line)-l);
   time(&cr_time);
}
const HistoryLine&   HistoryLine::operator=(const HistoryLine& hl)
{
   len=hl.len;
   memcpy(line,hl.line,sizeof(line));
   cr_time=hl.cr_time;
   return(hl);
}
int   HistoryLine::operator!=(const HistoryLine& hl) const
{
   return(len!=hl.len || memcmp(line,hl.line,len));
}

void  History::Merge(const History& add)
{
   const HistoryLine *l1=this->lines;
   const HistoryLine *l2=add.lines;
   History  newh;

   int   i,j;
   for(i=HISTORY_SIZE,j=HISTORY_SIZE; i>0 || j>0; )
   {
      if(j==0 || (i!=0 && l1[i-1].cr_time<l2[j-1].cr_time))
         newh+=l1[--i];
      else
         newh+=l2[--j];
   }
   *this=newh;
}

void  History::WriteTo(FILE *f)
{
   for(int i=0; i<HISTORY_SIZE; i++)
   {
      fprintf(f,"%10lu%4u",(unsigned long)lines[i].cr_time,lines[i].len);
      fwrite(lines[i].line,1,lines[i].len,f);
      fputc('\n',f);
   }
}
void  History::ReadFrom(FILE *f)
{
   char  buf[15];
   for(int i=0; i<HISTORY_SIZE; i++)
   {
      lines[i].len=0;
      fread(buf,1,sizeof(buf)-1,f);
      buf[sizeof(buf)-1]=0;
      unsigned long cr_time;
      if(sscanf(buf,"%10lu%4hu",&cr_time,&(lines[i].len))<2)
         return;
      lines[i].cr_time=cr_time;
      if(lines[i].len>sizeof(lines[i].line))
      {
         lines[i].len=0;
         return;
      }
      fread(lines[i].line,1,lines[i].len,f);
      fgetc(f);
   }
}

InodeInfo::InodeInfo()
{
   time=size=device=inode=line=col=offset=0;
   cr_time=0;
}
InodeInfo::InodeInfo(struct stat *st,num l,num c,num o)
{
   time=st->st_mtime;
   size=st->st_size;
   device=st->st_dev;
   inode=st->st_ino;
   line=l;
   col=c;
   offset=o;
   ::time(&cr_time);
}
int   InodeInfo::SameFile(const InodeInfo& file) const
{
#ifndef BROKEN_INODES
   return(device==file.device && inode==file.inode);
#else
   return(time==file.time && size==file.size);
#endif
}
int   InodeInfo::SameFileModified(const InodeInfo& file) const
{
   return(SameFile(file) && (size!=file.size || time!=file.time));
}

InodeInfo   *InodeHistory::FindInode(const InodeInfo& file)
{
   for(int i=0; i<HISTORY_SIZE; i++)
      if(files[i].SameFile(file) && !files[i].SameFileModified(file))
         return(files+i);
   return(NULL);
}
void  InodeHistory::operator+=(const InodeInfo& file)
{
   InodeInfo   *find=FindInode(file);
   if(find==NULL)
      memmove(files+1,files,sizeof(files)-sizeof(files[0]));
   else
      memmove(files+1,files,sizeof(files[0])*(find-files));
   files[0]=file;
}
void  InodeHistory::WriteTo(FILE *f)
{
   int i=0;
   for(;;)
   {
      fprintf(f,"%10ld %ld,%ld,%ld,%ld,%ld,%ld,%ld\n",(long)files[i].cr_time,
         (long)files[i].inode,(long)files[i].device,(long)files[i].time,
         (long)files[i].size,(long)files[i].line,(long)files[i].col,
	 (long)files[i].offset);
      if(++i>=HISTORY_SIZE)
         break;
   }
}
void  InodeHistory::ReadFrom(FILE *f)
{
   long  inode,device,time,size,line,col,cr_time,offset;
   int i=0;
   for(;;)
   {
      if(fscanf(f,"%10ld %ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
		  &cr_time,&inode,&device,&time,&size,&line,&col,&offset)<7)
         break;
      files[i].inode=inode;
      files[i].device=device;
      files[i].time=time;
      files[i].size=size;
      files[i].line=line;
      files[i].col=col;
      files[i].offset=offset;
      files[i].cr_time=cr_time;
      if(++i>=HISTORY_SIZE)
         break;
   }
}

void  InodeHistory::Merge(const InodeHistory& add)
{
   const InodeInfo *l1=this->files;
   const InodeInfo *l2=add.files;
   InodeHistory  newh;

   int   i,j;
   for(i=HISTORY_SIZE,j=HISTORY_SIZE; i>0 || j>0; )
   {
      if(j==0 || (i!=0 && l1[i-1].cr_time<l2[j-1].cr_time))
         newh+=l1[--i];
      else
         newh+=l2[--j];
   }
   memcpy(this,&newh,sizeof(newh));
}
