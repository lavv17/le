#include <stdio.h>
#include "edit.h"
#include "colormnu.h"
#include "options.h"

void ColorsSaveToFile(const char *f)
{
   DescribeColors(bw_pal,color_pal);
   SaveConfToFile(f,colors);
}

static const char *const colors_file="/.le/colors";
void ColorsSave()
{
   char *f=(char*)alloca(strlen(HOME)+strlen(colors_file)+1);
   sprintf(f,"%s%s",HOME,colors_file);
   ColorsSaveToFile(f);
}

void ColorsSaveForTerminal()
{
   char *f=(char*)alloca(strlen(HOME)+strlen(colors_file)+1+strlen(TERM)+1);
   sprintf(f,"%s%s-%s",HOME,colors_file,TERM);
   ColorsSaveToFile(f);
}

void LoadColor(const char *f)
{
   ReadConfFromFile(f,colors);
   ParseColors();
   init_attrs();
   clearok(stdscr,1);
   flag=REDISPLAY_ALL;
}

void LoadColorDefault()
{
   memcpy(color_pal,default_color_pal,sizeof(default_color_pal));
   memcpy(bw_pal,default_bw_pal,sizeof(default_bw_pal));
   init_attrs();
   clearok(stdscr,1);
   flag=REDISPLAY_ALL;
}

void LoadColorDefaultBG()
{
   LoadColor(PKGDATADIR"/colors-defbg");
}
void LoadColorBlue()
{
   LoadColor(PKGDATADIR"/colors-blue");
}
void LoadColorBlack()
{
   LoadColor(PKGDATADIR"/colors-black");
}
void LoadColorWhite()
{
   LoadColor(PKGDATADIR"/colors-white");
}
