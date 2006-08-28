/* Lua C routines that link the script with the GUI. */

#ifndef INTERP_H
#define INTERP_H

/* Functions this module uses. */

/* Update the val associated with the cell at the given row and col.
   The val parameter is the set of digits that have not yet been
   eliminated.  The mode is non-zero if a dot pattern is to be drawn
   when the cell has only one possible digit, otherwise a numeral is
   drawn. */

void interp_set_val(int row, int col, int val, int mode);

/* Edit a Sudoku board.  If the returned board is not NULL, the board
   will be freed after use. */

char *interp_edit(const char *board);

/* Show text in a dialog window.  The text should be freed after use. */

void interp_show(char *text);

/* Functions this module provides. */

/* The interpreter evaluates the command, and returns a message in
   response.  If the message is not NULL, the message should be freed
   after use. */

char *interp_eval(const char *cmd);

/* The remaining functions return a non-NULL message on error.  If the
   message is not NULL, the message should be freed after use. */

/* Load a board from a string.  Returns a non-NULL message on
   error. */

char *interp_load(const char *board);

/* Save a board as a string.  Sets board to NULL when no board is
   loaded.  If the board is not NULL, the board should be freed after
   use.  Returns a non-NULL message on error. */

char *interp_save(char **board);

/* Initialize the interpreter.  Returns a non-NULL message on error.
   There is no point in continuing if this function reports an
   error. */

char *interp_init(void);

#endif
