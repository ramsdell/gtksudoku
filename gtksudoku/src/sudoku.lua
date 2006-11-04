--[[

An implementation of the Sudoku game.  This file provides an
interpreter for commands that manipulate a Sudoku board.  The display
of the board is handled by C code.

Copyright (C) 2006 John D. Ramsdell

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

]]

-- This program uses a right-handed coordinate system.

-- Functions that implement rules return a boolean value which is true
-- if the rule eliminated some possible cell values.  These functions
-- may optionally return a second value, a message string.

-- Print mode.

local details = false

-- Useful constants

local sides = 3
local digits = sides * sides
local digits2 = digits * digits

-- Cells

local Cell = {}
Cell.__index = Cell

-- For digit i, cell[i] is false if i has been eliminated as a
-- possible value for the cell.

local function mk_cell()
   local obj = {}
   setmetatable(obj, Cell)
   for i=1,digits do
      obj[i] = true
   end
   return obj
end

function Cell:clone()
   local obj = {}
   setmetatable(obj, Cell)
   for i=1,#self do
      obj[i] = self[i]
   end
   if self.determined then
      obj.determined = true
   end
   return obj
end

function Cell:same(other)
   if not other then
      return false
   end
   if self.determined ~= other.determined then
      return false
   end
   for i=1,#self do
      if self[i] ~= other[i] then
         return false
      end
   end
   return true
end

-- Returns true when a some possible cell values have been eliminated.

function Cell:singleton(d)
   local e = false
   for i=1,#self do
      if i ~= d and self[i] then
	 self[i] = false
	 e = true
      end
   end
   return e
end

function Cell:unknowns()
   local n = 0
   for i=1,#self do
      if self[i] then
	 n = n + 1
      end
   end
   return n
end

function Cell:first()
   for i=1,#self do
      if self[i] then
	 return i
      end
   end
   error("Board inconsistent", 0)
end

function Cell:only_pair_present(d1, d2)
   if d1 == d2 then
      return false
   end
   for i=1,#self do
      if (i == d1 or i == d2) ~= self[i] then
	 return false
      end
   end
   return true
end

function Cell:val()
   local n = 0;
   for i=#self,1,-1 do
      n = 2 * n
      if self[i] then
	 n = n + 1
      end
   end
   return n
end

function Cell:show()
   if self.determined and self:unknowns() == 1 then
      return tostring(self:first())
   else
      return "."
   end
end

-- Boards

-- A board is a 3*3*3*3 array of cells.  In a board[i][j][k][l]
-- reference, i is the major row, j is the major column, k is in minor
-- row, and l is the minor column.

local Board = {}
Board.__index = Board

local function mk_board()
   local obj = {}
   setmetatable(obj, Board)
   for r1=1,sides do
      local major_row = {}
      obj[r1] = major_row
      for c1=1,sides do
	 local minor_column = {}
	 major_row[c1] = minor_column
	 for r2=1,sides do
	    local minor_row = {}
	    minor_column[r2] = minor_row
	    for c2=1,sides do
	       minor_row[c2] = mk_cell()
	    end
	 end
      end
   end
   return obj
end

function Board:clone()
   local obj = {}
   setmetatable(obj, Board)
   for r1=1,sides do
      local major_row = {}
      obj[r1] = major_row
      for c1=1,sides do
	 local minor_column = {}
	 major_row[c1] = minor_column
	 for r2=1,sides do
	    local minor_row = {}
	    minor_column[r2] = minor_row
	    for c2=1,sides do
	       minor_row[c2] = self[r1][c1][r2][c2]:clone()
	    end
	 end
      end
   end
   return obj
end

function Board:same(other)
   if not other then
      return false
   end
   for r1=1,sides do
      for c1=1,sides do
	 for r2=1,sides do
	    for c2=1,sides do
	       if not self[r1][c1][r2][c2]:same(other[r1][c1][r2][c2]) then
		  return false
	       end
	    end
	 end
      end
   end
   return true
end

function Board:__tostring()
   local s = ""
   for r1=1,sides do
      for r2=1,sides do
	 for c1=1,sides do
	    for c2=1,sides do
	       s = s .. self[r1][c1][r2][c2]:show()
	    end
	 end
      end
   end
   return s
end

function Board:show()
   local s = ""
   for r1=1,sides do
      for r2=1,sides do
	 for c1=1,sides do
	    for c2=1,sides do
	       s = s .. self[r1][c1][r2][c2]:show()
	    end
	 end
	 s = s .. "\n"
      end
   end
   return s
end

-- Board printing

local function zero_based_index(major, minor)
   return sides * (major - 1) + minor - 1
end

function Board:print_item(r1, c1, r2, c2)
   local cell = self[r1][c1][r2][c2]
   local val = cell:val()
   if val ~= 0 and not details and not cell.determined then
      val = -1
   end
   set_val(zero_based_index(r1, r2), zero_based_index(c1, c2),
	   val, not cell.determined)
end

function Board:print_all()
   for r1=1,sides do
      for c1=1,sides do
	 for r2=1,sides do
	    for c2=1,sides do
	       self:print_item(r1, c1, r2, c2)
	    end
	 end
      end
   end
end

local function print_blank_board()
   for r1=1,sides do
      for c1=1,sides do
	 for r2=1,sides do
	    for c2=1,sides do
	       set_val(zero_based_index(r1, r2), zero_based_index(c1, c2), -1)
	    end
	 end
      end
   end
end

-- Determine the value of a cell, and propagate the influence of that
-- determination in there is but one unknown in a square, row, or
-- column.

function Board:determine(r1, c1, r2, c2, d)
   local cell = self[r1][c1][r2][c2]
   if not cell[d] or cell.determined then
      return false
   end
   cell.determined = true
   local e = cell:singleton(d)
   local m = 0

   -- Propagate singleton's influence in a square.
   for rr2=1,sides do
      for cc2=1,sides do
	 if rr2 ~= r2 or cc2 ~= c2 then
	    if self[r1][c1][rr2][cc2][d] then
	       self[r1][c1][rr2][cc2][d] = false
	       e = true
	    end
	 end
	 if self[r1][c1][rr2][cc2].determined then
	    m = m + 1
	 end
      end
   end

   -- Finish off square with just one unknown.
   if m == digits - 1 then
      for rr2=1,sides do
	 for cc2=1,sides do
	    if not self[r1][c1][rr2][cc2].determined then
	       e = self:propagate_elimination(r1, c1, rr2, cc2) or e
	    end
	 end
      end
   end

   -- Propagate singleton's influence in a row.
   m = 0
   for cc1=1,sides do
      for cc2=1,sides do
	 if cc1 ~= c1 or cc2 ~= c2 then
	    if self[r1][cc1][r2][cc2][d] then
	       self[r1][cc1][r2][cc2][d] = false
	       e = true
	    end
	 end
	 if self[r1][cc1][r2][cc2].determined then
	    m = m + 1
	 end
      end
   end

   -- Finish off row with just one unknown.
   if m == digits - 1 then
      for cc1=1,sides do
	 for cc2=1,sides do
	    if not self[r1][cc1][r2][cc2].determined then
	       e = self:propagate_elimination(r1, cc1, r2, cc2) or e
	    end
	 end
      end
   end

   -- Propagate singleton's influence in a column.
   m = 0
   for rr1=1,sides do
      for rr2=1,sides do
	 if rr1 ~= r1 or rr2 ~= r2 then
	    if self[rr1][c1][rr2][c2][d] then
	       self[rr1][c1][rr2][c2][d] = false
	       e = true
	    end
	 end
	 if self[rr1][c1][rr2][c2].determined then
	    m = m + 1
	 end
      end
   end

   -- Finish off column with just one unknown.
   if m == digits - 1 then
      for rr1=1,sides do
	 for rr2=1,sides do
	    if not self[rr1][c1][rr2][c2].determined then
	       e = self:propagate_elimination(rr1, c1, rr2, c2) or e
	    end
	 end
      end
   end

   return e
end

-- Propagate the influence of eliminating a possible value by seeing
-- if the elimination determines the value in a cell.

function Board:propagate_elimination(r1, c1, r2, c2)
   local cell = self[r1][c1][r2][c2]
   if cell:unknowns() > 1 or cell.determined then
      return false		-- Cell is not a singleton or is
   end				-- determined, so bail out now.
   return self:determine(r1, c1, r2, c2, cell:first())
end

-- Reading puzzles from strings

local function strip_string(s)
   return s:gsub("[^.1-9]", "")  -- Flush separators and other characters
end

local function mk_digit_translator()
   local pattern = "123456789"
   local obj = {}
   for i=1,pattern:len() do
      obj[pattern:byte(i)] = i
   end
   return obj
end

local digit_translator = mk_digit_translator()

local function board(s)
   local t = strip_string(s)
   if t:len() ~= digits2 then
      local msg = "bad input: expected " .. digits2 .. " items but found "
      error(msg .. t:len() .. " in \n" .. s, 0)
   end
   local b = mk_board()
   local i = 0
   for r1=1,sides do
      for r2=1,sides do
	 for c1=1,sides do
	    for c2=1,sides do
	       i = i + 1
	       local c = t:byte(i)
	       local d = digit_translator[c]
	       if d then
		  b[r1][c1][r2][c2]:singleton(d)
		  b:determine(r1, c1, r2, c2, d)
	       end
	    end
	 end
      end
   end
   return b
end

-- Convert a row or column coordinate into a cell index pair.

local function spread(index)
   if index < 1 or index > digits then
      error("Bad index number " .. index, 0)
   else
      local i1 = math.ceil(index / sides)
      local i2 = (index - 1) % sides + 1
      return i1, i2
   end
end

-- Strategies

-- Each return a boolean value which is true if the rule eliminated
-- some possible cell values.  These functions may optionally return a
-- second value, a message string.

function Board:propagate_all_singletons()
   local e = false
   repeat
      local f = false
      for r1=1,sides do
	 for r2=1,sides do
	    for c1=1,sides do
	       for c2=1,sides do
		  f = self:propagate_elimination(r1, c1, r2, c2) or f
	       end
	    end
	 end
      end
      if f then
	 e = true
      else
	 return e
      end
   until false
end

-- The location of digit d is determined if there is only one place it
-- occurs in the square at (row, col).  The coordinates specify some
-- cell within the targeted square.

function Board:one_place_in_square(d, row, col)
   local r1 = spread(row)
   local c1 = spread(col)
   local e = false
   local m = 0
   for r2=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d] then
	    m = m + 1
	 end
      end
   end
   if m == 1 then
      for r2=1,sides do
	 for c2=1,sides do
	    if self[r1][c1][r2][c2][d] then
	       e = self:determine(r1, c1, r2, c2, d) or e
	    end
	 end
      end
   end
   return e
end

function Board:one_place_in_all_squares()
   local e = false
   for d=1,digits do
      for r1=1,sides do
	 for c1=1,sides do
	    e = self:one_place_in_square(d, r1 * sides, c1 * sides) or e
	 end
      end
   end
   return e
end

-- The location of digit d is determined if there is only one place it
-- occurs in the given row.

function Board:one_place_in_row(d, row)
   local r1, r2 = spread(row)
   local e = false
   local m = 0
   for c1=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d] then
	    m = m + 1
	 end
      end
   end
   if m == 1 then
      for c1=1,sides do
	 for c2=1,sides do
	    if self[r1][c1][r2][c2][d] then
	       e = self:determine(r1, c1, r2, c2, d) or e
	    end
	 end
      end
   end
   return e
end

function Board:one_place_in_all_rows()
   local e = false
   for d=1,digits do
      for row=1,digits do
	 e = self:one_place_in_row(d, row) or e
      end
   end
   return e
end

-- The location of digit d is determined if there is only one place it
-- occurs in the given column.

function Board:one_place_in_column(d, col)
   local c1, c2 = spread(col)
   local e = false
   local m = 0
   for r1=1,sides do
      for r2=1,sides do
	 if self[r1][c1][r2][c2][d] then
	    m = m + 1
	 end
      end
   end
   if m == 1 then
      for r1=1,sides do
	 for r2=1,sides do
	    if self[r1][c1][r2][c2][d] then
	       e = self:determine(r1, c1, r2, c2, d) or e
	    end
	 end
      end
   end
   return e
end

function Board:one_place_in_all_columns()
   local e = false
   for d=1,digits do
      for col=1,digits do
	 e = self:one_place_in_column(d, col) or e
      end
   end
   return e
end

-- Apply all the one place rules until none apply.

function Board:simp()
   local e = false
   local f = false
   repeat
      f = self:propagate_all_singletons()
	 or self:one_place_in_all_squares()
	 or self:one_place_in_all_rows()
	 or self:one_place_in_all_columns()
      e = e or f
   until not f
   return e
end

-- If digit d is only in one row in a square, it cannot be in that
-- same row in other squares.

function Board:one_row_in_square(d, row, col)
   local e = false
   local r1, r2 = spread(row)
   local c1 = spread(col)
   for rr2=1,sides do
      if rr2 ~= r2 then
	 for c2=1,sides do
	    if self[r1][c1][rr2][c2][d] then
	       return e		-- Rule not applicable
	    end
	 end
      end
   end
   for cc1=1,sides do
      if cc1 ~= c1 then
	 for c2=1,sides do
	    if self[r1][cc1][r2][c2][d] then
	       self[r1][cc1][r2][c2][d] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

-- If digit d is only in one column in a square, it cannot be in that
-- same column in other squares.

function Board:one_column_in_square(d, row, col)
   local e = false
   local r1 = spread(row)
   local c1, c2 = spread(col)
   for cc2=1,sides do
      if cc2 ~= c2 then
	 for r2=1,sides do
	    if self[r1][c1][r2][cc2][d] then
	       return e		-- Rule not applicable
	    end
	 end
      end
   end
   for rr1=1,sides do
      if rr1 ~= r1 then
	 for r2=1,sides do
	    if self[rr1][c1][r2][c2][d] then
	       self[rr1][c1][r2][c2][d] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

-- If digit d is only in one square of a row, it cannot be in other
-- rows in that square.

function Board:one_square_for_row(d, row, col)
   local e = false
   local r1, r2 = spread(row)
   local c1 = spread(col)
   for cc1=1,sides do
      if cc1 ~= c1 then
	 for c2=1,sides do
	    if self[r1][cc1][r2][c2][d] then
	       return e		-- Rule not applicable
	    end
	 end
      end
   end
   for rr2=1,sides do
      if rr2 ~= r2 then
	 for c2=1,sides do
	    if self[r1][c1][rr2][c2][d] then
	       self[r1][c1][rr2][c2][d] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

-- If digit d is only in one square of a column, it cannot be in other
-- columns in that square.

function Board:one_square_for_column(d, row, col)
   local e = false
   local r1 = spread(row)
   local c1, c2 = spread(col)
   for rr1=1,sides do
      if rr1 ~= r1 then
	 for r2=1,sides do
	    if self[rr1][c1][r2][c2][d] then
	       return e		-- Rule not applicable
	    end
	 end
      end
   end
   for cc2=1,sides do
      if cc2 ~= c2 then
	 for r2=1,sides do
	    if self[r1][c1][r2][cc2][d] then
	       self[r1][c1][r2][cc2][d] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

-- If there are only two places for d1 and d2 in a square, only d1 and
-- d2 can appear in those places.

function Board:two_places_for_pair_in_square(d1, d2, row, col)
   local e = false
   local r1 = spread(row)
   local c1 = spread(col)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for r2=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for r2=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    for d=1,digits do
	       if d ~= d1 and d ~= d2 and self[r1][c1][r2][c2][d] then
		  self[r1][c1][r2][c2][d] = false
		  e = true
	       end
	    end
	 end
      end
   end
   return e
end

-- If there are only two places for d1 and d2 in a row, only d1 and
-- d2 can appear in those places.

function Board:two_places_for_pair_in_row(d1, d2, row)
   local e = false
   local r1, r2 = spread(row)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for c1=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for c1=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    for d=1,digits do
	       if d ~= d1 and d ~= d2 and self[r1][c1][r2][c2][d] then
		  self[r1][c1][r2][c2][d] = false
		  e = true
	       end
	    end
	 end
      end
   end
   return e
end

-- If there are only two places for d1 and d2 in a column, only d1 and
-- d2 can appear in those places.

function Board:two_places_for_pair_in_column(d1, d2, col)
   local e = false
   local c1, c2 = spread(col)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for r1=1,sides do
      for r2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for r1=1,sides do
      for r2=1,sides do
	 if self[r1][c1][r2][c2][d1] or self[r1][c1][r2][c2][d2] then
	    for d=1,digits do
	       if d ~= d1 and d ~= d2 and self[r1][c1][r2][c2][d] then
		  self[r1][c1][r2][c2][d] = false
		  e = true
	       end
	    end
	 end
      end
   end
   return e
end

function Board:same_pair_in_square(d1, d2, row, col)
   local e = false
   local r1 = spread(row)
   local c1 = spread(col)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for r2=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for r2=1,sides do
      for c2=1,sides do
	 if not self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    if self[r1][c1][r2][c2][d1] then
	       self[r1][c1][r2][c2][d1] = false
	       e = true
	    end
	    if self[r1][c1][r2][c2][d2] then
	       self[r1][c1][r2][c2][d2] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

function Board:same_pair_in_row(d1, d2, row)
   local e = false
   local r1, r2 = spread(row)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for c1=1,sides do
      for c2=1,sides do
	 if self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for c1=1,sides do
      for c2=1,sides do
	 if not self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    if self[r1][c1][r2][c2][d1] then
	       self[r1][c1][r2][c2][d1] = false
	       e = true
	    end
	    if self[r1][c1][r2][c2][d2] then
	       self[r1][c1][r2][c2][d2] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

function Board:same_pair_in_column(d1, d2, col)
   local e = false
   local c1, c2 = spread(col)
   if d1 == d2 then
      return e			-- Bad input
   end
   local m = 0
   for r1=1,sides do
      for r2=1,sides do
	 if self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    m = m + 1
	 end
      end
   end
   if m ~= 2 then
      return e			-- Rule not applicable
   end
   for r1=1,sides do
      for r2=1,sides do
	 if not self[r1][c1][r2][c2]:only_pair_present(d1, d2) then
	    if self[r1][c1][r2][c2][d1] then
	       self[r1][c1][r2][c2][d1] = false
	       e = true
	    end
	    if self[r1][c1][r2][c2][d2] then
	       self[r1][c1][r2][c2][d2] = false
	       e = true
	    end
	 end
      end
   end
   return e
end

-- Try all rules.
function Board:all()
   local e = self:simp()
   if e then
      return e, "simp"
   end

   for d=1,digits do
      for row=1,digits do
	 for col=1,digits do
	    e = self:one_row_in_square(d, row, col)
	    if e then
	       return e, "one row in square at ("
		  .. row .. ", " .. col .. ")"
	    end
	    e = self:one_column_in_square(d, row, col)
	    if e then
	       return e, "one column in square at ("
		  .. row .. ", " .. col .. ")"
	    end
	    e = self:one_square_for_row(d, row, col)
	    if e then
	       return e, "one square for row at ("
		  .. row .. ", " .. col .. ")"
	    end
	    e = self:one_square_for_column(d, row, col)
	    if e then
	       return e, "one square for column at ("
		  .. row .. ", " .. col .. ")"
	    end
	 end
      end
   end

   for d1=1,digits do
      for d2=1,digits do
	 if d1 ~= d2 then
	    for col=1,digits do
	       e = self:two_places_for_pair_in_column(d1, d2, col)
	       if e then
		  return e, "two places for pair in column at " .. col
	       end
	       e = self:same_pair_in_column(d1, d2, col)
	       if e then
		  return e, "same pair in column at " .. col
	       end
	    end
	    for row=1,digits do
	       e = self:two_places_for_pair_in_row(d1, d2, row)
	       if e then
		  return e, "two places for pair in row at " .. row
	       end
	       e = self:same_pair_in_row(d1, d2, row)
	       if e then
		  return e, "same pair in row at " .. row
	       end
	       for col=1,digits do
		  e = self:two_places_for_pair_in_square(d1, d2, row, col)
		  if e then
		     return e, "two places for pair in square at ("
			.. row .. ", " .. col .. ")"
		  end
		  e = self:same_pair_in_square(d1, d2, row, col)
		  if e then
		     return e, "same pair in square at ("
			.. row .. ", " .. col .. ")"
		  end
	       end
	    end
	 end
      end
   end
   return e
end

function Board:hint()
   for r1=1,sides do		-- Look for one place in square hint
      for c1=1,sides do
	 for d=1,digits do
	    local m = 0
	    for r2=1,sides do
	       for c2=1,sides do
		  local cell = self[r1][c1][r2][c2]
		  if not cell.determined and cell[d] then
		     m = m + 1
		  end
	       end
	    end
	    if m == 1 then
	       for r2=1,sides do
		  for c2=1,sides do
		     local cell = self[r1][c1][r2][c2]
		     if not cell.determined and cell[d] then
			local row = (r1 - 1) * sides + r2
			local col = (c1 - 1) * sides + c2
			local msg = "look at " .. d .. " in ("
			return false, msg .. row .. ", " .. col .. ")"
		     end
		  end
	       end
	    end
	 end
      end
   end

   for r1=1,sides do		-- Look for one place in row hint
      for r2=1,sides do
	 for d=1,digits do
	    local m = 0
	    for c1=1,sides do
	       for c2=1,sides do
		  local cell = self[r1][c1][r2][c2]
		  if not cell.determined and cell[d] then
		     m = m + 1
		  end
	       end
	    end
	    if m == 1 then
	       local row = (r1 - 1) * sides + r2
	       return false, "look at " .. d .. " in row " .. row
	    end
	 end
      end
   end

   for c1=1,sides do		-- Look for one place in column hint
      for c2=1,sides do
	 for d=1,digits do
	    local m = 0
	    for r1=1,sides do
	       for r2=1,sides do
		  local cell = self[r1][c1][r2][c2]
		  if not cell.determined and cell[d] then
		     m = m + 1
		  end
	       end
	    end
	    if m == 1 then
	       local col = (c1 - 1) * sides + c2
	       return false, "look at " .. d .. " in column " .. col
	    end
	 end
      end
   end

   for r1=1,sides do	-- Look for undetermined cell with one unknown
      for c1=1,sides do
	 for r2=1,sides do
	    for c2=1,sides do
	       local cell = self[r1][c1][r2][c2]
	       if not cell.determined and cell:unknowns() == 1 then
		  local row = (r1 - 1) * sides + r2
		  local col = (c1 - 1) * sides + c2
		  local msg = "look at " .. cell:first() .. " in ("
		  return false, msg .. row .. ", " .. col .. ")"
	       end
	    end
	 end
      end
   end

   return false, "No hint available"
end

-- Board histories

local it			-- The current board
local history = {}		-- The history of boards

-- Push a board on the history only if it is not the same
-- as the one most recently pushed.
local function push()
   if it and not it:same(history[#history]) then
      table.insert(history, it:clone())
      return true
   end
end

-- pop a board from the history.  If that board is the same
-- the current one, pop another.
local function back()
   local top = table.remove(history)
   if not top then
      return
   end
   if not top:same(it) then
      it = top
      return true
   end
   top = table.remove(history)
   if top then
      it = top
      return true
   end
end

-- Swap the top of the stack with the current focus of attention.
local function swap()
   local penultimate = it
   back()
   local ultimate = it
   it = penultimate
   push()
   it = ultimate
   return true
end

-- Make a new board.
local function new()
   push()
   it = mk_board()
   return true
end

-- Load a puzzle from a string.

function load(s)
   it = board(s)
   history = {}
   return it:print_all()
end

function save()
   if it then
      return it:show()
   else
      error("nothing to save", 0)
   end
end

-- Board editing

local function do_edit()
   local s
   if it then
      s = tostring(it)
   end
   s = edit(s)
   if s then
      push()
      it = board(s)
      return it:print_all()
   else
      return "edit canceled"
   end
end

-- Help messages

local topics = {}

-- Converts a text string into something ready for word wrapping.

local function wrap(s)
   s = s:gsub("\r\n", "\n")	-- Handle Windows end of line.
   s = s:gsub("% +\n", "\n")	-- Remove trailing spaces.
   s = s:gsub("%.\n[^\n]",	-- Handle end of sentence
	      function (s)	-- within a paragraph.
		 return s:gsub("\n", "  ")
	      end)
   s = s:gsub("[^\n]\n[^\n]",	-- Handle word break
	      function (s)	-- within a sentence.
		 return s:gsub("\n", " ")
	      end)
   return s
end

local intro_help = [[
Quick Start

To enter a board, type "edit" and fill in the initial configuration.
Next type "help commands" to learn how to solve the puzzle.

Tips

The cell in the upper left-hand corner is at row 1 and column 1.
The program uses a right-handed coordinate system.

If the first argument of a command is the word "help", a brief help
message about the command is printed.  Thus "s help" gives:

s <digit> <row> <col> -- square determines digit

A detailed view of each cell is obtained with the command "details"
and a normal view is obtained with the command "normal".  In the
normal view, a cell is blank if it has not been determined, but in a
detailed view, each digit for a cell that has not been eliminated is
shown using a dot.  The position of the dot indicates the digit.  The
digits are positioned in a 3x3 grid, and digit 4 is in the second row
and the first column.

Type "help board" to learn how to load a board from a text file.  When
a file name is present on the command line used to start this program,
it loads the board in the file on start up.

The greatest challenge is to solve difficult puzzles without using
the detailed cell view.
]]

intro_help = wrap(intro_help)

topics.intro = intro_help

local commands_help = [[
Commands

GTK Sudoku is command driven.  A command is an operator followed by a
sequence of arguments, each separated by spaces.  Each argument is
normally a non-zero digit.  For example, the form of the one place for
digit in square command is:

s <digit> <row> <col>

and the instance "s 5 8 1" says the number 5 has been eliminated from
all but one cell in the square at row 8 and column 1.  The cell in the
upper left-hand corner is at row 1 and column 1.

This command alone can be used to solve most easy Sudoku puzzles.  The
other basic rules are: one place for digit in row (r), one place place
for digit in column (c), and digit is the only one possible in cell
(d).  These are the workhorse commands.  The command "help basic"
describes them.  The commands "help advanced" and "help pair" describe
commands used for difficult puzzles.

Other useful commands:

edit -- enter or modify a board.

index -- list all commands.

details -- show detailed cell view.

normal -- show normal cell view.

Other help topics: board, history, and impatient.  Be
sure to read the introduction in the help menu.
]]

commands_help = wrap(commands_help)

topics.commands = commands_help

local board_help = [[
Boards

A GTK Sudoku board can be saved in a text file, and then reloaded.
The format used by the text file is very simple, and you can enter a
puzzle using any editor and then open it, instead of using the entry
form provided by this program.

When a GTK Sudoku board is loaded, all but ten characters in the file
are ignored: period and the nine non-zero digits.  A period is used to
denote a blank cell.  Other characters can be used to aid readability.
Common practice is to list the nine cell characters for each row on
a separate line of text.
]]

board_help = wrap(board_help)

topics.board = board_help

local history_help = [[
History commands

back -- replace the current board with the one most recently saved.

swap -- swap the current board with the one most recently saved.

new -- make a blank board.
]]

history_help = wrap(history_help)

topics.history = history_help

local basic_help = [[
Basic commands

s <digit> <row> <col> -- square determines digit.  Digit is
eliminated from all but one cell in a square.

r <digit> <row> -- row determines digit.  Digit is eliminated
from all but one cell in a row.

c <digit> <col> -- column determines digit.  Digit is eliminated
from all but one cell in a column.

d <digit> <row> <col> -- determine digit.  Digit is the only one
possible in the given cell.

hint -- print a simple hint.  One of the above rules is
applicable if a hint is suggested.
]]

basic_help = wrap(basic_help)

topics.basic = basic_help

local advanced_help = [[
Advanced commands

rs <digit> <row> <col> -- digit in one row in square.
Digits in other squares and the same row are eliminated.

cs <digit> <row> <col> -- digit in one column in square.
Digits in other squares and the same column are eliminated.

sr <digit> <row> <col> -- digit in one square of row.
Digits in other rows of the same square are eliminated.

sc <digit> <row> <col> -- digit in one square of column.
Digits in other columns of the same square are eliminated.

These commands are typically used when a detailed cell view is
enabled.
]]

advanced_help = wrap(advanced_help)

topics.advanced = advanced_help

local pair_help = [[
Commands involving pairs

ps <digit> <digit> <row> <col> -- two places for pair in square.
Other digits in the two places are eliminated.

pr <digit> <digit> <row> -- two places for pair in row.
Other digits in the two places are eliminated.

pc <digit> <digit> <col> -- two places for pair in column.
Other digits in the two places are eliminated.

sps <digit> <digit> <row> <col> -- same pair in square.
Two cells contain only the same pair, so
other occurrences of the digits in the square are eliminated.

spr <digit> <digit> <row> -- same pair in row.
Two cells contain only the same pair, so
other occurrences of the digits in the row are eliminated.

spc <digit> <digit> <col> -- same pair in column.
Two cells contain only the same pair, so
other occurrences of the digits in the column are eliminated.

When there are two places for pair in square, row, or column, they are
sometimes called a hidden pair.  When two cells contain only the same
pair in square, row, or column, they are sometimes called a naked
pair.
]]

pair_help = wrap(pair_help)

topics.pair = pair_help

local impatient_help = [[
Commands that try many rules.

p -- propagate all singletons.  Determines each cell in which
all but one possibility has been eliminated.

simp -- repeatly apply simple rules.  The simple rules are,
one place for digit in square, one place for digit in row, one place
place for digit in column, and digit is the only one possible in cell.

all -- try all rules.  Stops when one rule makes progress.

solve -- repeatly apply all rules.  Stops when no rule is applicable.
]]

impatient_help = wrap(impatient_help)

topics.impatient = impatient_help

local function do_help(topic)
   local s
   if topic then
      s = topics[topic]
      if not s then
	 return "no help entry for " .. topic
      end
   else
      s = intro_help
   end
   show(s);
end

-- Command processing

-- The command table maps a command name to a command.

local cmds = {}

-- Commands

-- Each command is a table with three fields.
-- op      the function implementing the command
-- nargs   the number of arguments expected by the command
-- help    a one line help message

function eval(name, ...)
   local cmd = cmds[name]
   if not cmd then
      if name == "edit" then
	 return do_edit(...)
      elseif name == "help" then
	 return do_help(...)
      elseif name == "index" then
	 return do_help(name, ...)
      else
	 return "command " .. name .. " unknown"
      end
   end
   local arg1 = select(1, ...)
   if arg1 == "help" then
      return do_help(name)
   end
   local nargs = select('#', ...)
   if nargs ~= cmd.nargs then
      return cmd.help
   end
   for i=1,nargs do
      local arg = select(i, ...)
      if type(arg) ~= "number" or arg < 1 or arg > 9 then
         return cmd.help
      end
   end
   if not it then
      print_blank_board()
      return "no board"
   end
   push()
   local status, e, msg = pcall(cmd.op,...)
   it:print_all()
   if not status then
      msg = e
      e = false
   end
   if e then
      msg = msg or "yes"
   else
      msg = msg or "no"
   end
   return msg
end

-- History commands

cmds.back = {}
cmds.back.nargs = 0
cmds.back.help = "back -- replace the current board with the one most recently saved"
topics.back = history_help
cmds.back.op = back

cmds.swap = {}
cmds.swap.nargs = 0
cmds.swap.help = "swap -- swap the current board with the one most recently saved"
topics.swap = history_help
cmds.swap.op = swap

cmds.new = {}
cmds.new.nargs = 0
cmds.new.help = "new -- make a blank board"
topics.new = history_help
cmds.new.op = new

-- Cell details

cmds.details = {}
cmds.details.nargs = 0
cmds.details.help = "details -- show detailed cell view"
topics.details = intro_help
function cmds.details.op()
   details = true
   return false, "detailed view enabled"
end

cmds.normal = {}
cmds.normal.nargs = 0
cmds.normal.help = "normal -- show normal cell view"
topics.normal = intro_help
function cmds.normal.op()
   details = false
   return false, "detailed view disabled"
end

-- Basic or simple commands

cmds.s = {}
cmds.s.nargs = 3
cmds.s.help = "s <digit> <row> <col> -- square determines digit"
topics.s = basic_help
function cmds.s.op(d, row, col)
   return it:one_place_in_square(d, row, col)
end

cmds.r = {}
cmds.r.nargs = 2
cmds.r.help = "r <digit> <row> -- row determines digit"
topics.r = basic_help
function cmds.r.op(d, row)
   return it:one_place_in_row(d, row)
end

cmds.c = {}
cmds.c.nargs = 2
cmds.c.help = "c <digit> <col> -- column determines digit"
topics.c = basic_help
function cmds.c.op(d, col)
   return it:one_place_in_column(d, col)
end

cmds.d = {}
cmds.d.nargs = 3
cmds.d.help = "d <digit> <row> <col> -- determine digit"
topics.d = basic_help
function cmds.d.op(d, row, col)
   local r1, r2 = spread(row)
   local c1, c2 = spread(col)
   return it:propagate_elimination(r1, c1, r2, c2, d)
end

-- Advanced square rules

cmds.rs = {}
cmds.rs.nargs = 3
cmds.rs.help = "rs <digit> <row> <col> -- digit in one row in square"
topics.rs = advanced_help
function cmds.rs.op(d, row, col)
   return it:one_row_in_square(d, row, col)
end

cmds.cs = {}
cmds.cs.nargs = 3
cmds.cs.help = "cs <digit> <row> <col> -- digit in one column in square"
topics.cs = advanced_help
function cmds.cs.op(d, row, col)
   return it:one_column_in_square(d, row, col)
end

cmds.sr = {}
cmds.sr.nargs = 3
cmds.sr.help = "sr <digit> <row> <col> -- digit in one square of row"
topics.sr = advanced_help
function cmds.sr.op(d, row, col)
   return it:one_square_for_row(d, row, col)
end

cmds.sc = {}
cmds.sc.nargs = 3
cmds.sc.help = "sc <digit> <row> <col> -- digit in one square of column"
topics.sc = advanced_help
function cmds.sc.op(d, row, col)
   return it:one_square_for_column(d, row, col)
end

-- Rules involving pairs

cmds.ps = {}
cmds.ps.nargs = 4
cmds.ps.help =
   "ps <digit> <digit> <row> <col> -- two places for pair in square"
topics.ps = pair_help
function cmds.ps.op(d1, d2, row, col)
   return it:two_places_for_pair_in_square(d1, d2, row, col)
end

cmds.pr = {}
cmds.pr.nargs = 3
cmds.pr.help = "pr <digit> <digit> <row> -- two places for pair in row"
topics.pr = pair_help
function cmds.pr.op(d1, d2, row)
   return it:two_places_for_pair_in_row(d1, d2, row)
end

cmds.pc = {}
cmds.pc.nargs = 3
cmds.pc.help = "pc <digit> <digit> <col> -- two places for pair in column"
topics.pc = pair_help
function cmds.pc.op(d1, d2, col)
   return it:two_places_for_pair_in_column(d1, d2, col)
end

cmds.sps = {}
cmds.sps.nargs = 4
cmds.sps.help = "sps <digit> <digit> <row> <col> -- same pair in square"
topics.sps = pair_help
function cmds.sps.op(d1, d2, row, col)
   return it:same_pair_in_square(d1, d2, row, col)
end

cmds.spr = {}
cmds.spr.nargs = 3
cmds.spr.help = "spr <digit> <digit> <row> -- same pair in row"
topics.spr = pair_help
function cmds.spr.op(d1, d2, row)
   return it:same_pair_in_row(d1, d2, row)
end

cmds.spc = {}
cmds.spc.nargs = 3
cmds.spc.help = "spc <digit> <digit> <col> -- same pair in column"
topics.spc = pair_help
function cmds.spc.op(d1, d2, col)
   return it:same_pair_in_column(d1, d2, col)
end

-- Rules for triples have not been implemented.

-- Hints

cmds.hint = {}
cmds.hint.nargs = 0
cmds.hint.help = "hint -- print a simple hint"
topics.hist = basic_help
function cmds.hint.op()
   return it:hint()
end

-- For the impatient

cmds.p = {}
cmds.p.nargs = 0
cmds.p.help = "p -- propagate all singletons"
topics.p = impatient_help
function cmds.p.op()
   return it:propagate_all_singletons()
end

cmds.simp = {}
cmds.simp.nargs = 0
cmds.simp.help = "simp -- repeatly apply simple rules"
topics.simp = impatient_help
function cmds.simp.op()
   return it:simp()
end

cmds.all = {}
cmds.all.nargs = 0
cmds.all.help = "all -- try all rules"
topics.all = impatient_help
function cmds.all.op()
   return it:all()
end

cmds.solve = {}
cmds.solve.nargs = 0
cmds.solve.help = "solve -- repeatly apply all rules"
topics.solve = impatient_help
function cmds.solve.op()
   local e = false
   local f = false
   repeat
      f = it:all()
      e = e or f
   until not f
   return e
end

local function mk_index()	-- Construct the index
   local array = {}
   for name, cmd in pairs(cmds) do
      array[1 + #array] = cmd.help
   end
   table.sort(array)
   local index = "Index"
   for i,v in ipairs(array) do
      index = index .. "\n\n" .. v
   end
   return index
end

topics.index = mk_index()
