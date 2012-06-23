/* A drawing area on which a Sudoku cell is drawn. */

#ifndef SUDOKUCELL_H
#define SUDOKUCELL_H

G_BEGIN_DECLS

#define SUDOKU_CELL_TYPE (sudoku_cell_get_type())
#define SUDOKU_CELL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SUDOKU_CELL_TYPE, SudokuCell))
#define SUDOKU_CELL_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), SUDOKU_CELL, SudokuCellClass))
#define IS_SUDOKU_CELL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SUDOKU_CELL_TYPE))
#define IS_SUDOKU_CELL_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), SUDOKU_CELL_TYPE))
#define SUDOKU_CELL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SUDOKU_CELL_TYPE, SudokuCellClass))

typedef struct _SudokuCell SudokuCell;
typedef struct _SudokuCellClass SudokuCellClass;

struct _SudokuCell
{
  GtkDrawingArea parent;
};

struct _SudokuCellClass
{
  GtkDrawingAreaClass parent_class;

  /* Update the val associated with the cell.  The val parameter is
     the set of digits that have not yet been eliminated.  The mode is
     non-zero if a dot pattern is to be drawn when the cell has only
     one possible digit, otherwise a numeral is drawn. */
  void (*set_val)(SudokuCell *cell, int val, int mode);

  /* Get the val associated with the cell. */
  int (*get_val)(SudokuCell *cell);
};

GType sudoku_cell_get_type(void);
GtkWidget *sudoku_cell_new(int row, int col, gboolean editable);

G_END_DECLS

#endif
