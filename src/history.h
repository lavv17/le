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
#define  HISTORY_SIZE      128

class HistoryLine
{
   int	  len;
   char   *line;
   time_t cr_time;
   friend class History;

public:
   HistoryLine();
   HistoryLine(const HistoryLine &h);
   HistoryLine(const char *s,unsigned short len=0);

   bool equals(const char *s,int n) const { return(len==n && !memcmp(line,s,len)); }
   bool operator!=(const HistoryLine &h) const { return !equals(h.line,h.len); }
   const HistoryLine& operator=(const HistoryLine&);

   const char *get_line() const { return line; }
   int get_len() const { return len; }
};

class History
{
protected:
   HistoryLine **lines;
   int   curr;

public:
   History();

   void        Open();
   const HistoryLine *Next();
   const HistoryLine *Prev();
   const HistoryLine *Curr();
   void  operator+=(const HistoryLine&);
   void  operator-=(const HistoryLine&);
   void  operator+=(const HistoryLine *h) { *this+=*h; }
   void  operator-=(const HistoryLine *h) { *this-=*h; }
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

public:
   num      line,col,offset;

   InodeInfo(struct stat *st,num line=0,num col=0,num o=0);
   InodeInfo(const HistoryLine *line);
   InodeInfo();

   int   SameFile(const InodeInfo&) const;
   int   SameFileModified(const InodeInfo&) const;
   int   SameAndOlder(const InodeInfo&) const;

   const char *to_string() const;
};

class InodeHistory : public History
{
public:
   int FindInodeIndex(const InodeInfo&);
   const InodeInfo *FindInode(const InodeInfo&);
   void  operator+=(const InodeInfo&);
};
