/*
 * Copyright (c) 1993-2006 by Alexander V. Lukyanov (lav@yars.free.net)
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

#define	STSIZE		256

#define	OKAY		0
#define	STUNDERFLOW	(-1)
#define	STOVERFLOW	(-2)
#define	ILLEGALFN	(-3)
#define	INVALIDFN	(-4)
#define	INVALIDNUM	(-5)

struct calc_value
{
   long double value;
   unsigned char base;
   const char *to_string();
   operator long double() { return value; }
};

extern int sp;
extern calc_value stack[STSIZE];
extern int calcerrno;

const char *calcerrmsg();
int   calculator(const char *);
void  initcalc();
