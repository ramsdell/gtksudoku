/* A table of Sudoku cells that form a board. */

#ifndef SUDOKUBOARD_H
#define SUDOKUBOARD_H

G_BEGIN_DECLS

#define SUDOKU_BOARD_TYPE (sudoku_board_get_type())
#define SUDOKU_BOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SUDOKU_BOARD_TYPE, SudokuBoard))
#define SUDOKU_BOARD_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), SUDOKU_BOARD, SudokuBoardClass))
#define IS_SUDOKU_BOARD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SUDOKU_BOARD_TYPE))
#define IS_SUDOKU_BOARD_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), SUDOKU_BOARD_TYPE))
#define SUDOKU_BOARD_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SUDOKU_BOARD_TYPE, SudokuBoardClass))

typedef struct _SudokuBoard SudokuBoard;
typedef struct _SudokuBoardClass SudokuBoardClass;

struct _SudokuBoard
{
  GtkTable parent;
};

struct _SudokuBoardClass
{
  GtkTableClass parent_class;

  /* Update the val associated with the cell at the given row and col.
     The val parameter is the set of digits that have not yet been
     eliminated.  The mode is non-zero if a dot pattern is to be drawn
     when the cell has only one possible digit, otherwise a numeral is
     drawn. */
  void (*set_val)(SudokuBoard *board, int row, int col, int val, int mode);

  /* Get the val associated with the cell at the given location. */
  int (*get_val)(SudokuBoard *board, int row, int col);
};

GType sudoku_board_get_type(void);
GtkWidget *sudoku_board_new(gboolean editable);

/* When an editable board is created, the non-zero digits, period, and
   space bar keys change a cell in the board.  A signal is emitted
   when a cell's value changes via a key press.  The name of the
   signal folows. */

#define SUDOKU_BOARD_CHANGED_SIGNAL_NAME "sudoku-board-changed"

/* The signature of a callback for this signal is the same as the one
   for the set_val class member. */

G_END_DECLS

#endif
