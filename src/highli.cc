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

/* $Id$ */

#include "highli.h"
#include "edit.h"
#include "screen.h"
#include <fnmatch.h>
#include <string.h>
#include <stdlib.h>

int hl_option=1;
int hl_active=0;

syntax_hl *syntax_hl::chain=0;

syntax_hl::syntax_hl(int color,int mask)
{
   rexp=0;
   next=0;
   for(syntax_hl **scan=&chain; ; scan=&(**scan).next)
   {
      if(!*scan)
      {
	 *scan=this;
	 break;
      }
   }
   this->color=color;
   this->mask=mask;
   memset(&rexp_c,0,sizeof(rexp_c));
}

syntax_hl::~syntax_hl()
{
   if(rexp)
   {
      delete rexp;
      regfree(&rexp_c);
   }
   for(syntax_hl **scan=&chain; ; scan=&(**scan).next)
   {
      if(*scan==this)
      {
	 *scan=this->next;
	 break;
      }
   }
}

void syntax_hl::free_chain()
{
   while(chain)
      delete chain;
}

const char *syntax_hl::set_rexp(const char *nr)
{
   if(rexp)
   {
      delete rexp;
      regfree(&rexp_c);
      rexp=0;
   }
   re_syntax_options=RE_NO_BK_VBAR|RE_NO_BK_PARENS|RE_INTERVALS|
		     RE_CHAR_CLASSES|RE_CONTEXT_INDEP_ANCHORS;
   const char *err=re_compile_pattern(nr,strlen(nr),&rexp_c);
   if(err)
      return err;
   rexp=new char[strlen(nr)+1];
   strcpy(rexp,nr);
   return 0;
}

void c_string_interpret(char *s)
{
   while(*s)
   {
      if(*s=='\\')
      {
	 switch(s[1])
	 {
	 case('\0'):
	    return;
	 case('\\'):
	    break;
	 case('n'):
	    *s='\n';
	    break;
	 case('t'):
	    *s='\t';
	    break;
	 case('b'):
	    *s='\b';
	    break;
	 default:
	    s++;
	    continue;
	 }
	 s++;
	 memmove(s,s+1,strlen(s));
      }
      else
	 s++;
   }
}

extern void fskip(FILE*);

void InitHighlight()
{
   int ch;
   FILE *f;

   syntax_hl::free_chain();

   hl_active=0;
   if(!hl_option)
      return;

   const char base_fn[]="syntax";
   char *fn1=(char*)alloca(strlen(PKGDATADIR)+1+strlen(base_fn)+1);
   char *fn2=(char*)alloca(strlen(HOME)+1+3+1+ strlen(base_fn)+1);
   char *fn3=(char*)alloca(4+strlen(base_fn)+1);
   char *fn;

   sprintf(fn1,"%s/%s",PKGDATADIR,base_fn);
   sprintf(fn2,"%s/.le/%s",HOME,base_fn);
   sprintf(fn3,".le.%s",base_fn);

   f=0;
   if(!f)
      f=fopen(fn=fn3,"r");
   if(!f)
      f=fopen(fn=fn2,"r");
   if(!f)
      f=fopen(fn=fn1,"r");
   if(!f)
      return;

   char str[1024];
   char *s;
   int match=0;
   int len;
   int res;
   int color,mask;

   for(;;)
   {
      ch=fgetc(f);
      switch(ch)
      {
      case(EOF):
      end:
	 fclose(f);
	 return;
      case('/'):
	 if(match)
	    goto end;
	 s=fgets(str,sizeof(str),f);
	 if(!s)
	    goto end;
	 len=strlen(s);
	 if(s[len-1]=='\n')
	    len--;
	 if(s[len-1]=='\r')
	    len--;
	 s[len]=0;
	 s=strtok(str,"|");
	 while(s)
	 {
	    if(s[0]=='/')
	    {
	       // it is a regex for file contents
	       s++;

	       if(!buffer)
		  continue;   // no buffer - no pattern

	       static re_pattern_buffer rexp;
	       re_syntax_options=0;
	       if(!re_compile_pattern(s,strlen(s),&rexp))
	       {
		  int pos=re_search_2(&rexp,buffer,ptr1,buffer+ptr2,BufferSize-ptr2,
				      0,1024,NULL,1024);
		  if(pos!=-1)
		  {
		     match=1;
		     break;
		  }
	       }
	    }
	    if(fnmatch(s,FileName,0)==0)
	    {
	       match=1;
	       break;
	    }
	    s=strtok(0,"|");
	 }
	 break;
      case('c'):
      {
	 if(!match)
	 {
	    fskip(f);
	    continue;
	 }
	 res=fscanf(f,"%d,%i=",&color,&mask);
	 if(res!=2)
	 {
	    fskip(f);
	    continue;
	 }
	 char *accum=0;
	 int cont=1;
	 while(cont)
	 {
	    s=fgets(str,sizeof(str),f);
	    if(!s)
	    {
	       if(accum)
		  break;
	       goto end;
	    }
	    cont=0;
	    len=strlen(s);
	    if(s[len-1]=='\n')
	    {
	       len--;
	       if(s[len-1]=='\r')
		  len--;
	       if(s[len-1]=='\\')
	       {
		  len--;
		  cont=1;
	       }
	    }
	    else
	    {
	       cont=1;
	    }
	    s[len]=0;
	    if(!accum)
	    {
	       accum=strdup(str);
	       if(!accum)
		  goto end;
	    }
	    else
	    {
	       s=(char*)realloc(accum,strlen(accum)+len+1);
	       if(!s)
	       {
		  free(accum);
		  goto end;
	       }
	       accum=s;
	       strcat(accum,str);
	    }
	    if(!cont)
	       break;
	 }
	 c_string_interpret(accum);
	 syntax_hl *hl=new syntax_hl(color,mask);
	 const char *err=hl->set_rexp(accum);
	 if(err)
	 {
	    ErrMsg(err);
	    free(accum);
	    delete hl;
	    goto end;
	 }
	 free(accum);
	 hl_active=1;
	 break;
      }
      default:
	 fskip(f);
      case('\n'):
	 break;
      }
   }
}

void syntax_hl::attrib_line(const char *buf1,int len1,
			    const char *buf2,int len2,unsigned char *line)
{
   int ll=len1+len2;
   memset(line,'\0',ll);
   for(syntax_hl *scan=chain; scan; scan=scan->next)
   {
      int pos=0;
      for(;;)
      {
	 pos=re_search_2(&scan->rexp_c,buf1,len1,buf2,len2,
			 pos,ll-pos,&scan->regs,ll);
	 if(pos==-1)
	    break;
	 unsigned r;
	 unsigned m;
	 for(r=0,m=1; r<scan->regs.num_regs; r++,m=m<<1)
	 {
	    if(scan->regs.start[r]==-1 || !(scan->mask & m))
	       continue;
	    if(pos<scan->regs.end[r]-1)
	       pos=scan->regs.end[r]-1;
	    for(int i=scan->regs.start[r]; i<scan->regs.end[r]; i++)
	       if(line[i]==0)
		  line[i]=scan->color;
	 }
	 pos++;
      }
   }
}
