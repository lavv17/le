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

/* Definitions for pull-down menu */

#ifndef MENU1_H
#define MENU1_H
typedef struct	sr_menu
{
	char	*text;	/* the text of the item, NULL means end of (sub)menu */
	byte	fl;
	void	(*func)(void);	/* the function to call		*/
	int		*ctrl1;
	int		*ctrl2;
} Menu1;

#define SUBM    1
#define FUNC    2
#define HIDE    4

#endif /* MENU1_H */
