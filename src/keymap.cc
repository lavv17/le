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

char  StringTyped[256];
int   StringTypedLen;

int   FuncKeysNum=12;

int   MouseCounter=0;

ActionNameRec  ActionNameTable[]=
{
   {CHAR_LEFT,"backward-char"},
   {CHAR_RIGHT,"forward-char"},
   {WORD_LEFT,"backward-word"},
   {WORD_RIGHT,"forward-word"},
   {LINE_BEGIN,"beginning-of-line"},
   {LINE_END,"end-of-line"},
   {TEXT_BEGIN,"beginning-of-file"},
   {TEXT_END,"end-of-file"},
   {NEXT_PAGE,"next-page"},
   {PREV_PAGE,"previous-page"},
   {PAGE_TOP,"page-top"},
   {PAGE_BOTTOM,"page-bottom"},
   {TO_LINE_NUMBER,"to-line-number"},
   {TO_OFFSET,"to-offset"},
   {TO_PREVIOUS_LOC,"to-previous-edit"},
   {LINE_UP,"previous-line"},
   {LINE_DOWN,"next-line"},

// Delete actions
   {DELETE_CHAR,"delete-char"},
   {BACKSPACE_CHAR,"backward-delete-char"},
   {DELETE_WORD,"delete-word"},
   {BACKWARD_DELETE_WORD,"backward-delete-word"},
   {FORWARD_DELETE_WORD,"forward-delete-word"},
   {DELETE_TO_EOL,"delete-to-eol"},
   {DELETE_LINE,"delete-line"},
   {UNDELETE,"undelete"},

// Insert actions
   {INDENT,"indent"},
   {UNINDENT,"unindent"},
   {NEWLINE,"new-line"},
   {COPY_FROM_UP,"copy-from-up"},
   {COPY_FROM_DOWN,"copy-from-down"},

// File ops
   {LOAD_FILE,"load-file"},
   {SWITCH_FILE,"switch-file"},
   {SAVE_FILE,"save-file"},
   {SAVE_FILE_AS,"save-file-as"},
   {FILE_INFO,"file-info"},

// Block ops
   {COPY_BLOCK,"copy-block"},
   {MOVE_BLOCK,"move-block"},
   {DELETE_BLOCK,"delete-block"},
   {SET_BLOCK_END,"set-block-end"},
   {SET_BLOCK_BEGIN,"set-block-begin"},
   {READ_BLOCK,"read-block"},
   {WRITE_BLOCK,"write-block"},
   {PIPE_BLOCK,"pipe-block"},
   {INDENT_BLOCK,"indent-block"},
   {UNINDENT_BLOCK,"unindent-block"},
   {INSERT_PREFIX,"insert-prefix"},
   {TO_UPPER,"convert-to-upper"},
   {TO_LOWER,"convert-to-lower"},
   {EXCHANGE_CASE,"exchange-cases"},
   {BLOCK_HIDE,"hide-block"},
   {BLOCK_TYPE,"change-block-type"},
   {BLOCK_FUNC_BAR,"block-functions"},
   {MARK_LINE,"mark-line"},
   {MARK_TO_EOL,"mark-to-eol"},

// Search
   {SEARCH_FORWARD,"search-forward"},
   {SEARCH_BACKWARD,"search-backward"},
   {START_REPLACE,"start-replace"},
   {CONT_SEARCH,"continue-search"},
   {FIND_MATCH_BRACKET,"find-matching-bracket"},
   {FIND_BLOCK_BEGIN,"find-block-begin"},
   {FIND_BLOCK_END,"find-block-end"},

// Format
   {FORMAT_ONE_PARA,"format-paragraph"},
   {FORMAT_DOCUMENT,"format-document"},
   {CENTER_LINE,"center-line"},
   {AJUST_RIGHT_LINE,"ajust-right-line"},
   {FORMAT_FUNC_BAR,"format-functions"},

// Others
   {CALCULATOR,"calculator"},
   {DRAW_FRAMES,"draw-frames"},
   {TABS_EXPAND,"expand-tabs"},
   {TEXT_OPTIMIZE,"optimize-text"},
   {CHOOSE_CHAR,"choose-character"},
   {UNIX_DOS_TRANSFORM,"change-text-type"},

// Options
   {EDITOR_OPTIONS,"editor-options"},
   {TERMINAL_OPTIONS,"terminal-options"},
   {FORMAT_OPTIONS,"format-options"},
   {APPEARENCE_OPTIONS,"appearence-options"},
   {SAVE_OPTIONS,"save-options"},
   {SAVE_OPTIONS_LOCAL,"save-options-local"},

   {ENTER_CONTROL_CHAR,"quoted-insert"},
//   {ENTER_CHAR_CODE,UserEnterCharCode},

//   WINDOW_RESIZE,

   {EDITOR_HELP,"help"},
   {CONTEXT_HELP,"word-help"},

   {SUSPEND_EDITOR,"suspend-editor"},
   {QUIT_EDITOR,"escape"},

   {COMPILE_CMD,"compile"},
   {MAKE_CMD,"make"},
   {RUN_CMD,"make-run"},
   {SHELL_CMD,"shell-escape"},
   {ONE_SHELL_CMD,"shell-command"},

   {COMMENT_LINE,"comment-line"},

   {REFRESH_SCREEN,"refresh-screen"},

   {ENTER_MENU,"enter-menu"},

   {SWITCH_INSERT_MODE,"switch-insert-mode"},
   {SWITCH_HEX_MODE,"switch-hex-mode"},
   {SWITCH_AUTOINDENT_MODE,"switch-autoindent-mode"},
   {SWITCH_RUSSIAN_MODE,"switch-russian-mode"},
   {SWITCH_TEXT_MODE,"switch-text-mode"},
   {SWITCH_GRAPH_MODE,"switch-graph-mode"},

   {-1,NULL}
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
   struct KeyTreeNode *sibling;
   int keycode;
   struct KeyTreeNode *child;
};

extern   ActionCodeRec  DefaultActionCodeTable[];
ActionCodeRec  *ActionCodeTable=DefaultActionCodeTable;
//char  *ti_cache[128]={NULL};

char  *GetActionName(int action)
{
   for(int i=0; ActionNameTable[i].action!=-1; i++)
      if(ActionNameTable[i].action==action)
         return(ActionNameTable[i].name);
   return(NULL);
}

char  *GetActionCodeText(char *code)
{
   static char code_text[1024];
   char  *store=code_text;

   while(*code)
   {
      if(iscntrl(*code))
      {
         if(*code=='\033')
            sprintf(store,"\\e");
         else if(*code>=0 && *code<32)
            sprintf(store,"^%c",*code+'@');
         else
            sprintf(store,"\\%03o",*code);
         store+=strlen(store);
      }
      else
         *(store++)=*code;
      code++;
   }
   *store=0;
   return(code_text);
}

void  WriteActionMap(FILE *f)
{
   for(int i=0; ActionCodeTable[i].action!=-1; i++)
   {
      fprintf(f,"%-20s %s\n",GetActionName(ActionCodeTable[i].action),
                             GetActionCodeText(ActionCodeTable[i].code));
   }
}

ActionProc  GetActionProc(ActionProcRec *table,int action)
{
   for( ; table->action!=-1; table++)
      if(table->action==action)
         return(table->proc);
   return(NULL);
}

KeyTreeNode *AddToKeyTree(KeyTreeNode *curr,int key_code,int delay,int action)
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
      scan->child=0;
      scan->sibling=curr->child;
      curr->child=scan;
   }
   else
   {
      if(scan->action==NO_ACTION)
	 scan->action=action;
   }
   return(scan);
}

#define LEFT_BRACE  '{'
#define RIGHT_BRACE '}'

KeyTreeNode *BuildKeyTree(ActionCodeRec *ac_table)
{
   KeyTreeNode *top=0;
   char  term_name[256];
   char  *term_str;
   int   bracket;
   int   fk;

   top=new KeyTreeNode;
   top->keycode=-1;
   top->action=NO_ACTION;
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

	 char *code=ac_table->code;
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
	       while(*code!=0 && (bracket?*code==RIGHT_BRACE:isalnum(*code)) && term_str-term_name<255)
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
		     curr=AddToKeyTree(curr,(unsigned char)term_str[0],delay,NO_ACTION);
		     delay=HALF_DELAY;
		     term_str++;
		  }
		  key_code=term_str[0];
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
	       key_code=code_ch;
	       code++;
	    }

	    // now add the key_code to the tree
	    curr=AddToKeyTree(curr,key_code,delay,
			      (*code?NO_ACTION:ac_table->action));

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

void  ReadActionMap(FILE *f)
{
   char  ActionName[256];
   char  ActionCode[256];
   char  *store;
   int   ch;
   ActionCodeRec  *NewTable=NULL;
   int   CurrTableSize=0;
   int   CurrTableCell=0;
   int   action_no;

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

      /* find the named action in table */
      for(action_no=0; ActionNameTable[action_no].action!=-1
         && strcmp(ActionNameTable[action_no].name,ActionName); action_no++);

      /* is the action found in the table ? */
      if(ActionNameTable[action_no].action==-1)
         while(ch!='\n' && ch!=EOF)
            ch=fgetc(f);

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
            ch='\200';

         if(store-ActionCode<(int)sizeof(ActionCode)-1)
            *(store++)=ch;

         ch=fgetc(f);
         if(ch=='\n' || ch==EOF)
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
      NewTable[CurrTableCell].action=ActionNameTable[action_no].action;
      NewTable[CurrTableCell].code=strdup(ActionCode);
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
}

int   GetNextAction()
{
   char  *store;
   int   key;
   bool	 seen_func_key=false;

   store=StringTyped;
   StringTypedLen=0;
   *store=0;

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
	    while(getmouse(&mev)==OK)
	       ;  // flush mouse event queue
	    continue;
	 }
#endif

	 if(key>UCHAR_MAX)
	    seen_func_key=true;
	 else
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
	 return_action:
	    if(kt->action==REFRESH_SCREEN)
               clearok(stdscr,1); // force repaint for next refresh
	    timeout(-1);
	    return(kt->action);
	 }
	 kt=scan;
      }
   }
}
