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

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "edit.h"
#include "block.h"

int   PipeBlock(char *filter,int in,int out)
{
#ifdef __MSDOS__
   ErrMsg("Piping is not supportrd under MS-DOG");
#else
   int   pipe_in[2],pipe_out[2],pipe_err[2];
   int   in_done,err_done,out_done;
   char  errtext[1024];
   char  *errptr=errtext;
   struct  pollfd  pfd[3];
   long  res;
   char  input_buffer[BUFSIZ];
   offs  oldpos=Offset();
   int   nfd;
   char  *block_buf=0;
   char  *block_ptr=0;
   num   block_size=0;

   CheckBlock();
   if(hide)
      out=0;

   if(!in && !out)
      return OK;  /* nothing to do */

   if(View && in)
      return ERR;

   if(rblock)
   {
      if(!GetBlock())
	 return ERR;
      block_buf=(char*)malloc((block_width+1)*block_height);
      if(!block_buf)
      {
	 NotMemory();
	 return ERR;
      }
      if(out && in)
	 Delete();
      int l,c;
      for(l=0,block_ptr=block_buf,block_size=0; l<block_height; l++)
      {
	 int cm;
	 for(cm=block_width; cm>0 && block[l][cm-1]==' '; cm--);
	 for(c=0; c<cm; c++)
	 {
	    *(block_ptr++)=block[l][c];
	    block_size++;
	 }
	 *(block_ptr++)='\n';
	 block_size++;
      }
      block_ptr=block_buf;
   }
   
   if(in && !out)
   {
      BlockBegin=CurrentPos;
      BlockEnd=CurrentPos;
      hide=0;
   }

   if(pipe(pipe_in)==-1)
   {
      FError("pipe()");
      free(block_buf);
      return ERR;
   }
   if(pipe(pipe_out)==-1)
   {
      FError("pipe()");
      close(pipe_in[0]);
      close(pipe_in[1]);
      free(block_buf);
      return ERR;
   }
   if(pipe(pipe_err)==-1)
   {
      close(pipe_in[0]);
      close(pipe_in[1]);
      close(pipe_out[0]);
      close(pipe_out[1]);
      free(block_buf);
      return ERR;
   }

   fflush(stderr);
   switch(fork())
   {
   case(0):
      dup2(pipe_in[1],1);
      dup2(pipe_out[0],0);
      dup2(pipe_err[1],2);
      close(file);
      close(pipe_in[0]);
      close(pipe_in[1]);
      close(pipe_out[0]);
      close(pipe_out[1]);
      close(pipe_err[0]);
      close(pipe_err[1]);
      execl("/bin/sh","sh","-c",filter,NULL);
      perror("exec(/bin/sh) failed");
      fflush(stderr);
      _exit(1);
   case(-1):
      close(pipe_in[0]);
      close(pipe_in[1]);
      close(pipe_out[0]);
      close(pipe_out[1]);
      close(pipe_err[0]);
      close(pipe_err[1]);
      ErrMsg("fork() failed");
      free(block_buf);
      return ERR;
   }

   close(pipe_in[1]);
   close(pipe_out[0]);
   close(pipe_err[1]);

   fcntl(pipe_in[0],F_SETFL,O_NONBLOCK);
   fcntl(pipe_err[0],F_SETFL,O_NONBLOCK);
   fcntl(pipe_out[1],F_SETFL,O_NONBLOCK);

   pfd[0].fd=pipe_in[0];
   pfd[0].events=POLLIN;
   pfd[1].fd=pipe_err[0];
   pfd[1].events=POLLIN;
   pfd[2].fd=pipe_out[1];
   pfd[2].events=POLLOUT;
   nfd=3;

   out_done=in_done=err_done=0;

   offs old_block_begin=BlockBegin;
   offs old_block_end=BlockEnd;

   if(out)
      CurrentPos=BlockBegin;
   for(;;)
   {
      if(!rblock && !out_done && CurrentPos>=BlockEnd)
      {
         close(pipe_out[1]); /* Close input pipe of the subprocess */
                             /* so it would terminate on EOF  */
         out_done=1;
         nfd--;
      }

      if(poll(pfd,nfd,-1)<0)
      {
	 if(errno==EINTR)
   	    continue;
	 FError("poll()");
	 free(block_buf);
	 return ERR;
      }

      if((pfd[0].revents&(POLLHUP|POLLIN))==POLLHUP)
      {
         in_done=1;
         pfd[0].events=0;
      }
      if((pfd[1].revents&(POLLHUP|POLLIN))==POLLHUP)
      {
         err_done=1;
         pfd[1].events=0;
      }
      if(in_done && err_done)
         break;

      if(pfd[0].revents&POLLIN)
      {
         res=read(pipe_in[0],input_buffer,sizeof(input_buffer));
         if(res<=0)
         {
            in_done=1;
         }
         else if(in)
         {
            if(InsertBlock(input_buffer,res)!=OK)
               goto not_memory;
            if(CurrentPos>BlockEnd)
               BlockEnd=CurrentPos;
         }
      }
      if(pfd[1].revents&POLLIN)
      {
         res=read(pipe_err[0],errptr,sizeof(errtext)-1-(errptr-errtext));
         if(res<=0)
            err_done=1;
         else
            errptr+=res;
      }
      if(nfd>2 && pfd[2].revents&POLLOUT)
      {
         if(rblock)
	 {
	    res=write(pipe_out[1],block_ptr,block_size);
	    if(res==-1)
	       FError("write() to pipe");
	    else
	    {
	       block_ptr+=res;
	       block_size-=res;
	       if(block_size==0)
	       {
		  out_done=1;
        	  close(pipe_out[1]);
		  nfd--;
	       }
	    }
	 }
	 else
	 {
	    if(WriteBlock(pipe_out[1],CurrentPos,BlockEnd-CurrentPos,&res)!=OK)
	       FError("write() to pipe");
	    else
	    {
	       if(res==0)
	       {
		  out_done=1;
		  close(pipe_out[1]);
		  nfd--;
	       }
	       else
	       {
		  if(in)
		     DeleteBlock(0,res);
		  else
		     CurrentPos+=res;
	       }
	    }
	 }
      }
   }
   if(in && out)
      DeleteBlock(0,BlockEnd-CurrentPos);
not_memory:
   close(pipe_in[0]);
   close(pipe_err[0]);
   close(pipe_out[1]);
   free(block_buf);
   waitpid(-1,NULL,WUNTRACED);
   if(errptr!=errtext)
   {
      if(errptr[-1]=='\n')
         errptr--;
      *errptr=0;
      ErrMsg(errtext);
      if(in && out)
      {
	 if(rblock)
	 {
	    rblock=0;
	    Delete();
	    rblock=1;
	    PutOut(GetLine(),GetCol());
	    CurrentPos=oldpos;
	 }
	 else
	 {
	    Undelete(); /* undelete old block */
	    Delete();   /* delete new block */
	    CurrentPos=oldpos;
	    BlockBegin=old_block_begin;
	    BlockEnd  =old_block_end;
	    hide=0;
	 }
      }
      return ERR;
   }
   else
   {
      if(in)
	 rblock=0;
   }
#endif
   return OK;
}
