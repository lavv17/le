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

/* functions, invoked with the keyboard by the user */

void  UserDeleteToEol();
void  UserDeleteLine();
void  UserDeleteWord();
void  UserForwardDeleteWord();
void  UserBackwardDeleteWord();

void  UserCopyFromDown();
void  UserCopyFromUp();

void  UserDeleteBlock();
void  UserCopyBlock();
void  UserMoveBlock();
void  UserMarkWord();
void  UserMarkLine();
void  UserMarkToEol();
void  UserMarkAll();

void  UserLineUp();
void  UserLineDown();
void  UserCharLeft();
void  UserCharRight();
void  UserPageTop();
void  UserPageUp();
void  UserPageBottom();
void  UserPageDown();
void  UserWordLeft();
void  UserWordRight();
void  UserLineBegin();
void  UserLineEnd();

void  UserMarkCharLeft();
void  UserMarkCharRight();
void  UserMarkWordLeft();
void  UserMarkWordRight();
void  UserMarkLineBegin();
void  UserMarkLineEnd();
void  UserMarkFileBegin();
void  UserMarkFileEnd();
void  UserMarkPageDown();
void  UserMarkPageUp();
void  UserMarkPageTop();
void  UserMarkPageBottom();
void  UserMarkLineUp();
void  UserMarkLineDown();

void  UserMenu();

void  UserCommentLine();

void  UserSetBlockBegin();
void  UserSetBlockEnd();
void  UserFindBlockBegin();
void  UserFindBlockEnd();
void  UserPipeBlock();

void  UserFileBegin();
void  UserFileEnd();

void  UserPreviousEdit();

void  UserBackSpace();
void  UserDeleteChar();

void  UserLoad();
int   UserSave();
void  UserSwitch();
int   UserSaveAs();

void  UserInfo();

void  UserToLineNumber();
void  UserToOffset();

void  UserIndent();
void  UserUnindent();

void  UserAutoindent();
void  UserNewLine();

void  UserUndelete();

void  UserEnterControlChar();

void  UserWordHelp();
void  UserKeysHelp();
void  UserAbout();

void  UserRefreshScreen();

void  UserChooseChar();

void  UserInsertChar(char ch);
void  UserInsertControlChar(char ch);

void  UserSwitchHexMode();
void  UserSwitchTextMode();
void  UserSwitchInsertMode();
void  UserSwitchAutoindentMode();
void  UserSwitchRussianMode();
void  UserSwitchGraphMode();

void  UserBlockPrefixIndent();

void  UserShellCommand();

void  UserYankBlock();
void  UserRememberBlock();

void  UserStartDragMark();
void  UserStopDragMark();

void  UserOptimizeText();
