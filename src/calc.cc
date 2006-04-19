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
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "calc.h"

#define  EPS   DBL_MIN

double   stack[STSIZE];
int   sp;

char  word[256];
int   calcerrno=OKAY;

int   add()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2]+=stack[sp-1];
   sp--;
   return(OKAY);
}
int   sub()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2]-=stack[sp-1];
   sp--;
   return(OKAY);
}
int   mul()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2]*=stack[sp-1];
   sp--;
   return(OKAY);
}
int   div()
{
   if(sp<2)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-2]/=stack[sp-1];
   sp--;
   return(OKAY);
}
int   rem()
{
   if(sp<2)
      return(STUNDERFLOW);
   stack[sp-2]=fmod(stack[sp-2],stack[sp-1]);
   sp--;
   return(OKAY);
}
int   cpy()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp]=stack[sp-1];
   sp++;
   return(OKAY);
}
int   pwr()
{
   if(sp<2)
      return(STUNDERFLOW);
   if(stack[sp-1]<0)
      return(ILLEGALFN);
   stack[sp-2]=pow(stack[sp-1],stack[sp-2]);
   sp--;
   return(OKAY);
}
int   mpi()
{
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp++]=M_PI;
   return(OKAY);
}
int   me()
{
   if(sp>=STSIZE)
      return(STOVERFLOW);
   stack[sp++]=M_E;
   return(OKAY);
}
int   sqr()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=sqrt(fabs(stack[sp-1]));
   return(OKAY);
}
int   sq()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]*=stack[sp-1];
   return(OKAY);
}
int   del()
{
   if(sp<1)
      return(STUNDERFLOW);
   sp--;
   return(OKAY);
}
int   clr()
{
   initcalc();
   return(OKAY);
}
int   ln()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-1]=log(fabs(stack[sp-1]));
   return(OKAY);
}
int   lg()
{
   if(sp<1)
      return(STUNDERFLOW);
   if(fabs(stack[sp-1])<EPS)
      return(ILLEGALFN);
   stack[sp-1]=log10(fabs(stack[sp-1]));
   return(OKAY);
}
int   neg()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=(-stack[sp-1]);
   return(OKAY);
}
int   rev()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=1/stack[sp-1];
   return(OKAY);
}
int   xy()
{
   double   ox;
   if(sp<2)
      return(STUNDERFLOW);
   ox=stack[sp-1];
   stack[sp-1]=stack[sp-2];
   stack[sp-2]=ox;
   return(OKAY);
}
int   expx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=exp(stack[sp-1]);
   return(OKAY);
}
int   sinx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=sin(stack[sp-1]);
   return(OKAY);
}
int   cosx()
{
   if(sp<1)
      return(STUNDERFLOW);
   stack[sp-1]=cos(stack[sp-1]);
   return(OKAY);
}

struct   func_def
{
   const char *name;
   int (*func)();
}
const func[]={

{"+",add},  {"-",sub},  {"/",div},  {"*",mul},
{"cp",cpy}, {"**",pwr}, {"pi",mpi}, {"e",me},
{"sqr",sqr},   {"sq",sq},  {"del",del},   {"clr",clr},
{"ln",ln},  {"lg",lg},  {"xy",xy},  {"neg",neg},
{"rev",rev},   {"exp",expx},  {"%",rem},  {"sin",sinx},
{"cos",cosx},

{NULL}};

int   check_for_number(char *w)
{
   return(isdigit(w[0]) || w[0]=='-' && isdigit(w[1]));
}

int   calculator(const char *in)
{
   int      wl;
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
         if(sscanf(word,"%lg",stack+sp)==0)
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
