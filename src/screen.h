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
void  ErrMsg(const char *msg);

void  TestPosition();

extern WINDOW *text_w;

#endif SCREEN_H
