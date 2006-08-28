/*
 * A table of Sudoku cells that form a board.
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

/*
 * This class is quite straightforward.  It simply implements a
 * Sudoku board as a GTK 9x9 table of Sudoku cells of equal size.
 *
 * When an editable board is created, the non-zero digits, period, and
 * space bar keys change a cell in the board.  A signal is emitted
 * when a cell's value changes via a key press.
 *
 * Davyd Madeley's GTK+ clock face widget provided ideas for this
 * widget.
 */

#include "config.h"
#include "gtksudoku.h"
#include <gtk/gtk.h>
#include "sudokuboardmarshallers.h"
#include "sudokucell.h"
#include "sudokuboard.h"

#define SUDOKU_BOARD_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj), SUDOKU_BOARD_TYPE, SudokuBoardPrivate))

G_DEFINE_TYPE(SudokuBoard, sudoku_board, GTK_TYPE_TABLE);

static void sudoku_board_set_val(SudokuBoard *board,
				 int row, int col, int val, int mode);
static int sudoku_board_get_val(SudokuBoard *board, int row, int col);

typedef struct _SudokuBoardPrivate SudokuBoardPrivate;

struct _SudokuBoardPrivate
{
  SudokuCell *board[DIGITS][DIGITS];
};

static void
sudoku_board_class_init(SudokuBoardClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS(klass);
  klass->set_val = sudoku_board_set_val;
  klass->get_val = sudoku_board_get_val;

  g_type_class_add_private(obj_class, sizeof(SudokuBoardPrivate));

  /* register the board changed signal */
  g_signal_new(SUDOKU_BOARD_CHANGED_SIGNAL_NAME,
	       G_OBJECT_CLASS_TYPE(obj_class),
	       G_SIGNAL_RUN_FIRST,
	       0, NULL, NULL,
	       sudoku_board_VOID__INT_INT_INT_INT,
	       G_TYPE_NONE, 4,
	       G_TYPE_INT,
	       G_TYPE_INT,
	       G_TYPE_INT,
	       G_TYPE_INT);
}

static void
sudoku_board_init(SudokuBoard *board)
{
}

/* Redirect a set_val operation to the correct cell. */

static void
sudoku_board_set_val(SudokuBoard *board, int row, int col, int val, int mode)
{
  g_return_if_fail(row >= 0 && row < DIGITS);
  g_return_if_fail(col >= 0 && col < DIGITS);
  SudokuBoardPrivate *priv = SUDOKU_BOARD_GET_PRIVATE(board);
  SudokuCell *cell = priv->board[row][col];
  SUDOKU_CELL_GET_CLASS(cell)->set_val(cell, val, mode);
}

static int
sudoku_board_get_val(SudokuBoard *board, int row, int col)
{
  g_return_val_if_fail(row >= 0 && row < DIGITS, -1);
  g_return_val_if_fail(col >= 0 && col < DIGITS, -1);
  SudokuBoardPrivate *priv = SUDOKU_BOARD_GET_PRIVATE(board);
  SudokuCell *cell = priv->board[row][col];
  return SUDOKU_CELL_GET_CLASS(cell)->get_val(cell);
}

GtkWidget *
sudoku_board_new(gboolean editable)
{
  SudokuBoard *board = g_object_new(SUDOKU_BOARD_TYPE, NULL);
  SudokuBoardPrivate *priv = SUDOKU_BOARD_GET_PRIVATE(board);
  GtkTable *table = GTK_TABLE(board);
  GtkAttachOptions options = GTK_FILL | GTK_EXPAND | GTK_SHRINK;
  int row; int col;
  for (row = 0; row < DIGITS; row++)
    for (col = 0; col < DIGITS; col++) {
      GtkWidget *widget = sudoku_cell_new(row, col, editable);
      priv->board[row][col] = SUDOKU_CELL(widget);
      gtk_table_attach(table, widget,
		       col, col + 1, row, row + 1,
		       options, options, 0, 0);
    }
  return GTK_WIDGET(board);
}
