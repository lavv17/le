/*
 * Copyright (c) 2004 by Alexander V. Lukyanov (lav@yars.free.net)
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

#include <config.h>
#include "undo.h"

Undo undo;

Undo::Undo()
{
   chain_head=0;
   chain_ptr=0;
   chain_tail=0;
   group_open=0;
   current_group=0;
   enabled=true;
   locked=false;
   min_groups=4;
   max_size=0x10000000;
}
Undo::~Undo()
{
   while(chain_head)
   {
      Change *to_delete=chain_head;
      chain_head=chain_head->next;
      delete to_delete;
   }
}

void Undo::BeginUndoGroup()
{
   if(!group_open)
      current_group++;
   group_open++;
   group_pos=CurrentPos;
   group_stdcol=stdcol;
}
void Undo::AddChange(Change *c)
{
   if(!Enabled())
   {
      delete c;
      return;
   }
   while(chain_ptr)
   {
      if(chain_ptr==chain_tail)
	 chain_tail=chain_ptr->prev;
      if(chain_ptr==chain_head)
	 chain_head=chain_ptr->next;
      if(chain_ptr->next)
	 chain_ptr->next->prev=chain_ptr->prev;
      if(chain_ptr->prev)
	 chain_ptr->prev->next=chain_ptr->next;
      Change *to_delete=chain_ptr;
      chain_ptr=chain_ptr->next;
      delete to_delete;
   }
   if(!group_open)
   {
      current_group++;
      group_pos=CurrentPos;
      group_stdcol=stdcol;
   }
   c->group=current_group;
   c->group_pos=group_pos;
   c->group_stdcol=group_stdcol;
   c->prev=chain_tail;
   c->next=0;
   if(chain_tail)
      chain_tail->next=c;
   chain_tail=c;
   if(!chain_head)
      chain_head=c;
   group_pos=-1;
   group_stdcol=-1;
}
void Undo::EndUndoGroup()
{
   if(group_open<=0)
      return;
   group_open--;
   CheckSize();
   GlueGroup();
}

Undo::Change::Change(type_t t,const char *l,num ls,const char *r,num rs)
{
   left=(char*)malloc(ls);
   if(!left)
      return;
   right=(char*)malloc(rs);
   if(!right)
   {
      free(left);
      left=0;
      return;
   }
   type=t;
   left_size=ls;
   right_size=rs;
   memcpy(left,l,ls);
   memcpy(right,r,rs);

   pos=CurrentPos;
   old_modified=modified;

   next=prev=0;
}

void Undo::UndoGroup()
{
   if(!chain_tail)
      return;
   if(!chain_ptr)
      chain_ptr=chain_tail;
   else
   {
      if(!chain_ptr->prev)
	 return;
      chain_ptr=chain_ptr->prev;
   }
   unsigned g=chain_ptr->group;
   locked=true;
   for(;;)
   {
      chain_ptr->Undo();
      if(chain_ptr->prev==0 || chain_ptr->prev->group!=g)
	 break;
      chain_ptr=chain_ptr->prev;
   }
   CurrentPos=chain_ptr->group_pos;
   stdcol=chain_ptr->group_stdcol;
   locked=false;
}
void Undo::RedoGroup()
{
   if(!chain_ptr)
      return;
   unsigned g=chain_ptr->group;
   locked=true;
   for(;;)
   {
      chain_ptr->Redo();
      chain_ptr=chain_ptr->next;
      if(!chain_ptr || chain_ptr->group!=g)
	 break;
   }
   locked=false;
}

void Undo::Change::Undo()
{
   switch(type)
   {
   case DELETE:
      CurrentPos=pos-left_size;
      InsertBlock(left,left_size,right,right_size);
      break;
   case INSERT:
      CurrentPos=pos+left_size;
      DeleteBlock(left_size,right_size);
      break;
   case REPLACE:
      CurrentPos=pos;
      ReplaceBlock(left,left_size);
      break;
   }
   modified=old_modified;
}
void Undo::Change::Redo()
{
   CurrentPos=pos;
   switch(type)
   {
   case DELETE:
      DeleteBlock(left_size,right_size);
      break;
   case INSERT:
      InsertBlock(left,left_size,right,right_size);
      break;
   case REPLACE:
      ReplaceBlock(right,right_size);
      break;
   }
}

void Undo::CheckSize()
{
   Change *scan=0;
   if(chain_ptr)
      scan=chain_ptr->prev;
   else
      scan=chain_tail;
   if(!scan)
      return;
   unsigned g=scan->group;
   int count=1;
   num size=0;
   while(scan)
   {
      size+=scan->GetSize();
      scan=scan->prev;
      if(!scan)
	 break;
      if(scan->group!=g)
      {
	 g=scan->group;
	 count++;
	 if(size>=max_size && count>min_groups)
	 {
	    // cut the undo list.
	    scan->next->prev=0;
	    chain_head=scan->next;
	    while(scan)
	    {
	       Change *to_delete=scan;
	       scan=scan->prev;
	       delete to_delete;
	    }
	    break;
	 }
      }
   }
}

/* Check if previous group was the same type and position, e.g. when typing
 * letters or deleting several characters at the same position. Glue the
 * two groups together if they match. */
void Undo::GlueGroup()
{
#if 0 // NOT DONE YET
   Change *scan=chain_tail;
   if(!scan)
      return;
   unsigned g=scan->group;
   Change::type_t t=scan->type;
   offs pos=scan->pos;
   while(scan && scan->group==g)
   {
      if(scan->type!=t && scan->type!=Change::POSITION)
	 return;
      scan=scan->prev;
      // check if pos glues together
      ...
   }
   if(!scan)
      return;
   Change *to_glue=scan->next;
   unsigned g=scan->group;
   while(scan && scan->group==g)
   {
      if(scan->type!=t && scan->type!=Change::POSITION)
	 return;
      scan=scan->prev;
      // check if pos glues together
      ...
   }
#endif
}
