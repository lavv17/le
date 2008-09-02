/*
 * Copyright (c) 1993-2006 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "calc.h"

#define  EPS   DBL_MIN

calc_value stack[STSIZE];
int   sp;

int   calcerrno=OKAY;

static int add()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value+=stack[sp-1];
   sp--;
   return(OKAY);
}
static int sub()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value-=stack[sp-1];
   sp--;
   return(OKAY);
}
static int mul()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value*=stack[sp-1];
   sp--;
   return(OKAY);
}
static int div()
{
   if(sp<2)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-2].value/=stack[sp-1];
   sp--;
   return(OKAY);
}
static int rem()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value=fmod(stack[sp-2],stack[sp-1]);
   sp--;
   return(OKAY);
}
static int cpy()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp].value=stack[sp-1];
   stack[sp].base=stack[sp-1].base;
   sp++;
   return(OKAY);
}
static int pwr()
{
   if(sp<2)
      return(STUNDERFLOW);
   if(stack[sp-1]<0)
      return(ILLEGALFN);
   stack[sp-2].value=pow(stack[sp-1],stack[sp-2]);
   sp--;
   return(OKAY);
}
static int mpi()
{
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp].base=10;
   stack[sp++].value=M_PI;
   return(OKAY);
}
static int me()
{
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp].base=10;
   stack[sp++].value=M_E;
   return(OKAY);
}
static int sqr()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=sqrt(fabs(stack[sp-1]));
   return(OKAY);
}
static int sq()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value*=stack[sp-1];
   return(OKAY);
}
static int del()
{
   if(sp<1)
      return(STUNDERFLOW);
   sp--;
   return(OKAY);
}
static int clr()
{
   initcalc();
   return(OKAY);
}
static int ln()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-1].value=log(fabs(stack[sp-1]));
   return(OKAY);
}
static int lg()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-1].value=log10(fabs(stack[sp-1]));
   return(OKAY);
}
static int neg()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=(-stack[sp-1]);
   return(OKAY);
}
static int rev()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=1/stack[sp-1];
   return(OKAY);
}
static int xy()
{
   calc_value ox;
   if(sp<2)
      return(STUNDERFLOW);
   ox=stack[sp-1];
   stack[sp-1]=stack[sp-2];
   stack[sp-2]=ox;
   return(OKAY);
}
static int expx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=exp(stack[sp-1]);
   return(OKAY);
}
static int sinx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=sin(stack[sp-1]);
   return(OKAY);
}
static int cosx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=cos(stack[sp-1]);
   return(OKAY);
}
static int tgx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=tan(stack[sp-1]);
   return(OKAY);
}
static int ctgx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=1/tan(stack[sp-1]);
   return(OKAY);
}
static int asinx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=asin(stack[sp-1]);
   return(OKAY);
}
static int acosx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=acos(stack[sp-1]);
   return(OKAY);
}
static int atgx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=atan(stack[sp-1]);
   return(OKAY);
}
static int actgx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=atan(1/stack[sp-1]);
   return(OKAY);
}

static int fact()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(stack[sp-1]<0)
      return(ILLEGALFN);
   long long n=(long long)stack[sp-1].value;
   if(stack[sp-1].value==0)
      stack[sp-1].value=1;
   while(--n>0 && !isnan(stack[sp-1].value))
      stack[sp-1].value*=n;
   return(OKAY);
}

static int f_and()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value=(long long)(stack[sp-1])&(long long)(stack[sp-2]);
   sp--;
   return(OKAY);
}
static int f_or()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value=(long long)(stack[sp-1])|(long long)(stack[sp-2]);
   sp--;
   return(OKAY);
}
static int f_xor()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2].value=(long long)(stack[sp-1])^(long long)(stack[sp-2]);
   sp--;
   return(OKAY);
}
static int f_not()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].value=~(long long)(stack[sp-1]);
   return(OKAY);
}

static int b16()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].base=16;
   return(OKAY);
}
static int b10()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].base=10;
   return(OKAY);
}
static int b8()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1].base=8;
   return(OKAY);
}

static int f_sum()
{
   if(sp<1)
      return STUNDERFLOW;
   while(add()==OKAY);
   return OKAY;
}

struct   func_def
{
   const char *name;
   int (*func)();
}
const func[]={

{"+",add}, {"-",sub}, {"/",div}, {"*",mul},
{"cp",cpy}, {"**",pwr}, {"pi",mpi}, {"e",me},
{"sqr",sqr}, {"sq",sq}, {"del",del}, {"clr",clr},
{"ln",ln}, {"lg",lg}, {"xy",xy}, {"neg",neg},
{"rev",rev}, {"exp",expx}, {"%",rem}, {"fact",fact},

{"sin",sinx}, {"cos",cosx}, {"tg",tgx}, {"ctg",ctgx},
{"asin",asinx}, {"acos",acosx}, {"atg",atgx}, {"actg",actgx},

{"and",f_and}, {"or",f_or}, {"xor",f_xor}, {"not",f_not},
{"b16",b16}, {"b10",b10}, {"b8",b8},

{"sum",f_sum},

{NULL}};

static int check_for_number(char *w)
{
   return(isdigit(w[0]) || ((w[0]=='-'||w[0]=='.') && isdigit(w[1])));
}
const char *calc_value::to_string()
{
   static char s[256];
   if(base==10)
      sprintf(s,"%.28Lg",value);
   else if(base==8)
      sprintf(s,"%#llo",(long long)value);
   else if(base==16)
      sprintf(s,"0x%llX",(long long)value);
   else
      strcpy(s,"unsupported base");
   return s;
}

int   calculator(const char *in)
{
   char  word[256];
   int   wl;
   const func_def *f;
   do
   {
      while(*in && isspace(*in))
         in++;
      if(!*in)
         break;
      wl=0;
      while(*in && !isspace(*in) && wl<255)
         word[wl++]=(*(in++));
      while(*in && !isspace(*in))
         in++;
      word[wl]=0;
      if(check_for_number(word))
      {
         if(sp>=STSIZE)
            return(calcerrno=STOVERFLOW);
	 if(word[0]=='0' || (word[0]=='-' && word[1]=='0'))
	 {
	    char base_char=(word[0]=='-'?word[2]:word[1]);
	    long long n;
	    if(base_char=='x' || base_char=='X')
	    {
	       stack[sp].base=16;
	       if(sscanf(word,"%llx",&n)==0)
		  return(calcerrno=INVALIDNUM);
	       stack[sp++].value=n;
	       continue;
	    }
#if 0
	    if(base_char=='b' || base_char=='B')
	    {
	       stack[sp].base=2;
	       if(sscanf(word,"%llb",&n)==0)
		  return(calcerrno=INVALIDNUM);
	       stack[sp++].value=n;
	       continue;
	    }
#endif
	    if(base_char>='0' && base_char<='9')
	    {
	       stack[sp].base=8;
	       if(sscanf(word,"%llo",&n)==0)
		  return(calcerrno=INVALIDNUM);
	       stack[sp++].value=n;
	       continue;
	    }
	 }
	 stack[sp].base=10;
         if(sscanf(word,"%Lg",&stack[sp].value)==0)
            return(calcerrno=INVALIDNUM);
         sp++;
      }
      else
      {
         for(f=func; f->name && strcmp(f->name,word); f++);
         if(!f->name)
            return(calcerrno=INVALIDFN);
         calcerrno=f->func();
         if(calcerrno!=OKAY)
            return(calcerrno);
      }
   }
   while(1);
   return(OKAY);
}

void  initcalc()
{
   sp=0;
}

#define  MAXCALCERR  5
const char  *const calc_errlist[MAXCALCERR]=
{
   "Stack underflow",
   "Stack overflow",
   "Illegal function call",
   "Invalid function name",
   "Invalid number format"
};
const char *calcerrmsg()
{
   if(calcerrno>=0)
      return("No error");
   if(-calcerrno>MAXCALCERR)
      return("Unknown error");
   return(calc_errlist[-calcerrno-1]);
}

#ifdef   MAIN
main(argc,argv)
char  **argv;
{
   int   i;
   char  str[256];
   initcalc();
   printf("Calculator | Version 1.0 | Copyright (c) 1993 by Alexander V. Lukyanov\n");
   do
   {
      printf(sp?"Stack:\n":"Stack is empty\n");
      for(i=0; i<sp; i++)
         printf("\t%.40lg\n",stack[i]);
      if(scanf("%s",str)<1)
         break;
      if(calculator(str)!=OKAY)
         printf("Error: %s\n",calcerrmsg());
   }
   while(1);
   return(0);
}
#endif
