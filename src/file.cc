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

#include <sys/types.h>
#include <stdlib.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <string.h>
#include <sys/fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <fnmatch.h>

#include "edit.h"
#include "keymap.h"

#ifndef DIRSIZ
#define DIRSIZ 14
#endif

#ifndef HAVE_STRCOLL
#define	 strcoll  strcmp
#endif

static  char   directory[256];
static  char   filename[256];
struct  entry
{
   char     *name;
   struct stat st;
}   *dir=NULL;

int   isslash(char c)
{
#ifdef MSDOS
   return(c=='/' || c=='\\');
#else
   return(c=='/');
#endif
}

void  NormalizeFileName(char *s)
{
   for( ; *s; s++)
   {
     if(isslash(*s))
     {
       char  *p=s;
       while(isslash(*(++p)));
       if(!*p)
       {
         *s=0;
         break;
       }
       strcpy(s+1,p);
     }
   }
}

const char *le_dirname(const char *f)
{
   static
   char  dir[256];
   char  *s;

   strcpy(dir,f);
   NormalizeFileName(dir);
   if(!*dir)
     return("/");
   for(s=dir+strlen(dir); s>dir && !isslash(*(s-1)); s--);
   if(s==dir)
     return(".");
   *(s-1)=0;
   return(dir);
}
const char *le_basename(const char *f)
{
   static
   char  bn[256];
   char  *s;

   strcpy(bn,f);
   NormalizeFileName(bn);
   if(!*bn)
     return("/");
   for(s=bn+strlen(bn); s>bn && !isslash(*(s-1)); s--);
   return(s);
}

int    entry_compare(struct entry *a,struct entry *b)
{
   int diff;
   diff=((a->st.st_mode&S_IFMT)==S_IFDIR)-((b->st.st_mode&S_IFMT)==S_IFDIR);
   if(diff)
      return(diff);
   diff=(strcmp(a->name,"..")==0)-(strcmp(b->name,"..")==0);
   if(diff)
      return(diff);
   diff=(a->name[0]=='.')-(b->name[0]=='.');
   if(diff)
      return(diff);
   diff=strcoll(a->name,b->name);
   return(diff);
}

void  condense(char *filename)
{
   int    filenamelen;
   char  *scan;
   ino_t   *ino_scanned;
   dev_t   *dev_scanned;
   dev_t   root_dev,curr_dev;
   ino_t   root_ino,curr_ino;
   char  **point;
   int    currpointno;
   struct  stat   st;
   char  *newfilename;
   char  *store;
   int i;
   char  drive[3]="";
   char  tmp[256];

#ifdef MSDOS
   if(isalpha(filename[0]) && filename[1]==':')
      sprintf(drive,"%.2s",filename);
#endif

   filenamelen=strlen(filename);
   newfilename=(char*)malloc(filenamelen+1);
   if(newfilename==NULL)
   {
      return;
   }
   ino_scanned=(ino_t*)malloc((filenamelen/2+1)*sizeof(*ino_scanned));
   if(ino_scanned==NULL)
   {
      free(newfilename);
      return;
   }
   dev_scanned=(dev_t*)malloc((filenamelen/2+1)*sizeof(*dev_scanned));
   if(dev_scanned==NULL)
   {
      free(newfilename);
      free(ino_scanned);
      return;
   }
   point=(char**)malloc((filenamelen/2+1)*sizeof(*point));
   if(point==NULL)
   {
      free(newfilename);
      free(ino_scanned);
      free(dev_scanned);
      return;
   }

   sprintf(tmp,"%s/",drive);
   if(stat(tmp,&st)!=-1)
   {
      root_dev=st.st_dev;
      root_ino=st.st_ino;
   }
   else
   {
      root_dev=
      root_ino=0;
   }
   sprintf(tmp,"%s.",drive);
   if(stat(tmp,&st)!=-1)
   {
      curr_dev=st.st_dev;
      curr_ino=st.st_ino;
   }
   else
   {
      curr_dev=
      curr_ino=0;
   }

   scan=filename+strlen(drive);
   store=newfilename;
   currpointno=0;
   while(*scan)
   {
      if(isslash(*scan))
      {
         while(isslash(*scan))
            scan++;
         /* append a slash if it meens root directory
          or if it is not the end if filename and it is not added yet */
         if(store==newfilename || (*scan && store[-1]!='/'))
            *(store++)='/';
      }
      else
      {
         while(*scan && !isslash(*scan))
            *(store++)=*(scan++);
         *store=0;
         sprintf(tmp,"%s%s",drive,newfilename);
         if(stat(tmp,&st)!=-1)
         {
            for(i=0; i<currpointno; i++)
            {
               if(ino_scanned[i]==st.st_ino && dev_scanned[i]==st.st_dev)
               {
                  store=point[currpointno=i];
                  break;
               }
            }
            if(st.st_ino==root_ino && st.st_dev==root_dev)
            {
               strcpy(newfilename,"/");
               store=newfilename+strlen(newfilename);
               currpointno=0;
            }
            else if(st.st_ino==curr_ino && st.st_dev==curr_dev)
            {
               strcpy(newfilename,".");
               store=newfilename+strlen(newfilename);
               currpointno=0;
            }
            else
            {
               ino_scanned[currpointno]=st.st_ino;
               dev_scanned[currpointno]=st.st_dev;
               point[currpointno++]=store;
            }
         }
      }
   }
   *store=0;
   sprintf(filename,"%s%s",drive,newfilename);
   free(newfilename);
   free(ino_scanned);
   free(dev_scanned);
   free(point);
}

int ChooseFileName(char *fn)
{
   char	    *a;
   static WIN *w=NULL;
   int      action;
   int      shift,current;
   struct stat st;
   int      x,y;
   char     str[300];
   char     str1[300];
   int      i,l;
   int      dirsize;
   DIR      *dd;
   struct dirent *entry;
   char     drive[3]="";

#ifdef MSDOS
   if(isalpha(fn[0]) && fn[1]==':')
      sprintf(drive,"%.2s",fn);
#endif

   /* strip trailing slashes - they are not needed */
   for(a=fn+strlen(fn)-1; *a=='/' && a>fn; a--);
   a[1]=0;

   a=strrchr(fn,'/');
   if(a==NULL)
   {
      sprintf(directory,"%s.",drive);
      strcpy(filename,fn+strlen(drive));
   }
   else
   {
      if(a!=fn+strlen(drive))
      {
         sprintf(directory,"%.*s",(int)(a-fn),fn);
      }
      else
         sprintf(directory,"%s/",drive);
      strcpy(filename,a+1);
   }

   if(directory[0]=='~' && (directory[1]==0 || directory[1]=='/'))
   {
      if(strlen(directory)+strlen(HOME)>254)
      {
         ErrMsg("Path name is too long");
         return(-1);
      }
      sprintf(str,"%s%s",HOME,directory+1);
      strcpy(directory,str);
   }

   /* check if there is a wildcard(s) */
   for(a=filename; *a; a++)
   {
      if(*a=='\\' && a[1])
         a++;
      else if(*a=='*' || *a=='?')
         break;
   }
   if(*a==0) /* there is no wildcards */
   {
      LoadHistory-=fn;
      condense(directory);
      for(a=filename; *a; a++)
         if(*a=='\\' && a[1])
            strcpy(a,a+1);  /* delete backslashes */
      if(directory[strlen(directory)-1]=='/')
         sprintf(fn,"%s%s",directory,filename);
      else
         sprintf(fn,"%s/%s",directory,filename);
      if(!strncmp(fn+strlen(drive),"./",2))
         memmove(fn+strlen(drive),fn+strlen(drive)+2,strlen(fn+strlen(drive)+2)+1);
      LoadHistory+=fn;
      return(0);
   }

   w=CreateWin(MIDDLE,MIDDLE,(DIRSIZ+2)*4+4,12,DIALOGUE_WIN_ATTR," Directory ",0);
   DisplayWin(w);

   do
   {
      condense(directory);
      if(stat(directory,&st)==-1)
      {
         FError(directory);
         CloseWin();
         DestroyWin(w);
	 return(-1);
      }
      if((st.st_mode&S_IFMT)!=S_IFDIR)
      {
         char  msg[128];
         sprintf(msg,"%.60s: not a directory",directory);
         ErrMsg(msg);
         CloseWin();
         DestroyWin(w);
	 return(-1);
      }

      if(dir)
         free(dir);

      PutStr(MIDDLE,FDOWN-1,"READING");
      refresh();

      dd=opendir(directory);
      if(!dd)
      {
         FError(directory);
         CloseWin();
         DestroyWin(w);
	 return(-1);
      }
      dirsize=0;
      while(readdir(dd))
         dirsize++;
      dir=(struct entry*)malloc(dirsize*sizeof(*dir));
      if(dir==NULL)
      {
         NotMemory();
         CloseWin();
         DestroyWin(w);
	 closedir(dd);
         return(-1);
      }
      rewinddir(dd);
      for(i=0; i<dirsize; i++)
      {
         entry=readdir(dd);
         if(entry==NULL)
         {
            dirsize=i;
            break;
         }
         sprintf(str,"%s/%s",directory,entry->d_name);
         if(stat(str,&(dir[i].st))==-1
         || ((dir[i].st.st_mode&S_IFMT)==S_IFREG && fnmatch(filename,entry->d_name,0)!=0)
         || ((dir[i].st.st_mode&S_IFMT)!=S_IFREG && (dir[i].st.st_mode&S_IFMT)!=S_IFDIR)
         || !strcmp(entry->d_name,"."))
         {
            i--;
            continue;
         }
         dir[i].name=strdup(entry->d_name);
         if(dir[i].name==NULL)
         {
            for(i=i-1; i>=0; i--)
               free(dir[i].name);
            free(dir);
            NotMemory();
            CloseWin();
            DestroyWin(w);
	    closedir(dd);
            return(-1);
         }
      }
      closedir(dd);

      qsort(dir,dirsize,sizeof(*dir),(int (*)(const void*,const void*))entry_compare);

      shift=current=0;
      do
      {
         Clear();
         if(current<shift)
            shift=current&~3;
         else if(current>=shift+(Upper->h-2)*4)
            shift=(current-(Upper->h-3)*4)&~3;
         for(i=shift,y=1; y<Upper->h-1 && i<dirsize; y++)
         {
            for(x=1; x<Upper->w-DIRSIZ-2-1 && i<dirsize; x+=DIRSIZ+2,i++)
            {
               l=strlen(dir[i].name);
               if(l>DIRSIZ)
                  sprintf(str," %.*s..%-.*s",DIRSIZ/2-1,dir[i].name,
                   DIRSIZ-(DIRSIZ/2)-1,dir[i].name+l-(DIRSIZ-(DIRSIZ/2)-1));
               else
                  sprintf(str," %.*s",DIRSIZ,dir[i].name);
               if((dir[i].st.st_mode&S_IFMT)==S_IFDIR)
                  strcat(str,"/");
               sprintf(str1,"%-*.*s",DIRSIZ+2,DIRSIZ+2,str);
               SetAttr(i==current?CURR_BUTTON_ATTR:DIALOGUE_WIN_ATTR);
               PutStr(x+1,y,str1);
            }
         }
         SetAttr(DIALOGUE_WIN_ATTR);
         for(x=0; x<12; x++)
            PutACS(FRIGHT-14+x,FDOWN,HLINE);
         if(shift>0 && (shift+(Upper->h-2)*4)<dirsize)
            PutStr(FRIGHT-3,FDOWN," PgUp/PgDn ");
         else if(shift>0)
            PutStr(FRIGHT-6,FDOWN," PgUp ");
         else if((shift+(Upper->h-2)*4)<dirsize)
            PutStr(FRIGHT-6,FDOWN," PgDn ");

	 if(directory[strlen(directory)-1]=='/')
	    sprintf(str,"%s%s - %s",directory,filename,dir[current].name);
	 else
	    sprintf(str,"%s/%s - %s",directory,filename,dir[current].name);
	 Message(str);

         action=GetNextAction();
         switch(action)
         {
         case(CHAR_LEFT):
            if(current>0)
               current--;
            break;
         case(CHAR_RIGHT):
            if(current<dirsize-1)
               current++;
            break;
         case(LINE_DOWN):
            if(current>dirsize-4-1)
               current=dirsize-1;
            else
               current+=4;
            break;
         case(LINE_UP):
            if(current<4)
               current=0;
            else
               current-=4;
            break;
         case(PREV_PAGE):
            if(current>4*(Upper->h-3))
               current-=4*(Upper->h-3);
            else
               current=0;
            break;
         case(NEXT_PAGE):
            if(current>=dirsize-4*(Upper->h-3)-1)
               current=dirsize-1;
            else
               current+=4*(Upper->h-3);
            break;
         case(LINE_BEGIN):
            current=shift=0;
            break;
         case(LINE_END):
            current=dirsize-1;
            break;
         }
      }
      while(action!=CANCEL && action!=NEWLINE);

      if((dir[current].st.st_mode&S_IFMT)==S_IFDIR && action==NEWLINE)
      {
         if(strlen(directory)>256-strlen(dir[current].name)-1)
         {
            ErrMsg("Path is too long");
            continue;
         }
         strcat(directory,"/");
         strcat(directory,dir[current].name);
      }
      else
         break;
   }
   while(1);

   LoadHistory-=HistoryLine(fn); /* delete the pattern from the history */
   if(directory[strlen(directory)-1]=='/')
      sprintf(fn,"%s%s",directory,dir[current].name);
   else
      sprintf(fn,"%s/%s",directory,dir[current].name);
   if(!strncmp(fn+strlen(drive),"./",2))
      strcpy(fn+strlen(drive),fn+strlen(drive)+2);
   CloseWin();
   DestroyWin(w);
   for(i=dirsize; i>0; )
      free(dir[--i].name);
   free(dir);
   dir=NULL;
   if(action==NEWLINE)
   {
      LoadHistory+=HistoryLine(fn);
      return(0);
   }
   return(-2);
}
