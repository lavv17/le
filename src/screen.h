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

#ifndef SCREEN_H
#define SCREEN_H

void  CenterView();
void  Redisplay(num line,offs ptr,num limit);
void  RedisplayAll();
void  RedisplayLine();
void  RedisplayAfter();
void  StatusLine();
void  LocateCursor();
void  SetCursor();
void  SyncTextWin();

void  Message(const char *msg);
void  MessageSync(const char *msg);
void  AddMessage(const char *msg);
void  ClearMessage();

void  ErrMsg(const char *msg);

void  TestPosition();

#define   HexPos    11
#define   AsciiPos   (HexPos+3*16+2)

extern int ScrollBarPos;

extern int TextWinX,TextWinY,TextWinWidth,TextWinHeight,ScrollBarX,StatusLineY;

extern int ShowScrollBar;
extern int ShowStatusLine;

extern int ShowMatchPos; // for hex mode

enum {
    SHOW_RIGHT=0,
    SHOW_NONE=1,
    SHOW_LEFT=2,
    SHOW_BOTTOM=0,
    SHOW_TOP=2
};

extern num ScrShift;

#endif//SCREEN_H
