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

#ifndef CLIPBRD_H
#define CLIPBRD_H

class ClipBoard
{
   char *text;
   int width;
   int height;
   bool rect;
   bool	toeol;

public:
   ClipBoard();
   ~ClipBoard();

   void Empty();
   int Copy();
   int Paste(bool mark=false);
   int PasteAndMark() { return Paste(true); }

   int Write(int fd);
   int Linearize(char **buf,int *len);
};

extern ClipBoard MainClipBoard;

#endif  CLIPBRD_H
