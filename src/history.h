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
   num      line,col;

   InodeInfo(struct stat *st,num line=0,num col=0);
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
