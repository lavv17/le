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
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"

extern   Menu1 MainMenu[];

static   Menu1 *m=MainMenu;

WIN   *RootWin;

int   *CurrItem();

int   IsValid(int n)
{
   if(!strcmp(m[n].text,"---"))
      return(2);
   if(m[n].valid && !(*m[n].valid)())
      return(1);
   return(0);
}

int   *CurrItem(int n)
{
   int   level=0;
   do
   {
      if(m[n].text==NULL)
         level--;
      else if(m[n].fl==SUBM)
         level++;
      n++;
   }
   while(level>0 || m[n].text!=NULL);
   return((int*)&(m[n].func));
}

int   FirstItem(int n)
{
   int   level=0;

   while(level>0 || (n>0 && m[n-1].fl!=SUBM))
   {
      n--;
      if(m[n].text==NULL)
         level++;
      else if(m[n].fl==SUBM)
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
      else if(m[n].fl==SUBM)
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
      else if(m[n].fl==SUBM)
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
   PutCh(0,pos,ACS_LTEE);
   PutCh(Upper->w-1,pos,ACS_RTEE);
   for(i=Upper->w-2; i>0; i--)
      PutCh(i,pos,ACS_HLINE);
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
   DisplayWin((WIN*)(m[n].func));
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

void  CreateMenuWindow(int n,int x,int y)
{
   int   o,nw,
      width=0,
      height=0;

   n++;
   *CurrItem(n)=n;
   do
   {
      if((nw=ItemLen(m[n].text)+2)>width)
         width=nw;
      height++;
      if(m[n].fl==SUBM)
         CreateMenuWindow(n,x+2,y+height+1l);
      o=n;
      n=NextItem(n);
   }
   while(o<n);
   m[n-1].func=(void(*)())CreateWin(x,y+1,width,height+2,MENU_ATTR,"",0);
}

void  InitMenu()
{
   int   n,pos;

   RootWin=CreateWin(0,0,COLS,1,MENU_ATTR,"",NOSHADOW);
   n=0;
   pos=2;
   *CurrItem(0)=0;
   do
   {
      if(m[n].fl==SUBM)
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
            if(level==0 && m[curr].fl==SUBM)
               goto enter;
            n=MoveToValid(NextItem(curr),NextItem);
            if(n==-1)
               break;
            MoveBar(curr=n);
            break;
         case(NEWLINE):
   enter:      if(IsValid(curr)==0)
            {
               if(m[curr].fl==SUBM)
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
                  m[curr].func();
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
      if(level==0 && pd && m[curr].fl==SUBM)
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
