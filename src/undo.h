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

#include "edit.h"
#include <stdlib.h>

class Undo
{
   struct GroupHead
   {
      offs pos;
      num stdcol;
      offs block_begin;
      offs block_end;
      bool block_hide;

      GroupHead();
      void Undo();
   };
   class Change
   {
      friend class Undo;
      unsigned group;
      GroupHead *group_head;
      Change *next;
      Change *prev;
      bool Join(const Change *);
   protected:
      enum type_t { INSERT, DELETE, REPLACE } type;
      offs pos;
      char *left;
      num left_size;
      char *right;
      num right_size;
      bool old_modified;
   public:
      Change(type_t t,const char *l,num ls,const char *r,num rs);
      ~Change() { free(left); free(right); }
      size_t GetSize()
	 {
	    return (left?left_size:0)
		  +(right?right_size:0)
		  +(group_head?sizeof(*group_head):0)
		  +sizeof(Change);
	 }
      void Undo();
      void Redo();
   };

   Change *chain_head;
   Change *chain_ptr;
   Change *chain_tail;
   int group_open;
   unsigned current_group;
   GroupHead *group_head;

   bool locked;
   bool enabled;

   bool glue_changes;
   num max_size;
   int min_groups;

   void CheckSize();

public:
   struct Delete : public Change {
      Delete(const char *l,num ls,const char *r,num rs) : Change(DELETE,l,ls,r,rs) {} };
   struct Insert : public Change {
      Insert(const char *l,num ls,const char *r,num rs) : Change(INSERT,l,ls,r,rs) {} };
   struct Replace : public Change {
      Replace(const char *l,num ls,const char *r,num rs) : Change(REPLACE,l,ls,r,rs) {} };

   void BeginUndoGroup();
   void AddChange(Change *);
   void EndUndoGroup();

   void UndoGroup();
   void RedoGroup();
   void UndoOne();
   void RedoOne();

   void Clear();

   Undo();
   ~Undo();

   bool Enabled() { return enabled&&!locked; }
   bool Locked()  { return locked; }

   void SetMaxSize(num ms) { max_size=ms; }
   void SetMinGroups(int mg) { min_groups=mg; }
   void SetEnable(bool en) { if(enabled!=en) Clear(); enabled=en; }
   void SetGlue(bool g) { glue_changes=g; }
};

extern Undo undo;
