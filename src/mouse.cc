/*
 * Copyright (c) 1993-2012 by Alexander V. Lukyanov (lav@yars.free.net)
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

#ifdef WITH_MOUSE

#include "edit.h"
#include "screen.h"
#include "mouse.h"
#include "block.h"

int UseMouse=true;

bool InTextWin(int line,int col)
{
   return (col>=TextWinX && line>=TextWinY
           && col-TextWinX<TextWinWidth && line-TextWinY<TextWinHeight);
}
bool InScrollBar(int line,int col)
{
   return (ShowScrollBar!=SHOW_NONE
	   && col>=ScrollBarX && line>=TextWinY
           && col-ScrollBarX<1 && line-TextWinY<TextWinHeight);
}

void MoveToScreenLC(int line,int col)
{
   if(in_hex_mode)
   {
      ascii=0;
      if(col<HexPos)
	 col=0;
      else if(col>=AsciiPos)
      {
	 ascii=1;
	 col-=AsciiPos;
      }
      else
	 col=(col-HexPos)/3;
      if(col>=16)
	 col=15;
      CurrentPos=ScreenTop+16*(line-TextWinY)+col;
   }
   else
   {
      CurrentPos=TextPoint(line-TextWinY+ScreenTop.Line(),
			   stdcol=col-TextWinX+ScrShift);
   }
}

void MouseInTextWin(MEVENT &mev)
{
// printf("\rbstate=0%lo          \n\r",mev.bstate);
   switch(mev.bstate)
   {
   case BUTTON1_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      break;
   case BUTTON1_DOUBLE_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      UserMarkWord();
      break;
   case BUTTON1_TRIPLE_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      UserMarkLine();
      break;
   case BUTTON2_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      UserCopyBlock();
      break;
   case BUTTON2_DOUBLE_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      UserMoveBlock();
      break;
   case BUTTON3_CLICKED:
      MoveToScreenLC(mev.y,mev.x);
      if(hide)
	 UserSetBlockBegin();
      if(CurrentPos>(BlockBegin+BlockEnd)/2)
      {
	 UserSetBlockEnd();
      	 if(DragMark)
	    *DragMark=BlockBegin;
      }
      else
      {
	 UserSetBlockBegin();
      	 if(DragMark)
	    *DragMark=BlockEnd;
      }
      break;
   case BUTTON1_PRESSED:
      mousemask(ALL_MOUSE_EVENTS|REPORT_MOUSE_POSITION,0);
      MoveToScreenLC(mev.y,mev.x);
      if(!DragMark)
	 UserStartDragMark();
      break;
   case BUTTON1_RELEASED:
      mousemask(ALL_MOUSE_EVENTS,0);
      MoveToScreenLC(mev.y,mev.x);
      if(DragMark)
      {
	 ProcessDragMark();
	 UserStopDragMark();
      }
      break;
   case BUTTON4_PRESSED:
   case BUTTON4_CLICKED:
      UserScrollUp();
      break;
#ifdef BUTTON5_PRESSED
   case BUTTON5_PRESSED:
   case BUTTON5_CLICKED:
#else
   // hack for old ncurses mouse ABI
   case 0x80:
   case 0x8000000:
#endif
      UserScrollDown();
      break;
   }
}

void MouseInScrollBar(MEVENT &mev)
{
   int pos=mev.y-TextWinY;
   switch(mev.bstate)
   {
   case BUTTON1_CLICKED:
      if(pos>ScrollBarPos)
	 UserPageDown();
      else if(pos<ScrollBarPos)
	 UserPageUp();
      break;
   }
}

void SetupMouse()
{
   if(UseMouse)
      mousemask(ALL_MOUSE_EVENTS,0);
   else
      mousemask(0,0);
}

#endif//WITH_MOUSE
