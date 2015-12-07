/*
 * Copyright (c) 1993-2013 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include "edit.h"
#include "keymap.h"
#include "keynames.h"
#include "getch.h"
#include <term.h>

#include "block.h"
#include "options.h"
#include "keymap.h"
#include "format.h"
#include "search.h"
#include "colormnu.h"

unsigned char StringTyped[256];
int   StringTypedLen;
int   LastActionCode;
const char *ActionArgument;
int   ActionArgumentLen;

int   FuncKeysNum=12;

int   MouseCounter=0;

const ActionNameProcRec ActionNameProcTable[]=
{
#include "action-name-func.h"
};

const struct {
   const char *alias;
   int code;
} ActionNameAliases[]={
   "quit-editor", A_ESCAPE,
   0
};

enum
{
   CODE_EQUAL,
   CODE_PREFIX,
   CODE_PAUSE,
   CODE_NOT_EQUAL,
   CODE_TOO_MUCH
};

const int MAX_DELAY=30000000;
const int HALF_DELAY=500;

struct KeyTreeNode
{
   int maxdelay;
   int action;
   int keycode;
   struct KeyTreeNode *sibling;
   struct KeyTreeNode *child;
   const char *arg;
};

const ActionCodeRec *ActionCodeTable=DefaultActionCodeTable;
ActionCodeRec *DynamicActionCodeTable;

const char *GetActionName(int action)
{
   if(action>=A__FIRST && action<=A__LAST)
      return ActionNameProcTable[action-A__FIRST].name;
   return(NULL);
}

const char *GetActionCodeText(const char *code)
{
   static char code_text[1024];
   char  *store=code_text;

   while(*code)
   {
      unsigned char the_code=*code++;
      if(iscntrl(the_code))
      {
         if(the_code=='\033')
            sprintf(store,"\\e");
         else if(the_code<32)
            sprintf(store,"^%c",the_code+'@');
         else
            sprintf(store,"\\%03o",the_code);
         store+=strlen(store);
      }
      else
         *(store++)=the_code;
   }
   *store=0;
   return(code_text);
}

#define LEFT_BRACE  '{'
#define RIGHT_BRACE '}'

static int PrettyCodeScore(const char *c)
{
   if(c==0)
      return 1000000;

   int score=0;
   while(*c)
   {
      score++;

      char  term_name[256];
      char  *term_str;
      int   bracket;
      int   fk;
      int   shift;
      char code_ch=*c;
      switch(code_ch)
      {
      case('$'):
	 code_ch=*(++c);

	 if(code_ch==0)
	    break;

	 bracket=(code_ch==LEFT_BRACE);
	 c+=bracket;

	 term_str=term_name;
	 while(*c!=0 && (bracket?*c!=RIGHT_BRACE:isalnum((unsigned char)*c)) && term_str-term_name<255)
	    *term_str++=*c++;
	 *term_str=0;
	 if(!(bracket && *c==RIGHT_BRACE))
	    c--;

	 shift=0;
	 if(sscanf(term_name,"%1dkf%d",&shift,&fk)==2
	 || sscanf(term_name,"kf%d",&fk)==1)
	 {
	    if(shift)
	       score+=2+2*shift;
	    else
	       score+=2;
	    if(shift)
	       sprintf(term_name,"kf%d",shift*FuncKeysNum+fk);
	 }
	 else
	    score+=8;
	 term_str=tigetstr(term_name);
	 if(term_str==(char*)-1 || !term_str || !*term_str)
	    return 1000000;
	 break;
      case('|'):
	 score+=5;
	 break;
      case('^'):
      case('\\'):
	 break;
      case('\e'):
	 if(c[1]=='[') // terminal codes are not pretty
	    score+=6;
	 break;
      }
      c++;
   }
   return score;
}

const char *ActionCodePrettyPrint(const char *c)
{
   static char code_text[1024];
   char  *store=code_text;
   *store=0;

   while(*c)
   {
      char  term_name[256];
      char  *term_str;
      int   bracket;
      int   fk;
      int   shift;
      unsigned char code_ch=*c;
      switch(code_ch)
      {
      case('$'):
	 code_ch=*(++c);

	 if(code_ch==0)
	    break;

	 bracket=(code_ch==LEFT_BRACE);
	 c+=bracket;

	 term_str=term_name;
	 while(*c!=0 && (bracket?*c!=RIGHT_BRACE:isalnum((unsigned char)*c)) && term_str-term_name<255)
	    *term_str++=*c++;
	 *term_str=0;
	 if(!(bracket && *c==RIGHT_BRACE))
	    c--;

	 shift=0;
	 if((sscanf(term_name,"%1dkf%d",&shift,&fk)==2
	  || sscanf(term_name,"kf%d",&fk)==1) && shift<4)
	 {
	    static char shift_str_map[][3]={"","~","^","~^"};
	    store+=sprintf(store,"%sF%d",shift_str_map[shift],fk);
	 }
	 else
	 {
	    // FIXME.
	    store+=sprintf(store,"%s",term_name);
	 }
	 if(c[1] && c[1]!='|')
	 {
	    *store++=' ';
	    *store=0;
	 }
	 break;
      case('|'):
	 *store++='+';
	 *store=0;
	 break;
      case('^'):
	 if(c[1])
	 {
	    *store++='^';
	    *store++=toupper(*++c);
	    *store=0;
	    break;
	 }
	 goto default_l;
      case('\\'):
	 code_ch=*(++c);
      default:
      default_l:
	 if(code_ch==27 && c[1]=='|' && c[2] && c[2]!='$')
	 {
	    *store++='M';
	    *store++='-';
	    *store=0;
	    c++;
	 }
	 else if(code_ch<32)
	 {
	    *store++='^';
	    *store++=code_ch+'@';
	 }
	 else if(code_ch==128)
	 {
	    *store++='^';
	    *store++='@';
	 }
	 else
	    *store++=code_ch;
	 *store=0;
      }
      c++;
   }
   return code_text;
}

const char *ShortcutPrettyPrint(int c,const char *arg)
{
   static char code_text[1024];
   char  *store=code_text;

   const char *best_code=0;
   int best_score=1000000;
   for(int i=0; ActionCodeTable[i].action!=-1; i++)
   {
      if(ActionCodeTable[i].action!=c || xstrcmp(ActionCodeTable[i].arg,arg))
	 continue;
      const char *code=ActionCodeTable[i].code;
      int score=PrettyCodeScore(code);
      if(score<best_score)
      {
	 best_code=code;
	 best_score=score;
      }
   }
   if(best_code==0)
      return 0;

   strcpy(store,ActionCodePrettyPrint(best_code));
   return code_text;
}

void  WriteActionMap(FILE *f)
{
   for(int i=0; ActionCodeTable[i].action!=-1; i++)
   {
      int pos=0;
      const char *a_name=GetActionName(ActionCodeTable[i].action);
      fputs(a_name,f);
      pos+=strlen(a_name);
      const char *arg=ActionCodeTable[i].arg;
      if(arg) {
	 fputc('(',f),pos++;
	 while(*arg) {
	    char out=*arg;
	    char bsout=0;
	    switch(*arg) {
	    case '\n': bsout='n'; break;
	    case '\r': bsout='r'; break;
	    case '\t': bsout='t'; break;
	    case '_':
	    case '\\': bsout=*arg; break;
	    case ' ': out='_'; break;
	    }
	    if(bsout)
	       fputc('\\',f),pos++;
	    fputc(bsout?bsout:out,f),pos++;
	    arg++;
	 }
	 fputc(')',f),pos++;
      }
      fputc(' ',f),pos++;
      while(pos<23)
	 fputc(' ',f),pos++;
      fputs(GetActionCodeText(ActionCodeTable[i].code),f);
      putc('\n',f);
   }
}

ActionProc GetActionProc(int action)
{
   if(action>=A__FIRST && action<=A__LAST)
      return ActionNameProcTable[action-A__FIRST].proc;
   return(NULL);
}

static KeyTreeNode *AddToKeyTree(KeyTreeNode *curr,int key_code,int delay,int action,const char *arg)
{
   KeyTreeNode *scan;
   for(scan=curr->child; scan; scan=scan->sibling)
      if(scan->keycode==key_code)
	 break;
   if(!scan)
   {
      scan=new KeyTreeNode;
      scan->maxdelay=delay;
      scan->keycode=key_code;
      scan->action=action;
      scan->arg=arg;
      scan->child=0;
      scan->sibling=curr->child;
      curr->child=scan;
   }
   else
   {
      if(scan->action==NO_ACTION) {
	 scan->action=action;
	 scan->arg=arg;
      }
   }
   return(scan);
}

#define LEFT_BRACE  '{'
#define RIGHT_BRACE '}'

KeyTreeNode *BuildKeyTree(const ActionCodeRec *ac_table)
{
   KeyTreeNode *top=0;
   char  term_name[256];
   char  *term_str;
   int   bracket;
   int   fk;

   top=new KeyTreeNode;
   top->keycode=-1;
   top->action=NO_ACTION;
   top->arg=0;
   top->maxdelay=MAX_DELAY;
   top->child=0;
   top->sibling=0;

   while(ac_table->action!=-1)
   {
      int fk_mask=0;
      int fk_num=0;
      while(fk_mask < (1<<fk_num))
      {
	 KeyTreeNode *curr=top;

	 const char *code=ac_table->code;
	 int delay=MAX_DELAY;

	 fk_num=0;
	 while(*code)
	 {
	    int shift=0;
	    int key_code=0;

	    char code_ch=*code;
	    switch(code_ch)
	    {
	    case('$'):
	       code_ch=*(++code);

	       if(code_ch==0)
		  break;

	       bracket=(code_ch==LEFT_BRACE);
	       code+=bracket;

	       term_str=term_name;
	       while(*code!=0 && (bracket?*code!=RIGHT_BRACE:isalnum((unsigned char)*code)) && term_str-term_name<255)
		  *term_str++=*code++;
	       *term_str=0;
	       if(bracket && *code==RIGHT_BRACE)
		  code++;

	       if(sscanf(term_name,"%1dkf%d",&shift,&fk)==2)
		  sprintf(term_name,"kf%d",shift*FuncKeysNum+fk);

	       if((fk_mask&(1<<fk_num))==0)
	       {
	       fallback:
		  key_code=FindKeyCode(term_name);
	       }
	       else
	       {
		  term_str=tigetstr(term_name);
		  if(term_str==NULL || term_str==(char*)-1)
	       	     goto fallback;
	       	  while(term_str[0] && term_str[1])
		  {
		     curr=AddToKeyTree(curr,(unsigned char)term_str[0],delay,NO_ACTION,NULL);
		     delay=HALF_DELAY;
		     term_str++;
		  }
		  key_code=(unsigned char)term_str[0];
		  if(key_code==0)
		     goto fallback;
	       }
	       fk_num++;
	       break;
	    case('|'):
	       delay=MAX_DELAY;
	       code++;
	       continue;
	       break;
	    case('^'):
	       if(code[1])
	       {
		  code_ch=toupper(*++code)-'@';
		  if(!code_ch)
		     code_ch|=0200;
	       }
	       goto default_l;
	    case('\\'):
	       code_ch=*(++code);
	    default:
	    default_l:
	       key_code=(unsigned char)code_ch;
	       code++;
	    }

	    // now add the key_code to the tree
	    curr=AddToKeyTree(curr,key_code,delay,
			      (*code?NO_ACTION:ac_table->action),
			      (*code?NULL:ac_table->arg));
	    delay=HALF_DELAY;
	 }

      	 fk_mask++;
      }
      ac_table++;
   }

   return top;
}

KeyTreeNode    *KeyTree=0;

void FreeKeyTree(KeyTreeNode *kt)
{
   if(!kt)
      return;
   FreeKeyTree(kt->sibling);
   FreeKeyTree(kt->child);
   delete kt;
}

void RebuildKeyTree()
{
   FreeKeyTree(KeyTree);
   KeyTree=BuildKeyTree(ActionCodeTable);
}

int FindActionCode(const char *ActionName)
{
   int lo=A__FIRST;
   int hi=A__LAST+1;

   while(lo<hi) {
      int mid=(lo+hi)/2;
      int cmp=strcmp(ActionName,GetActionName(mid));
      if(cmp==0)
	 return mid;
      if(cmp>0)
	 lo=mid+1;
      else
	 hi=mid;
   }

   // try aliases (there are few, no need for bsearch)
   for(int i=0; ActionNameAliases[i].alias; i++)
      if(!strcmp(ActionName,ActionNameAliases[i].alias))
	 return ActionNameAliases[i].code;

   return -1;
}

int ParseActionNameArg(char *action,const char **arg)
{
      // extract the action parameter
      *arg=NULL;
      char *end=action+strlen(action);
      char *par1=strchr(action,'(');
      if(par1 && end[-1]==')') {
	 *par1=0;
	 end[-1]=0;
	 *arg=par1+1;
      }
      // convert the action name to code
      return FindActionCode(action);
}

char *ParseActionArgumentAlloc(const char *arg)
{
   if(!arg || !*arg)
      return NULL;
   char *alloc=(char*)malloc(strlen(arg)+1);
   char *store=alloc;
   while(*arg) {
      switch(*arg) {
      case '_':
	 *store++=' ';
	 arg++;
	 break;
      case '\\':
	 arg++;
	 switch(*arg) {
	 case '\0':
	    *store++='\\';
	    break;
	 case 'n': *store++='\n'; arg++; break;
	 case 'r': *store++='\r'; arg++; break;
	 case 't': *store++='\t'; arg++; break;
	 default:
	    *store++=*arg++;
	 }
	 break;
      default:
	 *store++=*arg++;
      }
   }
   *store=0;
   return alloc;
}

void  ReadActionMap(FILE *f)
{
   FreeActionCodeTable();

   char  ActionName[1024];
   const char *ActionArg;
   char  ActionCode[256];
   char  *store;
   int   ch;
   ActionCodeRec  *NewTable=NULL;
   int   CurrTableSize=0;
   int   CurrTableCell=0;

   for(;;)  /* line cycle */
   {
      store=ActionName;
      for(;;)  /* action name cycle */
      {
         ch=fgetc(f);
         if(ch==EOF || isspace(ch))
            break;
         if(store-ActionName<(int)sizeof(ActionName)-1)
            *(store++)=ch;
      }
      *store=0;

      int action_found=ParseActionNameArg(ActionName,&ActionArg);
      if(action_found==-1)
      {
         while(ch!='\n' && ch!=EOF)
            ch=fgetc(f);
	 if(ch==EOF)
	    break;
	 continue;
      }

      /* skip spaces between action name and action code */
      while(ch!='\n' && ch!=EOF && isspace(ch))
         ch=fgetc(f);

      if(ch==EOF || ch=='\n')
         break;

      store=ActionCode;
      for(;;)
      {
         if(ch=='\\')
         {
            ch=fgetc(f);
            switch(ch)
            {
            case('e'):
               ch=27;
               break;
            case('n'):
               ch=10;
               break;
            case('r'):
               ch=13;
               break;
            case('t'):
               ch=9;
               break;
            case('b'):
               ch=8;
               break;
            default:
               if(isdigit(ch))
               {
                  ungetc(ch,f);
                  fscanf(f,"%3o",&ch);
               }
               else
               {
                  ungetc(ch,f);
                  ch='\\';
               }
            }
         }
         if(ch=='\000')
            ch=128;

         if(store-ActionCode<(int)sizeof(ActionCode)-1)
            *(store++)=ch;

         ch=fgetc(f);
         if(ch==EOF || isspace(ch))
            break;
      }
      *store=0;

      if(CurrTableSize<=CurrTableCell)
      {
         if(NewTable==NULL)
            NewTable=(ActionCodeRec*)malloc((CurrTableSize=16)*sizeof(*NewTable));
         else
            NewTable=(ActionCodeRec*)realloc(NewTable,(CurrTableSize*=2)*sizeof(*NewTable));
         if(!NewTable)
         {
            fprintf(stderr,"le: Not enough memory!\n");
            exit(1);
         }
      }
      NewTable[CurrTableCell].action=action_found;
      NewTable[CurrTableCell].code=strdup(ActionCode);
      NewTable[CurrTableCell].arg=ParseActionArgumentAlloc(ActionArg);
      if(NewTable[CurrTableCell].code==NULL)
      {
         fprintf(stderr,"le: Not enough memory!\n");
         exit(1);
      }
      CurrTableCell++;
   }

   NewTable=(ActionCodeRec*)realloc(NewTable,(CurrTableCell+1)*sizeof(*NewTable));
   if(!NewTable)
   {
      fprintf(stderr,"le: Not enough memory!\n");
      exit(1);
   }
   NewTable[CurrTableCell].action=-1;
   NewTable[CurrTableCell].code=NULL;

   ActionCodeTable=NewTable;
   DynamicActionCodeTable=NewTable;
}

void FreeActionCodeTable()
{
   if(DynamicActionCodeTable)
   {
      for(int i=0; DynamicActionCodeTable[i].code; i++)
	 free(DynamicActionCodeTable[i].code);
      free(DynamicActionCodeTable);
      DynamicActionCodeTable=0;
   }
   ActionCodeTable=0;
}

int   GetNextAction()
{
   unsigned char *store;
   int   key;
   static KeyTreeNode kt_mb = { HALF_DELAY, NO_ACTION, -1, NULL,  NULL, NULL };

   store=StringTyped;
   StringTypedLen=0;
   *store=0;
   ActionArgument=NULL;

   KeyTreeNode *kt=KeyTree;

   for(;;)  // loop for a whole key sequence
   {
      int time_passed=0;

      for(;;)  // loop for one key
      {
	 int delay=-1;
	 KeyTreeNode *scan;

	 for(scan=kt->child; scan; scan=scan->sibling)
	    if(delay==-1 ||
	       (delay>scan->maxdelay && scan->maxdelay<time_passed))
	       delay=scan->maxdelay;

	 if(delay==MAX_DELAY)
	    key=GetKey();
	 else if(delay==-1 || time_passed>=delay)
	    goto return_action;
	 else
	    key=GetKey(delay-time_passed);

#ifdef KEY_RESIZE
	 if(key==KEY_RESIZE)
	    return WINDOW_RESIZE;
#else // !KEY_RESIZE
	 extern int resize_flag;
	 if(resize_flag && kt==KeyTree)
	 {
	    if(key!=ERR)
	       ungetch(key);
	    CheckWindowResize();
	    return WINDOW_RESIZE;
	 }
#endif // !KEY_RESIZE

	 if(key==ERR)
	 {  // no key in the time interval
	    time_passed=delay;
	    continue;
	 }

#ifdef WITH_MOUSE
	 if(key==KEY_MOUSE)
	 {
	    if(kt==KeyTree)
	       return MOUSE_ACTION;
	    MEVENT mev;
	    int limit=100; // workaround for ncurses bug
	    while(getmouse(&mev)==OK && limit-->0)
	       ;  // flush mouse event queue
	    continue;
	 }
#endif

	 if(key<=UCHAR_MAX)
	 {
	    *(store++)=key;
	    *store=0;
	    StringTypedLen++;
	 }

	 for(scan=kt->child; scan; scan=scan->sibling)
	    if(scan->keycode==key || (key==0 && scan->keycode==128))
	       break;
	 if(!scan)
	 {
	    if(StringTypedLen>1 && kt->action==NO_ACTION && StringTyped[0]<32) {
	    // We've got an unknown sequence.
	    // It is likely that it is a bit longer that we've already got,
	    // so try to flush it.
	       napms(10);
	       flushinp();
	    }
#if USE_MULTIBYTE_CHARS
	    // check for partial mb chars
	    if(mb_mode && StringTypedLen>0 && kt->action==NO_ACTION && StringTyped[0]>=128) {
	       mbtowc(0,0,0);
	       wchar_t wc;
	       int mb_size=mbtowc(&wc,(const char*)StringTyped,StringTypedLen);
	       if(mb_size<=0) {
		  kt=&kt_mb;
		  continue;
	       }
	    }
#endif
	 return_action:
	    if(kt->action==REFRESH_SCREEN)
               clearok(stdscr,1); // force repaint for next refresh
	    timeout(-1);
	    ActionArgument=kt->arg;
	    ActionArgumentLen=xstrlen(ActionArgument);
	    return(LastActionCode=kt->action);
	 }
	 kt=scan;
      }
   }
}

const char *GetActionArgument(const char *prompt,History* history,const char *help,const char *title)
{
   static char *buf[A__LAST-A__FIRST+1];
   static int buf_len[A__LAST-A__FIRST+1];
   if(ActionArgument)
      return ActionArgument;
   char **b=buf+LastActionCode-A__FIRST;
   int *len=buf_len+LastActionCode-A__FIRST;
   const int maxlen=256;
   if(!*b)
      *b=(char*)malloc(maxlen);
   if(!*b) {
      NoMemory();
      return NULL;
   }
   int res=getstring(prompt,*b,maxlen-1,history,len,help,title);
   if(res==-1)
      return NULL;
   ActionArgument=*b;
   ActionArgumentLen=*len;
   return *b;
}
