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

const char *le_basename(const char *);
const char *le_dirname(const char *);
static inline char *le_basename(char *f) { return const_cast<char*>(le_basename(const_cast<const char*>(f))); }
static inline char *le_dirname(char *f) { return const_cast<char*>(le_dirname(const_cast<const char*>(f))); }

int ChooseFileName(char *pattern, unsigned nbytes);
