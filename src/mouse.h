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

/* $Id$ */

#ifndef MOUSE_H
#define MOUSE_H

#ifdef WITH_MOUSE

bool InTextWin(int line,int col);
bool InScrollBar(int line,int col);
void MoveToScreenLC(int line,int col);
void MouseInTextWin(MEVENT &mev);
void MouseInScrollBar(MEVENT &mev);

extern int UseMouse;

void SetupMouse();

#endif

#endif // MOUSE_H
