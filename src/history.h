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

#undef	lines
#undef	cols

#define  HISTORY_LINE_LEN  256
#define  HISTORY_SIZE      32

class HistoryLine
{
public:
   unsigned short len;
   char     line[HISTORY_LINE_LEN];
   time_t   cr_time;

   HistoryLine();
   HistoryLine(char* s,unsigned short len=0);

   int   operator!=(const HistoryLine&) const;
   const HistoryLine&   operator=(const HistoryLine&);
};

class History
{
   HistoryLine lines[HISTORY_SIZE];
   int   curr;

public:
   History();

   void        Open();
   HistoryLine *Next();
   HistoryLine *Prev();
   HistoryLine *Curr();
   void  operator+=(const HistoryLine&);
   void  operator-=(const HistoryLine&);
   void  Push();
   void  ReadFrom(FILE*);
   void  WriteTo(FILE*);
   void  Merge(const History& h);
};

class InodeInfo
{
   ino_t    inode;
   dev_t    device;
   time_t   time;
   size_t   size;
   time_t   cr_time;

public:
   num      line,col,offset;

   InodeInfo(struct stat *st,num line=0,num col=0,num o=0);
   InodeInfo();

   int   SameFile(const InodeInfo&) const;
   int   SameFileModified(const InodeInfo&) const;
   int   SameAndOlder(const InodeInfo&) const;

   friend class   InodeHistory;
};

class InodeHistory
{
   InodeInfo   files[HISTORY_SIZE];

public:
   InodeHistory() {}
   InodeInfo   *FindInode(const InodeInfo&);
   void  operator+=(const InodeInfo&);

   void  WriteTo(FILE *f);
   void  ReadFrom(FILE *f);
   void  Merge(const InodeHistory& add);
};
