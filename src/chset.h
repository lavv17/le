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

#define	 CHSET_BITS_PER_BYTE  4
#define	 CHSET_SIZE	((256+CHSET_BITS_PER_BYTE-1)/CHSET_BITS_PER_BYTE)

extern	 byte  chset[CHSET_SIZE+1];

void  init_chset();
void  set_chset_8bit();
void  set_chset_8bit_noctrl();

void  addch_visual(chtype ch);
chtype visualize(struct attr *a,chtype ch);
