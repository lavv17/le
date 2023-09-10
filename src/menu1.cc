/*
 * Copyright (c) 1993-2008 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#include "clipbrd.h"
#include "block.h"

extern   Menu1 MainMenu[];

static   Menu1 *m=MainMenu;
static   bool  free_m=false;

WIN   *RootWin;

int   *CurrItem();

int   IsValid(int n)
{
   if(!strcmp(m[n].text,"---"))
      return(2);
   if((m[n].fl&MENU_COND_RW) && View)
      return(1);
   if((m[n].fl&MENU_COND_RO) && !View)
      return(1);
   if((m[n].fl&MENU_COND_NO_MM) && buffer_mmapped)
      return 1;
   if((m[n].fl&MENU_COND_CLIPBOARD) && MainClipBoard.IsEmpty())
      return 1;
   if((m[n].fl&MENU_COND_BLOCK))
   {
      CheckBlock();
      if(hide)
	 return 1;
   }
   return(0);
}

int   *CurrItem(int n)
{
   int   level=0;
   do
   {
      if(m[n].text==NULL)
         level--;
      else if(m[n].fl&SUBM)
         level++;
      n++;
   }
   while(level>0 || m[n].text!=NULL);
   return(&(m[n].curritem));
}

int   FirstItem(int n)
{
   int   level=0;

   while(level>0 || (n>0 && !(m[n-1].fl&SUBM)))
   {
      n--;
      if(m[n].text==NULL)
         level++;
      else if(m[n].fl&SUBM)
         level--;
   }
   return(n);
}
int   NextItem(int n)
{
   int   level=0;

   do
   {
      if(m[n].text==NULL)
         level--;
      else if(m[n].fl&SUBM)
         level++;
      n++;
   }
   while(level>0);
   return(m[n].text==NULL?FirstItem(n):n);
}
int   LastItem(int n)
{
   int   o;
   do
   {
      o=n;
      n=NextItem(n);
   }
   while(o<n);
   return(o);
}
int   PrevItem(int n)
{
   int   level=0;

   if(n==0)
      return(LastItem(n));
   do
   {
      n--;
      if(m[n].text==NULL)
         level++;
      else if(m[n].fl&SUBM)
      {
         if(level==0)
            return(LastItem(n+1));
         level--;
      }
   }
   while(level>0);
   return(n);
}

void  Divide(int pos)
{
   int   i;
   SetAttr(MENU_ATTR);
   PutACS(0,pos,LTEE);
   PutACS(Upper->w-1,pos,RTEE);
   for(i=Upper->w-2; i>0; i--)
      PutACS(i,pos,HLINE);
}

void  DisplayMItem(int n,int pos)
{
   if(FirstItem(n)==0)
   {
      if(n==*CurrItem(n) && IsValid(n)==0)
	 DisplayItem(pos,0,m[n].text,CURR_BUTTON_ATTR);
      else
      {
         if(IsValid(n)==0)
            DisplayItem(pos,0,m[n].text,MENU_ATTR);
         else
         {
            DisplayItem(pos,0,m[n].text,DISABLED_ITEM_ATTR);
            PutCh(pos,0,'-');
         }
      }
   }
   else
   {
      if(n==*CurrItem(n) && IsValid(n)==0)
         DisplayItem(1,pos,m[n].text,CURR_BUTTON_ATTR);
      else
      {
         if(IsValid(n)==0)
            DisplayItem(1,pos,m[n].text,MENU_ATTR);
         else if(IsValid(n)==1)
         {
            DisplayItem(1,pos,m[n].text,DISABLED_ITEM_ATTR);
            PutCh(1,pos,'-');
         }
         else
            Divide(pos);
      }
   }
}
void  MoveBar(int n)
{
   int   i,
         pos,
         curr=(*CurrItem(n));

   if(n==-1)
      return;

   *CurrItem(n)=n;

   if((i=FirstItem(n))==0)
   {
      pos=2;
      do
      {
         if(i==n || i==curr)
            DisplayMItem(i,pos);
         pos+=ItemLen(m[i].text);
         i=NextItem(i);
      }
      while(i!=0);
   }
   else
   {
      int   o;
      pos=0;
      do
      {
         pos++;
         if(i==n || i==curr)
            DisplayMItem(i,pos);
         o=i;
         i=NextItem(i);
      }
      while(o<i);
   }
}

void  DisplayMenuWindow(int n)
{
   int   pos,o;
   DisplayWin(m[n].win);
   n++;
   pos=0;
   do
   {
      pos++;
      DisplayMItem(n,pos);
      o=n;
      n=NextItem(n);
   }
   while(o<n);
}

void  FormatItemText(int n,int clear_len)
{
   const char *old_text=m[n].text;
   if(!strcmp(old_text,"---"))
      return;
   int len=strlen(old_text);
   const char *right_text="";
   const char *tab=strchr(old_text,'\t');
   if(tab)
   {
      clear_len++;
      len=tab-old_text;
      right_text=tab+1;
   }
   int right_len=strlen(right_text);
   unsigned nbytes=len+clear_len+right_len+3;
   char *new_text=(char*)malloc(nbytes);
   snprintf(new_text,nbytes," %.*s%*s ",len,old_text,clear_len+right_len,right_text);
   m[n].SetText(new_text);
}

void  CreateMenuWindow(int n,int x,int y)
{
   int   o,nw,
      width=0,
      height=0;

   n++;
   *CurrItem(n)=n;
   do
   {
      if(!strchr(m[n].text,'\t')) {
	 // add shortcut hint
	 const char *shcut=ShortcutPrettyPrint(m[n].action,m[n].arg);
	 if(shcut) {
	    char *new_text=(char*)malloc(strlen(m[n].text)+1+strlen(shcut)+1);
	    strcpy(new_text,m[n].text);
	    strcat(new_text,"\t");
	    strcat(new_text,shcut);
	    m[n].SetText(new_text);
	 }
      }

      if((nw=ItemLen(m[n].text)+5)>width)
         width=nw;
      height++;
      if(m[n].fl&SUBM)
         CreateMenuWindow(n,x+2,y+height+1l);
      o=n;
      n=NextItem(n);
   }
   while(o<n);
   m[n-1].win=CreateWin(x,y+1,width,height+2,MENU_ATTR,"",0);

   int clear_len;
   do
   {
      if(!strcmp(m[n].text,"---"))
	 goto next;
      clear_len=width-4-ItemLen(m[n].text);
      if(clear_len<0)
	 goto next;
      FormatItemText(n,clear_len);
   next:
      o=n;
      n=NextItem(n);
   }
   while(o<n);
}

void  InitMenu()
{
   int   n,pos;

   if(RootWin)
      DestroyWin(RootWin);
   RootWin=CreateWin(0,0,COLS,1,MENU_ATTR,"",NOSHADOW);
   n=0;
   pos=2;
   *CurrItem(0)=0;
   do
   {
      FormatItemText(n,0);
      if(m[n].fl&SUBM)
         CreateMenuWindow(n,pos,0);
      pos+=ItemLen(m[n].text);
      n=NextItem(n);
   }
   while(n!=0);
}

void  DisplayRoot()
{
   int   i=0,
      pos=2;
   do
   {
      DisplayMItem(i,pos);
      pos+=ItemLen(m[i].text);
      i=NextItem(i);
   }
   while(i!=0);
}

int   MoveToValid(int n,int (*dir)(int))
{
   int   start=n;
   if(IsValid(n)==0)
      return(n);
   do
   {
      n=dir(n);
   }
   while(n!=start && IsValid(n)!=0);
   if(n==start)
      return(-1);
   return(n);
}

void    ActivateMainMenu(void)
{
   int     level=0,key,action;
   int     curr=0;
   int     pd=FALSE;
   int     n;

   curs_set(0);

   curr=MoveToValid(*CurrItem(0),NextItem);
   if(curr==-1)
      return;
   DisplayWin(RootWin);
   Clear();
   DisplayRoot();
   MoveBar(curr);

   do
   {
      n=MoveToValid(*CurrItem(curr),NextItem);
      if(n==-1)
      {
         if(level==0)
            goto leave_menu;
      }
      else
         curr=n;
      move(LINES-1,COLS-1);
      action=GetNextAction();
      switch(action)
      {
         case(LINE_BEGIN):
            n=MoveToValid(FirstItem(curr),NextItem);
            if(n==-1)
               break;
            MoveBar(curr=n);
            break;
         case(LINE_END):
            n=MoveToValid(LastItem(curr),PrevItem);
            if(n==-1)
               break;
            MoveBar(curr=n);
            break;
         case(CHAR_LEFT):
            if(level==1)
            {
               CloseWin();
               level--;
               curr=MoveToValid(*CurrItem(FirstItem(curr)-1),PrevItem);
               pd=TRUE;
            }
            if(level==0)
               MoveBar(curr=MoveToValid(PrevItem(curr),PrevItem));
            break;
         case(CHAR_RIGHT):
            if(level==1)
            {
               CloseWin();
               level--;
               curr=MoveToValid(*CurrItem(FirstItem(curr)-1),NextItem);
               pd=TRUE;
            }
            if(level==0)
               MoveBar(curr=MoveToValid(NextItem(curr),NextItem));
            break;
         case(LINE_UP):
            if(level>0)
            {
               n=MoveToValid(PrevItem(curr),PrevItem);
               if(n==-1)
                  break;
               MoveBar(curr=n);
            }
            break;
         case(LINE_DOWN):
            if(level==0 && (m[curr].fl&SUBM))
               goto enter;
            n=MoveToValid(NextItem(curr),NextItem);
            if(n==-1)
               break;
            MoveBar(curr=n);
            break;
         case(NEWLINE):
   enter:      if(IsValid(curr)==0)
            {
               if(m[curr].fl&SUBM)
               {
                  DisplayMenuWindow(curr);
                  level++;
                  curr=(*CurrItem(curr+1));
                  n=MoveToValid(curr,NextItem);
                  if(n!=-1)
                     curr=n;
                  MoveBar(curr);
               }
               else
               {
                  if(m[curr].fl&HIDE)
                  {
                     while(level>0)
                     {
                        level--;
                        CloseWin();
                     }
                     CloseWin();
                  }
		  ActionArgument=m[curr].arg;
                  GetActionProc(m[curr].action)();
                  if(m[curr].fl&HIDE)
                  {
                     flag=1;
                     return;
                  }
               }
            }
            break;
         case(CANCEL):
            if(level==0)
               goto leave_menu;
            else
            {
               CloseWin();
               curr=(*CurrItem(FirstItem(curr)-1));
               if(--level==0)
                  goto leave_menu;
            }
            break;
         default:
            if(StringTypedLen==1)
            {
               n=curr;
               key=toupper(StringTyped[0]);
               do
               {
                  n=NextItem(n);
                  if(ItemChar(m[n].text)==key && IsValid(n)==0)
                  {
                     MoveBar(curr=n);
                     goto enter;
                  }
               }
               while(curr!=n);
            }
            break;
      }
      if(curr==-1)
         goto leave_menu;
      if(level==0 && pd && (m[curr].fl&SUBM))
      {
         DisplayMenuWindow(curr);
         level++;
         curr=(*CurrItem(curr+1));
         pd=FALSE;
      }
   }
   while(action!=ENTER_MENU);
leave_menu:
   while(level-->0)
      CloseWin();
   CloseWin();
   refresh();
}

extern void fskip(FILE*);

void LoadMainMenu()
{
   FILE *f;
   char fn[1024];
   char func[256];
   char str[256];
   int mi=0;
   int level=0;

   snprintf(fn,sizeof(fn),"%s/.le/mainmenu",HOME);

   f=fopen(fn,"r");
   if(f)
      goto read_it;

   f=fopen(PKGDATADIR "/mainmenu","r");
   if(f==0)
      return;

read_it:

   if(m && free_m)
   {
      for(int i=0; ; i++)
      {
	 if(m[i].fl&SUBM) {
	    level++;
	    DestroyWin(m[i].win);
	 } else {
	    if(m[i].arg)
	       free(m[i].arg);
	 }
	 if(m[i].text==0)
	 {
	    if(level==0)
	       break;
	    level--;
	 }
	 else
	 {
	    free(m[i].text);
	 }
      }
      free(m);
   }

   m=(Menu1*)calloc(1024,sizeof(Menu1));
   if(!m)
   {
      fclose(f);
      return;
   }
   free_m=true;

   for(;;)
   {
      int c=fgetc(f);
      if(c=='#')
      {
	 fskip(f);
	 continue;
      }
      if(c==' ' || c=='\t' || c=='\n')
	 continue;
      if(c==EOF)
	 break;
      ungetc(c,f);

      if(fscanf(f,"%255s",func)!=1)
	 break;
      if(!strcmp(func,"submenu") || !strcmp(func,"function"))
      {
	 for(;;)
	 {
	    c=fgetc(f);
	    if(c!=' ' && c!='\t')
	       break;
	 }
	 if(c!='"')
	 {
	    if(c!='\n' && c!=EOF)
	       fskip(f);
	    continue;
	 }
	 if(fscanf(f,"%[^\"]\"",str)!=1)
	 {
	    fskip(f);
	    continue;
	 }
	 m[mi].SetText(strdup(str));
	 if(!strcmp(func,"submenu"))
	 {
	    m[mi].fl=SUBM;
	    m[mi].action=0;
	    level++;
	 }
	 else
	 {
	    m[mi].fl=FUNC;

	    for(;;)
	    {
	       c=fgetc(f);
	       if(c!=' ' && c!='\t')
		  break;
	    }
	    if(c==EOF)
	       break;
	    ungetc(c,f);
	    if(c=='\n')
	       continue;

	    if(fscanf(f,"%s",str)==1)
	    {
	       const char *arg=0;
	       int code=ParseActionNameArg(str,&arg);
	       if(code!=-1) {
		  m[mi].action=code;
		  m[mi].arg=ParseActionArgumentAlloc(arg);
	       } else {
		  fprintf(stderr,"invalid function name: %s\n",str);
	       }
	    }
	 }
	 for(;;)
	 {
	    for(;;)
	    {
	       c=fgetc(f);
	       if(c!=' ' && c!='\t')
		  break;
	    }
	    if(c==EOF)
	       break;
	    ungetc(c,f);

	    if(c=='\n')
	       break;

	    if(fscanf(f,"%s",str)==1)
	    {
	       if(!strcmp(str,"hide"))
		  m[mi].fl|=HIDE;
	       else if(!strcmp(str,"rw"))
		  m[mi].fl|=MENU_COND_RW;
	       else if(!strcmp(str,"ro"))
		  m[mi].fl|=MENU_COND_RO;
	       else if(!strcmp(str,"block"))
		  m[mi].fl|=MENU_COND_BLOCK;
	       else if(!strcmp(str,"no-mm"))
		  m[mi].fl|=MENU_COND_NO_MM;
	    }
	 }
	 mi++;
	 fskip(f);
      }
      else if(!strcmp(func,"hline"))
      {
	 m[mi++].text=strdup("---");
	 fskip(f);
      }
      else if(!strcmp(func,"end"))
      {
	 m[mi++].text=0;
	 level--;
	 fskip(f);
      }
      else
      {
	 fskip(f);
      }
   }
   fclose(f);
   InitMenu();
}
