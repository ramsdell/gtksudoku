/*
 * The edit dialog for GTK Sudoku.
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

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "config.h"
#include "gtksudoku.h"
#include "board.h"
#include "sudokuboard.h"

/* Sets the board currently displayed by this widget.  The value in
   each board's cell is initialized from the board string, as long as
   it has 81 valid board characters.  A cell is initialized using the
   function boardchar2val. */

static void
sudoku_dialog_set(SudokuBoard *widget, const char *board)
{
  if (boardlen(board) != DIGITS * DIGITS)
    return;
  int row; int col;
  for (row = 0; row < DIGITS; row++)
    for (col = 0; col < DIGITS; col++) {
      if (isboardchar(*board)) {
	int val = boardchar2val(*board);
	SUDOKU_BOARD_GET_CLASS(widget)->set_val(widget, row, col, val, 0);
      }
      board++;
    }
}

/* #define DEMO_BOARD_CHANGED_SIGNAL */

#if defined DEMO_BOARD_CHANGED_SIGNAL
static void
board_changed_callback(SudokuBoard *board, int row, int col,
		       int val, int mode)
{
  g_print("::board-changed - %d %d %d %d\n", row, col, val, mode);
}
#endif

/* Create an Sudoku board editor dialog.  If the edits are accepted,
   an 81 character string is returned in which period is used to
   represent a blank cell.  Otherwise, the null string is returned.
   The string should be freed when it is no longer of use. */

char *
sudoku_edit_dialog(GtkWidget *window, const char *board)
{
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWidget *dialog = gtk_dialog_new_with_buttons(PACKAGE_NAME " Editor",
						  GTK_WINDOW(window),
						  flags,
						  GTK_STOCK_APPLY,
						  GTK_RESPONSE_APPLY,
						  GTK_STOCK_CANCEL,
						  GTK_RESPONSE_CANCEL,
						  NULL);
  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *widget = sudoku_board_new(TRUE);
#if defined DEMO_BOARD_CHANGED_SIGNAL
  g_signal_connect(widget, SUDOKU_BOARD_CHANGED_SIGNAL_NAME,
		   G_CALLBACK(board_changed_callback), NULL);
#endif
  SudokuBoard *grid = SUDOKU_BOARD(widget);
  sudoku_dialog_set(grid, board);
  gtk_container_add_with_properties(GTK_CONTAINER(content_area), widget,
				    "expand", TRUE,
				    "fill", TRUE,
				    NULL);
#if defined SUDOKU_BOARD_MIN_ASPECT || defined SUDOKU_BOARD_MAX_ASPECT
  GdkGeometry hints;
  hints.min_aspect = SUDOKU_BOARD_MIN_ASPECT;
  hints.max_aspect = SUDOKU_BOARD_MAX_ASPECT;
  gtk_window_set_geometry_hints(GTK_WINDOW(window),
				GTK_WIDGET(board),
				&hints,
				GDK_HINT_ASPECT);
#endif
  gtk_widget_show_all(widget);
  char *result = NULL;
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_APPLY) {
    result = malloc(DIGITS * DIGITS + 1);
    if (!result)
      exit(EXIT_FAILURE);
    char *b = result;
    int row; int col;
    for (row = 0; row < DIGITS; row++)
      for (col = 0; col < DIGITS; col++) {
	int val = SUDOKU_BOARD_GET_CLASS(grid)->get_val(grid, row, col);
	*b++ = val2boardchar(val);
      }
    *b = 0;
  }
  gtk_widget_destroy(dialog);
  return result;
}
