/*
 * Boards as character strings.
 *
 * Copyright (C) 2006 John D. Ramsdell
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stddef.h>
#include "config.h"
#include "gtksudoku.h"

int
isboardchar(int c)
{
  return c == '.' || (c >= '1' && c <= '9');
}

size_t
boardlen(const char *s)
{
  size_t n = 0;
  if (!s)
    return n;
  for (; *s; s++)
    if (isboardchar(*s))
      n++;
  return n;
}

int
boardchar2val(int c)
{
  if (c >= '1' && c <= '9')
    return 1 << (c - '1');
  else
    return ALL;
}

int
val2boardchar(int val)
{
  int d;
  for (d = 0; d < DIGITS; d++)
    if (val == (1 << d))
      return '1' + d;
  return '.';
}
