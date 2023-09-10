/*
 * Copyright (c) 1993-2019 by Alexander V. Lukyanov (lav@yars.free.net)
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

#include "edit.h"
#include <fnmatch.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "highli.h"
#include "screen.h"
#include "search.h"
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#include <limits.h>

#include <set>
#include <string>

int hl_option=1;
int hl_active=0;

int hl_lines=20; // maximum height of highlighted constructs

syntax_hl *syntax_hl::chain;
char *syntax_hl::selector;

syntax_hl::syntax_hl(int color,int mask)
{
   rexp=0;
   next=sub=0;
   this->color=color;
   this->mask=mask;
   memset(&rexp_c,0,sizeof(rexp_c));
   memset(&regs,0,sizeof(regs));
}

syntax_hl::~syntax_hl()
{
   if(rexp)
   {
      free(rexp);
      regfree(&rexp_c);
   }
   free_chain(sub);
}

void syntax_hl::free_chain(syntax_hl *chain)
{
   for(syntax_hl *r=chain; r; r=chain) {
      chain=r->next;
      delete r;
   }
}

const char *syntax_hl::set_rexp(const char *nr,bool ignore_case)
{
   if(rexp)
   {
      free(rexp);
      regfree(&rexp_c);
      memset(&rexp_c,0,sizeof(rexp_c));
      rexp=0;
      if(regs.start)
	 free(regs.start);
      if(regs.end)
	 free(regs.end);
      memset(&regs,0,sizeof(regs));
   }
   rexp=strdup(nr);
   if(rexp==0)
      return 0;
   if(ignore_case)
   {
      map_to_lower_init();
      rexp_c.translate=(RE_TRANSLATE_TYPE)malloc(256);
      memcpy(rexp_c.translate,map_to_lower,256);
   }
   re_syntax_options = RE_SYNTAX_EMACS | RE_FRUGAL | RE_NO_POSIX_BACKTRACKING |
      RE_NO_BK_VBAR | RE_NO_BK_PARENS | RE_CONTEXT_INDEP_ANCHORS;
   const char *err=re_compile_pattern(rexp,strlen(rexp),&rexp_c);
   if(err)
      return err;
   rexp_c.fastmap=(char*)malloc(256);
   re_compile_fastmap(&rexp_c);
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
	 case('r'):
	    *s='\r';
	    break;
	 case('t'):
	    *s='\t';
	    break;
#if 0 // \b is word bound in regex (one could type \\b, but it's not convenient)
	 case('b'):
	    *s='\b';
	    break;
#endif
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

char *read_regex(FILE*f)
{
   char str[1024];
   char *accum=0;
   int cont=1;
   while(cont)
   {
      char *s=fgets(str,sizeof(str),f);
      if(!s)
      {
	 if(accum)
	    break;
	 return 0;
      }
      cont=0;
      int len=strlen(s);
      if(s[len-1]=='\n')
      {
	 len--;
	 if(s[len-1]=='\r')
	    len--;
	 if(s[len-1]=='\\')
	 {
	    len--;
	    cont=1;
	    for(;;) {
	       int ch=fgetc(f);
	       if(ch==EOF || ch=='\n') {
		  cont=0;
		  break;
	       }
	       if(ch!=' ' && ch!='\t') {
		  ungetc(ch,f);
		  break;
	       }
	    }
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
	    return 0;
      }
      else
      {
	 s=(char*)realloc(accum,strlen(accum)+len+1);
	 if(!s)
	 {
	    free(accum);
	    return 0;
	 }
	 accum=s;
	 strcat(accum,str);
      }
   }
   c_string_interpret(accum);
   return accum;
}

static FILE *open_syntax_d(const char *name)
{
   if(name[0]!='/') {
      const char *base_dir="syntax.d";
      unsigned nbytes=strlen(PKGDATADIR)+strlen(HOME)+1+strlen(base_dir)+1+strlen(name)+1;
      char *fn=(char*)alloca(nbytes);
      snprintf(fn,nbytes,"%s/.le/%s/%s",HOME,base_dir,name);
      if(access(fn,R_OK)==-1)
	 snprintf(fn,nbytes,"%s/%s/%s",PKGDATADIR,base_dir,name);
      name=fn;
   }
   return fopen(name,"r");
}

static std::set< std::string > files_loaded;
static bool hl_section_match;
static void ReadSyntaxFile(const char *fn,FILE *f,syntax_hl **chain)
{
   if(!files_loaded.insert(fn).second)
      return;

   int ch;
   char str[1024];
   char *s;
   unsigned len;
   int res;
   int color,mask;
   const char *bn=le_basename(FileName);
   char *rx;

   for(;;)
   {
      ch=fgetc(f);
      switch(ch)
      {
      case(EOF):
	 goto end;
      case('/'):
	 if(hl_section_match)
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
	 syntax_hl::selector=strdup(s);
	 s=strtok(str,"|");
	 while(s)
	 {
	    if(s[0]=='/')
	    {
	       // it is a regex for file contents
	       if(strlen(s)+(s-str)<len)
		  s[strlen(s)]='|';    // undo strtok

	       s++;

	       if(!buffer)
		  break;

	       static re_pattern_buffer rexp;
	       re_syntax_options = RE_SYNTAX_EMACS |
		  RE_NO_BK_VBAR | RE_NO_BK_PARENS | RE_CONTEXT_INDEP_ANCHORS;
	       if(!re_compile_pattern(s,strlen(s),&rexp))
	       {
		  int s1=ptr1;
		  int s2=BufferSize-ptr2;
		  char *p1=s1?buffer:0;
		  char *p2=s2?buffer+ptr2:0;
		  if(p2 && !p1)
		  {
		     p1=p2;
		     s1=s2;
		     p2=0;
		     s2=0;
		  }
		  int pos=-1;
		  if(p1)
		     pos=re_search_2(&rexp,p1,s1,p2,s2,0,1024,NULL,1024);
		  if(pos!=-1)
		  {
		     hl_section_match=true;
		     break;
		  }
	       }
	       break;
	    }
	    if(fnmatch(s,bn,0)==0)
	    {
	       hl_section_match=true;
	       break;
	    }
	    s=strtok(0,"|");
	 }
	 if(!hl_section_match) {
	    free(syntax_hl::selector);
	    syntax_hl::selector=0;
	 }
	 break;
      case('c'):
      {
	 if(!hl_section_match)
	 {
	    fskip(f);
	    continue;
	 }
	 bool ignore_case=false;
	 int c=fgetc(f);
	 if(c=='i')
	    ignore_case=true;
	 else
	    ungetc(c,f);
	 res=fscanf(f,"%d,%i=",&color,&mask);
	 if(res==1) {
	    mask=1;
	    if(fgetc(f)!='=') {
	       fskip(f);
	       continue;
	    }
	 }
	 else if(res!=2)
	 {
	    fskip(f);
	    continue;
	 }
	 else {
	    mask<<=1;
	 }
	 rx=read_regex(f);
	 if(!rx)
	    goto end;
	 syntax_hl *hl=new syntax_hl(color,mask);
	 const char *err=hl->set_rexp(rx,ignore_case);
	 free(rx); rx=0;
	 if(err)
	 {
	    ErrMsg(err);
	    delete hl;
	    goto end;
	 }
	 *chain=hl; // add to chain
	 chain=&hl->next;
	 hl_active=1; // have at least one element
	 break;
      }
      case('h'):
	 if (fscanf(f,"%d",&hl_lines) < 0)
	    /*ignore*/;
	 if(hl_lines<1)
	    hl_lines=1;
	 fskip(f);
	 break;
      case('i'):
	 if(!hl_section_match)
	 {
	    fskip(f);
	    continue;
	 }
	 /*fallthrought*/
      case('I'):
	 if(fscanf(f,"=%255s",str)==1) {
	    FILE *i_f=open_syntax_d(str);
	    if(i_f) {
	       ReadSyntaxFile(str,i_f,chain);
	       while(*chain) // skip the newly added nodes
		  chain=&chain[0]->next;
	    }
	 }
	 fskip(f);
	 break;
      case('s'):
      {
	 if(!hl_section_match)
	 {
	    fskip(f);
	    continue;
	 }
	 ch=fgetc(f);
	 bool ignore_case=false;
	 if(ch=='i') {
	    ignore_case=true;
	    ch=fgetc(f);
	 }
	 if(ch!='(') {
	    fskip(f);
	    break;
	 }
	 if(fscanf(f,"%255[^)\n=])",str)==1) {
	    res=fscanf(f,"%i=",&mask);
	    if(res!=1) {
	       mask=1;
	       if(fgetc(f)!='=') {
		  fskip(f);
		  continue;
	       }
	    } else {
	       mask<<=1;
	    }
	    rx=read_regex(f);
	    if(!rx)
	       goto end;

	    syntax_hl *hl=new syntax_hl(-1,mask);
	    const char *err=hl->set_rexp(rx,ignore_case);
	    free(rx); rx=0;
	    if(err)
	    {
	       ErrMsg(err);
	       delete hl;
	       goto end;
	    }
	    *chain=hl; // add to chain
	    chain=&hl->next;
	    hl_active=1; // have at least one element

	    FILE *i_f=open_syntax_d(str);
	    if(i_f) {
	       ReadSyntaxFile(str,i_f,&hl->sub);
	    }
	 } else {
	    fskip(f);
	 }
	 break;
      }
      default:
	 fskip(f);
      case('\n'):
	 break;
      }
   }
end:
   fclose(f);
}

void InitHighlight()
{
   files_loaded.clear();
   free(syntax_hl::selector);
   syntax_hl::selector=0;
   syntax_hl::free_chain(syntax_hl::chain);
   syntax_hl::chain=0;

   hl_active=0;
   if(!hl_option)
      return;

   static const char base_fn[]="syntax";
   unsigned nbytes1=strlen(PKGDATADIR)+1+strlen(base_fn)+1;
   unsigned nbytes2=strlen(HOME)+1+3+1+ strlen(base_fn)+1;
   unsigned nbytes3=4+strlen(base_fn)+1;
   char *fn1=(char*)alloca(nbytes1);
   char *fn2=(char*)alloca(nbytes2);
   char *fn3=(char*)alloca(nbytes3);
   char *fn;

   snprintf(fn1,nbytes1,"%s/%s",PKGDATADIR,base_fn);
   snprintf(fn2,nbytes2,"%s/.le/%s",HOME,base_fn);
   snprintf(fn3,nbytes3,".le.%s",base_fn);

   FILE *f=0;
   if(!f)
      f=fopen(fn=fn3,"r");
   if(!f)
      f=fopen(fn=fn2,"r");
   if(!f)
      f=fopen(fn=fn1,"r");
   if(!f)
      return;
   hl_section_match=false;
   ReadSyntaxFile(fn,f,&syntax_hl::chain);
}

class element
{
   static element *pool;
   static element *hunk;
   static int hunk_size;
public:
   int begin,end;
   element *next;
   element *sub;
   byte color;

   static element *New();
   static void Free(element *);
   static void FreeChain(element *);
};

element *element::pool;
element *element::hunk;
int	 element::hunk_size;

element *element::New()
{
   element *res;
   if(pool)
   {
      res=pool;
      pool=pool->next;
      return res;
   }
   if(!hunk || hunk_size==0)
      hunk=new element[hunk_size=128];
   res=hunk;
   hunk++;
   hunk_size--;
   return res;
}

void element::Free(element *el)
{
   FreeChain(el->sub);
   el->sub=0;
   el->next=pool;
   pool=el;
}
void element::FreeChain(element *el)
{
   if(!el)
      return;

   element *tmp=el;
   while(tmp->next)
      tmp=tmp->next;
   tmp->next=pool;
   pool=el;
}

static element *top_els;

#ifdef HAVE_TIMES
static clock_t clock0;
#endif

void syntax_hl::make_els(const char *buf1,int len1,
			 const char *buf2,int len2,
			 int pos0,int ll,syntax_hl *scan,element **elpp0)
{
   element *el;
   element **elpp;

   for( ; scan; scan=scan->next)
   {
      int pos=0;
      elpp=elpp0;
      for(;;)
      {
	 if(pos>=ll)
	    break;
	 pos=re_search_2(&scan->rexp_c,buf1,len1,buf2,len2,
			 pos,ll-pos,&scan->regs,ll);
	 if(pos==-1) // not found
	    break;
	 if(pos==-2) // error ?
	    break;
	 unsigned r;
	 unsigned m;
	 for(r=0,m=1; r<scan->regs.num_regs; r++,m=m<<1)
	 {
	    if(scan->regs.start[r]==-1 || scan->regs.start[r]==scan->regs.end[r]
	    || !(scan->mask & m))
	       continue;

	    el=element::New();
	    el->begin=scan->regs.start[r]+pos0;
	    el->end=scan->regs.end[r]+pos0;
	    el->color=scan->color;
	    el->sub=0;

	    // insert new element in proper position
	    while(*elpp && elpp[0]->begin<=el->begin)
	       elpp=&elpp[0]->next;
	    el->next=*elpp;
	    *elpp=el;

	    if(scan->sub) {
	       // reduce the buffer so that ^ and $ work properly
	       int sub_ll=scan->regs.end[r]-scan->regs.start[r];
	       int sub_pos=scan->regs.start[r];
	       int sub_len1=len1>sub_pos?len1-sub_pos:0;
	       const char *sub_buf1=len1>sub_pos?buf1+sub_pos:0;
	       int sub_len2=len1>sub_pos?len2:len2-(sub_pos-len1);
	       const char *sub_buf2=len1>sub_pos?buf2:buf2+(sub_pos-len1);
	       if(sub_len2<0) {
		  sub_len2=0;
		  sub_buf2=0;
	       }
	       if(sub_len1<=0 && sub_len2>0) {
		  sub_len1=sub_len2;
		  sub_buf1=sub_buf2;
		  sub_len2=0;
		  sub_buf2=0;
	       }
	       if(sub_len1>sub_ll) {
		  sub_len1=sub_ll;
		  sub_len2=0;
		  sub_buf2=0;
	       } else if(sub_len1+sub_len2>sub_ll) {
		  sub_len2=sub_ll-sub_len1;
	       }
	       make_els(sub_buf1,sub_len1,sub_buf2,sub_len2,sub_pos+pos0,sub_len1+sub_len2,scan->sub,&el->sub);
	    }
	 }
	 pos++;

#ifdef HAVE_TIMES
# ifndef CLOCKS_PER_SEC
#  define CLOCKS_PER_SEC CLK_TCK
# endif
	 struct tms tms;
	 times(&tms);
	 clock_t clock1=tms.tms_utime;
	 if(clock1-clock0>CLOCKS_PER_SEC/5)
	    break;
#endif
      }
   }
}

static void do_color(element *els,unsigned char *line)
{
   for(element *el=els; el; el=el->next)
   {
      int end=el->end;
      if(el->sub) {
	 do_color(el->sub,line);
      } else {
	 for(int i=el->begin; i<end; i++)
	    line[i]=el->color;
      }
      // skip overlapping elements
      while(el->next && el->next->begin<end)
	 el=el->next;
   }
}

void syntax_hl::attrib_line(const char *buf1,int len1,
			    const char *buf2,int len2,unsigned char *line)
{
   int ll=len1+len2;
   if(ll==0)
      return;

   memset(line,'\0',ll);

   // It's too expensive to color such a long text
   if(ll>hl_lines*1024)
      return;

#ifdef HAVE_TIMES
   struct tms tms;
   times(&tms);
   clock0=tms.tms_utime;
#endif

   make_els(buf1,len1,buf2,len2,0,ll,chain,&top_els);
   do_color(top_els,line);

   element::FreeChain(top_els);
   top_els=0;
}
