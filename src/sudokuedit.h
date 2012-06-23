/* The edit dialog for GTK Sudoku. */

#ifndef SUDOKUEDIT_H
#define SUDOKUEDIT_H

/* Create an Sudoku board editor dialog.  The value in each board's
   cell is initialized from the board string, as long as it has 81
   valid board characters.  A cell is initialized using the function
   boardchar2val.  If the edits are accepted, an 81 character string
   is returned in which period is used to represent a blank cell.
   Otherwise, the null string is returned.  The string should be freed
   when it is no longer of use. */
char *sudoku_edit_dialog(GtkWidget *window, const char *board);

#endif
