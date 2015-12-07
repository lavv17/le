/*
 * Copyright (c) 1993-2015 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <stdio.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include "edit.h"
#include "getch.h"

#ifndef MSDOS
char    Shell  [256]="exec $SHELL";
char    Make   [256]="exec make";
char    Run    [256]="exec make run";
char    Compile[256]="exec make \"$FNAME.o\"";
char    HelpCmd[256]="exec "PKGDATADIR"/help";
#else
char    Shell  [256]="command";
char    Make   [256]="make";
char    Run    [256]="make run";
char    Compile[256]="make \"$FNAME.o\"";
char    HelpCmd[256]="man \"$WORD\"";
#endif

/* cmd - execute command c */
void    cmd(const char *c,bool autosave,bool pauseafter)
{
    char        cl[256];
    char        file[256],name[256],ext[256];
    char        *s,*f,*n,*e,*p;
    extern struct menu ConCan4Menu[];
    int         exitcode;
    int         oldalarm=alarm(0);

    errno=0;
    if(modified && autosave)
    {
        SaveFile(FileName);
        if(errno)
        {
            switch(ReadMenuBox(ConCan4Menu,HORIZ,"Cannot save the file"," Warning ",
	       VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
            {
            case('C'):
            case(0):
                return;
            }
        }
    }
    /* check if the file has lock enforce flag */
    if(autosave && LockEnforce(FileMode))
    {
        /* turn off lock feature of the file */
        if(chmod(FileName,LockEnforceStrip(FileMode))==(-1))
        {
            FError(FileName);
            switch(ReadMenuBox(ConCan4Menu,HORIZ,
	       "Cannot change the file mode.\nThe file will not be readable.",
	       " Warning ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
            {
            case('C'):
            case(0):
                return;
            }
        }
    }
    for(s=FileName,f=file,n=name,e=ext,p=NULL; *s; s++)
    {
#ifndef MSDOS
        if(*s=='$' || *s=='`' || *s=='\\' || *s=='"')
            *e++ = *n++ = *f++ = '\\';
#endif
        if(*s=='.')
        {
            p=n;
            e=ext;
        }
        else
        {
            if(*s=='/')
                p=NULL,e=ext;
        }
        *e++ = *n++ = *f++ = *s;
    }
    *e = *f = '\0';
    if(p)
        *p='\0';    /* there was extension */
    else
        *n='\0',*ext='\0';  /* there was no extension */
#ifndef __MSDOS__
    sprintf(cl,"FILE=\"%s\";FNAME=\"%s\";EXT=\"%s\";WORD=\"%s\";export WORD FILE EXT FNAME; %s",
                file,name,ext,GetWord(),c);
#else
    {
      FILE *bat;
      bat=fopen("lecmd.bat","wt");
      if(bat==NULL)
      {
         FError("lecmd.bat");
         return;
      }
      fprintf(bat,"@echo off\nset FILE=%s\nset FNAME=%s\nset EXT=%s\nset WORD=%s\n%s",
                file,name,ext,GetWord(),c);
      fclose(bat);
      strcpy(cl,"lecmd.bat");
    }
#endif
    TermCurses();
    ReleaseSignalHandlers();
    fflush(stdout);

#ifdef __MSDOS__
   char oldwd[256];
   if(getcwd(oldwd,sizeof(oldwd))==NULL)
   {
      FError("getcwd()");
      remove("lecmd.bat");
      return;
   }
#endif
    errno=0;
    exitcode=system(cl);
    if(exitcode==-1)
    {
        perror("system()");
        putchar('\r');
    }
#ifdef __MSDOS__
   if(chdir(oldpw)==-1)
      FError(oldpw);
    remove("lecmd.bat");
#endif
#ifndef __MSDOS__
    reset_prog_mode();
#endif
    flushinp();
    if(pauseafter || exitcode!=0)
    {
        if(LockEnforce(FileMode))
            chmod(FileName,FileMode);   /* ??? */
        printf("[Press any key to continue]");
        fflush(stdout);
#ifdef __MSDOS__
	if(!GetRawKey())
#endif
	(void)GetRawKey();
        printf("\r\n");
        fflush(stdout);
    }
    InstallSignalHandlers();
#ifdef __MSDOS__
    InitCurses(); // In PDCurses, endwin fatally terminates screen I/O,
		  // so we need to reinitialize.
#endif
    alarm(oldalarm);
    refresh();
    flag=REDISPLAY_ALL;
}

void    DoMake()
{
    if(View)
        return;
    cmd(Make,1,1);
}
void    DoShell()
{
    cmd(Shell,0,0);
}
void    DoRun()
{
    if(View)
        return;
    cmd(Run,1,1);
}
void    DoCompile()
{
    if(View)
        return;
    cmd(Compile,1,1);
}
