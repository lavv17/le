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

/* $Id$ */

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "edit.h"
#include "keymap.h"
#include "highli.h"
#include <xalloca.h>
#include "block.h"
#include "clipbrd.h"
#include "getch.h"
#include "bm.h"

#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

#ifndef DISABLE_FILE_LOCKS
int    LockFile(int fd,bool drop)
{
   struct  flock   Lock;
   Lock.l_start=0;
   Lock.l_len=0;
   Lock.l_type=F_WRLCK;
   Lock.l_whence=SEEK_SET;

   if(fcntl(fd,F_SETLK,&Lock)==-1)
   {
      if(errno==EACCES || errno==EAGAIN)
      {
         struct flock   Lock1;
         char   msg[100];
         static  struct menu LockMenu[]={
         {" &Cancel ",MIDDLE-10,FDOWN-2},
         {"  &Wait  ",MIDDLE,FDOWN-2},
         {" &Ignore ",MIDDLE+10,FDOWN-2},
         {NULL}};
         static  struct menu LockMenu1[]={
         {" &Cancel ",MIDDLE-5,FDOWN-2},
         {"  &Wait  ",MIDDLE+5,FDOWN-2},
         {NULL}};
         struct  stat   st;
         Lock1=Lock;
         fcntl(fd,F_GETLK,&Lock1);
         if(Lock1.l_type==F_UNLCK)
         {
            return(-2);
         }
         fstat(fd,&st);

         sprintf(msg,"This file is already locked by prosess %ld",(long)Lock1.l_pid);
         switch(ReadMenuBox(LockEnforce(st.st_mode)?
            LockMenu1:LockMenu,HORIZ,msg," Lock Error ",
	    VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
         {
         case(0):
         case('C'):
            return(-1);
         case('I'):
            return(0);
         case('W'):
            MessageSync("Waiting for unlocking the file... (C-x - cancel)");
            errno=EACCES;
            while(fcntl(fd,F_SETLK,&Lock)==-1 && (errno==EACCES || errno==EAGAIN))
            {
	       if(WaitForKey(1000)!=ERR)
	       {
	       	  int action=GetNextAction();
		  if(action==CANCEL)
                  {
                     ErrMsg("Interrupted by user");
                     return(-1);
                  }
               }
            }
            if(errno!=EACCES && errno!=EAGAIN)
               return(-2);
         }
      }
      else
      {
         return(-2);
      }
   }
   if(drop)
   {
      Lock.l_type=F_RDLCK;	// drop write lock to read lock
      fcntl(fd,F_SETLK,&Lock);
   }
   return(0);
}
#else /* DISABLE_FILE_LOCKS */
int   LockFile(int,bool)
{
   return 0;
}
#endif /* DISABLE_FILE_LOCKS */

struct  menu   ConCan4Menu[]={
{   " C&ontinue ",MIDDLE-6,FDOWN-2  },
{   "  &Cancel  ",MIDDLE+6,FDOWN-2  },
{NULL}};

off_t  GetDevSize(int fd)
{
#ifdef BLKGETSIZE
   unsigned long sect=0;
   if(ioctl(fd,BLKGETSIZE,&sect)==0)
      return ((off_t)sect)<<9;
#endif

   off_t lower=0;
   off_t upper=0x10000;
   char buf[1024];

   for(;;)
   {
      off_t pos=lseek(fd,upper,SEEK_SET);
      if(pos!=upper)
	 break;
      int res=read(fd,buf,sizeof(buf));
      if(res<=0)
	 break;
      lower=upper;
      upper*=2;
   }
   for(;;)
   {
      if(upper<=lower)
	 break;
      off_t mid=(upper+lower)/2;
      off_t pos=lseek(fd,mid,SEEK_SET);
      if(pos!=mid)
      {
	 upper=mid;
	 continue;
      }
      int res=read(fd,buf,sizeof(buf));
      if(res>0)
	 lower=mid+res;
      else
	 upper=mid;
   }

   return upper;
}

int   LoadFile(char *name)
{
   struct stat    st;
   offs   i;
   num    act_read;
   char   msg[256];
   InodeInfo   *old;

   CheckBlock();
   if(!hide)
      MainClipBoard.Copy();

   EmptyText();
   ResetBookmarks();

   flag=REDISPLAY_ALL;

   errno=0;
   View&=~2;      /* clear the 'temporarily read-only' bit */

   if(!name[0])
   {
      buffer_mmapped=false;
      return(OK);
   }

   sprintf(msg,"Loading the file \"%.60s\"...",name);
   MessageSync(msg);

   newfile=0;

   if(stat(name,&st)==-1 && errno==ENOENT)
   {
      int f=creat(name,0644);
      if(f!=-1)
      {
	 close(f);
	 newfile=1;
      }
      else
      {
	 ErrMsg("Cannot create the file.\n"
		"The directory does not exist or is not accessible\n"
	        "or does not permit writing");
	 EmptyText();
	 return(ERR);
      }
   }
   else if(errno==0)
   {
      FileMode=st.st_mode;
      if((!buffer_mmapped && (S_ISBLK(FileMode) || S_ISCHR(FileMode)))
	 || S_ISFIFO(FileMode))
      {
	 ErrMsg("This is a special file or a pipe\nthat I cannot edit.");
	 EmptyText();
	 return(ERR);
      }
      if(S_ISDIR(FileMode))
	 View|=2;
   }
   file=open(name,(View?O_RDONLY:O_RDWR|O_CREAT),0644);
   if(file==-1)
   {
      View|=2;
      file=open(name,O_RDONLY);
	 /* try to open the file in read-only mode */
      if(file==-1)
      {
	 FError(name);
	 EmptyText();
	 return(ERR);
      }
   }

   // re-stat the file in case it was created
   fstat(file,&st);
   FileMode=st.st_mode;

   if(!View)
   {
      int lock_res=LockFile(file,true);
      if(lock_res==-1)
      {
	 View&=~2;
	 close(file);
	 file=-1;
	 EmptyText();
         return(ERR);
      }
      if(lock_res==-2)
	 ErrMsg("Warning: file locking failed");
   }

   if(!buffer_mmapped)
   {
      if(ReplaceTextFromFile(file,st.st_size,&act_read)!=OK)
      {
	 if(errno)
	    FError(name);
	 EmptyText();
	 return(ERR);
      }
      CheckPoint();

      num DosLastLine=0;
      num UnixLastLine=0;
      for(i=0; ; i++)
      {
	 i=ScanForCharForward(i,'\n');
	 if(i==-1)
	    break;
	 UnixLastLine++;
	 if(i>0 && CharAt_NoCheck(i-1)=='\r')
	    DosLastLine++;
      }
#if !defined(__MSDOS__) && !defined(__CYGWIN32__)
      if(UnixLastLine/2<DosLastLine) /* check if the file has unix or dos format */
#else
      if(UnixLastLine/2<=DosLastLine)
#endif
      {
	DosEol=1;
	EolSize=2;
	EolStr="\r\n";
	TextPoint::OrFlags(COLUNDEFINED|LINEUNDEFINED);
	TextEnd=TextPoint(Size(),DosLastLine,-1);
      }
      else
      {
	 TextEnd=TextPoint(Size(),UnixLastLine,-1);
      }
   }
   else /* buffer_mmapped */
   {
#ifdef HAVE_MMAP
      if((S_ISBLK(FileMode) || S_ISCHR(FileMode))
      && st.st_size<=0)
      {
	 // try to get device size
	 st.st_size=GetDevSize(file);
      }
      if(st.st_size>0)
      {
	 buffer=(char*)mmap(0,st.st_size,PROT_READ|(View?0:PROT_WRITE),
			    MAP_SHARED,file,0);
	 if(buffer==(char*)MAP_FAILED)
	 {
	    FError(name);
	    EmptyText();
	    return ERR;
	 }
	 BufferSize=st.st_size;
	 ptr1=ptr2=BufferSize;
	 GapSize=0;
	 TextEnd=TextPoint(BufferSize);
      }
#endif
   }

   stdcol=modified=0;

   hide=1;
   flag=REDISPLAY_ALL;

   fstat(file,&st);
   FileInfo=InodeInfo(&st);
   strcpy(FileName,name);

   CurrentPos=TextBegin;
   if(SavePos)
   {
      old=PositionHistory.FindInode(FileInfo);
      if(old)
      {
	 if(old->offset!=-1)
	 {
	    CurrentPos=old->offset;
	    if(!hex)
	       stdcol=GetCol();
	 }
	 else if(old->line!=-1 && old->col!=-1)
	    MoveLineCol(old->line,old->col);
      }
   }

   LoadHistory+=HistoryLine(name);

   InitHighlight();

   ScrShift=0;
   CenterView();

   alarm(ALARMDELAY); /* set alarm so the file is dumped at regular intervals */
   return(OK);
}

int   MaxBackup=9;

static char *BackupName(char *buf,char *bp,char *filename,char *bak,int n)
{
   char *suffix=(char*)alloca(strlen(bak)+40+1);
   sprintf(suffix,bak,n);
   sprintf(buf,"%s/%s%s",bp,filename,suffix);
   return buf;
}

static void MoveBackup(char *bp,char *filename,char *bak,int n)
{
   char *bakname=(char*)alloca(strlen(bp)+1+strlen(filename)+strlen(bak)+40+1);

   BackupName(bakname,bp,filename,bak,n);
   if(access(bakname,F_OK)!=-1)
   {
      if(n>=MaxBackup)
	 remove(bakname);
      else
      {
	 char *bakname1=(char*)alloca(strlen(bp)+1+strlen(filename)+strlen(bak)+40+1);
	 BackupName(bakname1,bp,filename,bak,n+1);
	 if(!strcmp(bakname,bakname1))
	    remove(bakname);
	 else
	 {
	    MoveBackup(bp,filename,bak,n+1);
	    if(rename(bakname,bakname1)==-1)
	       remove(bakname);
	 }
      }
   }
}

static int CreateBak(char *name)
{
   char  *buf2;
   num   buf2size;
   num   bytesread;
   struct stat st;
   int   fd,bfd;
   char  directory[256];
   char  *filename;
   int   namemax;
   int	 res=OK;

   filename=strrchr(name,'/');
   if(filename==NULL)
   {
      strcpy(directory,".");
      filename=name;
   }
   else
   {
      if(filename==name)
        strcpy(directory,"/");
      else
      {
        strncpy(directory,name,filename-name);
        directory[filename-name]=0;
      }
      filename++;
   }

   MessageSync("Creating backup file...");

   namemax=pathconf(directory,_PC_NAME_MAX);
   if(namemax==-1)
     namemax=14;


   char *bp=BakPath;
   if(*bp==0)
      bp=directory;
   else if(bp[0]=='~' && (bp[1]==0 || isslash(bp[1])))
   {
      bp=(char*)alloca(strlen(bp)+strlen(HOME));
      sprintf(bp,"%s%s",HOME,BakPath+1);
   }

   MoveBackup(bp,filename,bak,1);

   char *bakname=(char*)alloca(strlen(bp)+1+strlen(filename)+strlen(bak)+40+1);
   BackupName(bakname,bp,filename,bak,1);

   if(stat(name,&st)==-1)
   {
      FError(name);
      return ERR;
   }

   fd=open(name,O_RDONLY);
   bfd=open(bakname,O_TRUNC|O_CREAT|O_WRONLY,st.st_mode&~0077);

   if(fd==-1)
   {
      FError(name);
      return ERR;
   }
   else if(bfd==-1)
   {
      FError(bakname);
      return ERR;
   }
   buf2size=st.st_size;
   if(buf2size>0x1000)
      buf2size=0x1000;
   if((buf2=(char*)malloc(buf2size))==NULL)
   {
      NotMemory();
      res=ERR;
   }
   else
   {
      for(;;)
      {
         bytesread=read(fd,buf2,buf2size);
         if(bytesread==-1)
         {
            FError(name);
            res=ERR;
	    break;
         }
         if(bytesread==0)
            break;
         if(write(bfd,buf2,bytesread)==-1)
	 {
	    FError(bakname);
      	    res=ERR;
	    break;
	 }
      }
      free(buf2);
   }
   close(fd);
   close(bfd);

   if(res==OK)
   {
      struct utimbuf ut;
      ut.actime=st.st_atime;
      ut.modtime=st.st_mtime;
      utime(bakname,&ut);
   }
   return res;
}

int CheckMode(mode_t mode)
{
   if((mode&S_IFMT)!=S_IFREG)
   {
     ErrMsg("This is not a regular file");
     return(0);
   }
   return(1);
}

int   SaveFile(char *name)
{
   struct stat st;
   char  msg[256];
   int   nfile;
   num   act_written;
   int   delete_old_file=0;

   if(buffer_mmapped)
   {
      if(!strcmp(name,FileName))
	 return OK;
   }

   if(Text && !View)
      UserOptimizeText();

   sprintf(msg,"Saving the file \"%.60s\"...",name);
   MessageSync(msg);

   if(stat(name,&st)!=-1)
   {
      if(!CheckMode(st.st_mode))
         return(ERR);

      InodeInfo   NewFileInfo(&st,GetLine(),GetCol());

      if(file!=-1)
      {
	 if(buffer_mmapped && FileInfo.SameFile(NewFileInfo))
	    return OK;

	 if(FileInfo.SameFileModified(NewFileInfo))
	 {
	    switch(ReadMenuBox(ConCan4Menu,HORIZ,"The file was changed out of the editor",
		     " Warning ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
	    {
	    case('C'):
	    case(0):
	       return(ERR);
	    }
	 }
	 else if(!FileInfo.SameFile(NewFileInfo))
	 {
	    switch(ReadMenuBox(ConCan4Menu,HORIZ,"The file already exists and will be overwritten",
		     " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
	    {
	    case('C'):
	    case(0):
	       return(ERR);
	    }
	    delete_old_file=1;
	 }
      }

      if(makebak && !newfile) /* only for 'old' files */
      {
	 if(CreateBak(name)!=OK)
	 {
	    switch(ReadMenuBox(ConCan4Menu,HORIZ,"Cannot create backup file",
		     " Warning ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
	    {
	    case('C'):
	    case(0):
	       return(ERR);
	    }
      	 }
      }
   }
   else
   {
     if(errno!=ENOENT)
     {
       FError(name);
       return(ERR);
     }
     st.st_mode=FileMode|0600;
     delete_old_file=1;
   }

   if(!newfile)
     delete_old_file=0;

   newfile=0;

   MessageSync(msg);

   errno=0;
   nfile=open(name,O_CREAT|O_RDWR,st.st_mode);
   if(nfile==-1)
   {
     FError(name);
     return(ERR);
   }

   int lock_res=LockFile(nfile,false);
   if(lock_res==-1)
   {
     close(nfile);
     return(ERR);
   }
   if(lock_res==-2)
      ErrMsg("Warning: file locking failed");

   // now after locking truncate the file
#ifdef HAVE_FTRUNCATE
   ftruncate(nfile,0);
#else
   close(open(name,O_TRUNC|O_RDONLY));
#endif

   struct stat new_st;
   if(fstat(nfile,&new_st)!=-1 && new_st.st_mode!=st.st_mode)
   {
      /* force new file to be the same mode as source one */
#ifdef HAVE_FCHMOD
      fchmod(nfile,st.st_mode);
#else
      chmod(name,st.st_mode);
#endif
   }

   /* now, after all that stuff, write the buffer contents */
   errno=0;
   if(WriteBlock(nfile,0,Size(),&act_written)!=OK)
   {
     if(errno)
       FError(name);
     close(nfile);
     return(ERR);
   }
   if(act_written!=Size())
   {
     ErrMsg("Cannot write the file up to end\nPerhaps disk is full");
     close(nfile);
     return(ERR);
   }

   if(buffer_mmapped)
   {
      close(nfile);
      return OK;
   }

   modified=0;
   CheckPoint();

   stat(name,&st);
   FileInfo=InodeInfo(&st);
   SavePosition();

   close(file);
   file=nfile;
   LockFile(file,true);

   if(delete_old_file)
   {
     if(stat(FileName,&st)!=-1 && st.st_size==0)
       remove(FileName);
   }

   strcpy(FileName,name);

   return(OK);
}

int   ReopenRW()
{
   struct stat st;

   if(View==0)
      return(OK);

   if(access(FileName,W_OK|R_OK)==-1)
   {
      if(stat(FileName,&st)==-1)
      {
         FError(FileName);
         return ERR;
      }

      if(st.st_uid!=geteuid())
      {
	 ErrMsg("You are not the owner of the file,\nso you cannot force read-write open");
	 return ERR;
      }

      st.st_mode|=S_IRUSR|S_IWUSR;

      if(chmod(FileName,st.st_mode)==-1)
      {
	 FError("chmod() failed");
	 return ERR;
      }
   }

   View=0;

   char	 *name=(char*)alloca(strlen(FileName)+1);
   strcpy(name,FileName);

   offs oldbb=BlockBegin;
   offs oldbe=BlockEnd;
   offs oldpos=CurrentPos;
   int oldhide=hide;

   int res=LoadFile(name);
   if(res==OK)
   {
      BlockBegin=oldbb;
      BlockEnd=oldbe;
      CurrentPos=oldpos;
      hide=oldhide;
   }
   return res;
}

void SavePosition()
{
   num offset=CurrentPos;
   num line=CurrentPos.LineSimple();
   num col=CurrentPos.ColSimple();

   FileInfo.line=line;
   FileInfo.col=col;
   FileInfo.offset=offset;

   PositionHistory+=FileInfo;
}
