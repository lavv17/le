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
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#ifndef __MSDOS__
#include <term.h>
#else
extern "C" {
char *tigetstr(char *);
}
#endif
#include <poll.h>

char  StringTyped[256];
int   StringTypedLen;

int     FuncKeysNum=12;

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

extern   ActionCodeRec  DefaultActionCodeTable[];
ActionCodeRec  *ActionCodeTable=DefaultActionCodeTable;
char  *ti_cache[1024]={NULL};

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

char  *FindTermString(char *str)
{
   char  *ti_str;
   unsigned i;

   for(i=0; i<sizeof(ti_cache) && ti_cache[i]
                     && strcmp(ti_cache[i],str); i+=2);

   if(i>=sizeof(ti_cache) || ti_cache[i]==NULL)
   {
      ti_str=tigetstr(str);
      if(ti_str==NULL || ti_str==(char*)-1)
         return(NULL);
      if(i>=sizeof(ti_cache))
      {
         free(ti_cache[sizeof(ti_cache)-2]);
         memmove(ti_cache+2,ti_cache,(sizeof(ti_cache)-2)*sizeof(char*));
         ti_cache[i=0]=NULL;
      }
      else /* ti_cache[i]==NULL */
      {
         ti_cache[i]=strdup(str);
         ti_cache[i+1]=ti_str;
      }
   }
   return(ti_cache[i+1]);
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

static	string_has_ti;

int   CodeCompare(char *typed,int typedLen, char *code)
{
   register byte  code_ch;
   register byte  typed_ch;
   char  term_name[256];
   char  *term_str;
   int   bracket;
   int   shift,fk;

   string_has_ti=0;

   for(;;)
   {
      code_ch=*code;

      if(typedLen==0)
      {
         if(code_ch==0)
            return(CODE_EQUAL);
         if(code_ch=='|')
            return(CODE_PAUSE);
         return(CODE_PREFIX);
      }
      else if(code_ch==0)
         return(CODE_TOO_MUCH);

      typed_ch=*typed;

      switch(code_ch)
      {
      case('$'):
	 string_has_ti=1;

         code_ch=*(++code);

         if(code_ch==0)
            return(CODE_NOT_EQUAL);

         bracket=(code_ch=='{');
         code+=bracket;

	 term_str=term_name;
	 while(*code!=0 && (bracket?*code=='}':isalnum(*code)) && term_str-term_name<255)
	    *term_str++=*code++;
	 *term_str=0;
	 if(bracket && *code=='}')
	    code++;

         if(sscanf(term_name,"%1dkf%d",&shift,&fk)==2)
            sprintf(term_name,"kf%d",shift*FuncKeysNum+fk);

         term_str=FindTermString(term_name);
         if(term_str==NULL)
            return(CODE_NOT_EQUAL);

         for(;;)
         {
            code_ch=*term_str;
            if(typedLen==0)
            {
               if(code_ch==0)
               {
                  if(*code=='|')
                     return(CODE_PAUSE);
                  if(*code==0)
                     return(CODE_EQUAL);
               }
               return(CODE_PREFIX);
            }
            else if(code_ch==0)
               break;

            typed_ch=*typed;

            if(typed_ch!=code_ch && (typed_ch || code_ch!=128))
               return(CODE_NOT_EQUAL);

            typed++;
            typedLen--;
            term_str++;
         }
         break;
      case('|'):
         code++;
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
         if(code_ch!=typed_ch && (typed_ch || code_ch!=128))
            return(CODE_NOT_EQUAL);
         typed++;
	 typedLen--;
         code++;
      }
   }
}

int   GetNextAction()
{
   static   typeahead_hits=0;
   const    typeahead_max=8;

   char  *store;
   int   pause=1;
   int   prefix=0;
   int   action_found=NO_ACTION;
   int	 had_ti=0;
   int   key;
   int   delay;
   struct pollfd pfd;

   extern   resize_flag;

   pfd.fd=0;
   pfd.events=POLLIN;

   if(!(poll(&pfd,1,0)==1 && (pfd.revents&POLLIN) && typeahead_hits++<typeahead_max))
   {
      typeahead_hits=0;
      refresh();
      fflush(stdout);
   }

   store=StringTyped;
   StringTypedLen=0;
   *store=0;
   for(;;)
   {
      if(pause && action_found==NO_ACTION)
         delay=-1;
      else
         delay=500;

      if(resize_flag && prefix==0 && action_found==NO_ACTION)
      {
	 CheckWindowResize();
	 return WINDOW_RESIZE;
      }

      if(WaitForKey(delay)==ERR)
         key=ERR;
      else
         key=GetRawKey();

      if(key==ERR)
      {
         if(pause && !prefix)
            continue;
         return(action_found);
      }

      *(store++)=key;
      *store=0;
      StringTypedLen++;

scan_again:
      pause=0;
      prefix=0;
      for(int i=0; ActionCodeTable[i].action!=-1; i++)
      {
         switch(CodeCompare(StringTyped,StringTypedLen,ActionCodeTable[i].code))
         {
         case(CODE_EQUAL):
	    if(action_found==NO_ACTION || (!had_ti && string_has_ti))
	    {
               action_found=ActionCodeTable[i].action;
	       had_ti=string_has_ti;
	    }
            break;
         case(CODE_PAUSE):
            pause++;
            break;
         case(CODE_PREFIX):
            prefix++;
            break;
         case(CODE_TOO_MUCH):
            if(ActionCodeTable[i].action==action_found)
	    {
               action_found=NO_ACTION;
	       goto scan_again;
	    }
            break;
         }
      }
      if(prefix==0)
      {
         if(action_found!=NO_ACTION)
         {
            if(action_found==REFRESH_SCREEN)
               clearok(curscr,1);
            return(action_found);
         }
         if(pause==0)
         {
#ifndef __MSDOS__
            if(store-StringTyped>1)
               flushinp();
#endif
            return(action_found);
         }
      }
   }
}
