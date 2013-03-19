/*
 * Copyright (c) 2001 by Alexander V. Lukyanov (lav@yars.free.net)
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

/* $Id: bm.cc,v 1.2 2001/05/16 14:42:28 lav Exp $ */

#include <config.h>
#include <assert.h>
#include "edit.h"
#include "bm.h"

#define N 256

static TextPoint *bm[N];
static TextPoint *bm_scrtop[N];
static num bm_scrshift[N];

void SetBookmark(int n)
{
   assert(n>=0 && n<N);
   if(bm[n])
   {
      *bm[n]=CurrentPos;
      *bm_scrtop[n]=ScreenTop;
   }
   else
   {
      bm[n]=new TextPoint(CurrentPos);
      bm_scrtop[n]=new TextPoint(ScreenTop);
   }
   bm_scrshift[n]=ScrShift;
}

void ClearBookmark(int n)
{
   assert(n>=0 && n<N);
   if(bm[n])
   {
      delete bm[n];
      bm[n]=0;
      delete bm_scrtop[n];
      bm_scrtop[n]=0;
   }
}

void GoBookmark(int n)
{
   assert(n>=0 && n<N);
   if(bm[n])
   {
      CurrentPos=*bm[n];
      stdcol=GetCol();
      ScreenTop=*bm_scrtop[n];
      ScrShift=bm_scrshift[n];
      flag=REDISPLAY_ALL;
   }
}

void ResetBookmarks()
{
   for(int i=0; i<N; i++)
      ClearBookmark(i);
}
