/*
 * Lua C routines that link the script with the GUI.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "config.h"
#include "gtksudoku.h"
#include "interp.h"
#include "sudoku.h"

static char *
clone(const char *src)		/* Malloc a copy of a string. */
{
  if (!src)
    return NULL;
  size_t n = strlen(src) + 1;
  char *dest = malloc(n);
  if (!dest) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  return memcpy(dest, src, n);
}

static int
set_val(lua_State *L)
{
  int row = luaL_checkint(L, 1);
  int col = luaL_checkint(L, 2);
  int val = luaL_checkint(L, 3);
  int mode = lua_toboolean(L, 4);
  interp_set_val(row, col, val, mode);
  return 0;
}

static int
edit(lua_State *L)
{
  const char *board = lua_tostring(L, 1);
  char *result = interp_edit(board);
  if (result) {
    lua_pushstring(L, result);
    free(result);
  }
  else
    lua_pushnil(L);
  return 1;
}

static int
show(lua_State *L)
{
  const char *text = lua_tostring(L, 1);
  if (text)
    interp_show(clone(text));
  return 0;
}

static lua_State *L;

static void
push_item(lua_State *L, const char *cmd, const char *tail)
{
  const char *s;
  for (s = cmd; s < tail; s++)
    if (!isdigit(*s)) {
      lua_pushlstring(L, cmd, tail - cmd);
      return;
    }
  lua_pushinteger(L, atoi(cmd));
}

char *
interp_eval(const char *cmd)
{
  while (*cmd == ' ') 		/* Get rid of leading spaces */
    cmd++;
  if (!*cmd)			/* If nothing left, silently exit */
    return NULL;
  lua_getglobal(L, "eval");
  int nargs = 0;
  for (;;) {
    nargs++;
    const char *tail = strchr(cmd, ' '); /* Find separator */
    if (!tail) {
      push_item(L, cmd, cmd + strlen(cmd));
      break;
    }
    else{
      push_item(L, cmd, tail);
      cmd = tail + 1;
      while (*cmd == ' ')
	cmd++;
      if (!*cmd)		/* Last token found */
	break;
    }
  }
  lua_pcall(L, nargs, 1, 0);
  return clone(lua_tostring(L, -1));
}

char *
interp_load(const char *board)
{
  lua_getglobal(L, "load");
  lua_pushstring(L, board);
  if (lua_pcall(L, 1, 0, 0))
    return clone(lua_tostring(L, -1));
  else
    return NULL;
}

char *
interp_save(char **board)
{
  lua_getglobal(L, "save");
  if (lua_pcall(L, 0, 1, 0)) {
    *board = NULL;
    return clone(lua_tostring(L, -1));
  }
  else {
    *board = clone(lua_tostring(L, -1));
    return NULL;
  }
}

char *
interp_init(void)
{
  L = luaL_newstate();
  if (!L)
    return clone("Failed to create a Lua interpreter");
  luaL_openlibs(L);		/* Load libraries */
  lua_pushcfunction(L, set_val);
  lua_setglobal(L, "set_val");
  lua_pushcfunction(L, edit);
  lua_setglobal(L, "edit");
  lua_pushcfunction(L, show);
  lua_setglobal(L, "show");
  /* Load application written in Lua */
  if (luaL_loadbuffer(L, (const char*)sudoku_lua_bytes,
		      sizeof(sudoku_lua_bytes), sudoku_lua_source)
      || lua_pcall(L, 0, 0, 0))
    return clone(lua_tostring(L, -1));
  else
    return NULL;
}
