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

#ifndef KEYMAP_H
#define KEYMAP_H

#include "action-enum.h"

// some compatibility defines
#define CHAR_LEFT	A_BACKWARD_CHAR
#define CHAR_RIGHT	A_FORWARD_CHAR
#define LINE_UP		A_PREVIOUS_LINE
#define LINE_DOWN	A_NEXT_LINE
#define LINE_BEGIN	A_BEGINNING_OF_LINE
#define LINE_END	A_END_OF_LINE
#define CANCEL		A_ESCAPE
#define QUIT_EDITOR	A_ESCAPE
#define A_QUIT_EDITOR	A_ESCAPE
#define NEWLINE		A_NEW_LINE
#define NEXT_PAGE	A_NEXT_PAGE
#define PREV_PAGE	A_PREVIOUS_PAGE
#define EDITOR_HELP	A_HELP
#define REFRESH_SCREEN	A_REFRESH_SCREEN
#define BACKSPACE_CHAR	A_BACKWARD_DELETE_CHAR
#define DELETE_CHAR	A_DELETE_CHAR
#define DELETE_TO_EOL	A_DELETE_TO_EOL
#define CHOOSE_CHAR	A_CHOOSE_CHARACTER
#define ENTER_CHAR_CODE	   A_INSERT_CHAR_BY_CODE
#define ENTER_WCHAR_CODE   A_INSERT_WCHAR_BY_CODE
#define ENTER_CONTROL_CHAR A_QUOTED_INSERT
#define ENTER_MENU	A_ENTER_MENU
#define SAVE_FILE	A_SAVE_FILE_AS
#define SAVE_FILE_AS	A_SAVE_FILE

typedef  void  (*ActionProc)();

struct   ActionNameProcRec
{
   const char *name;
   ActionProc  proc;
};

struct   ActionCodeRec
{
   int   action;
   char	 *code;
};

extern unsigned char StringTyped[];
extern int   StringTypedLen;
extern const ActionCodeRec *ActionCodeTable;
extern const ActionCodeRec DefaultActionCodeTable[];

int   GetNextAction(void);
const char *GetActionString(int action);
void  ReadActionMap(FILE*);
void  WriteActionMap(FILE*);
ActionProc GetActionProc(int action);
void  EditorReadKeymap();
void  RebuildKeyTree();
void  FreeActionCodeTable();

int   FindActionCode(const char *);

const char *ShortcutPrettyPrint(int c);

void LoadKeymapEmacs();
void LoadKeymapDefault();
void SaveKeymap();
void SaveKeymapForTerminal();

#endif /* KEYMAP_H */
