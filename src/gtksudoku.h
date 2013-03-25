/* Constants used through out GTK Sudoku. */

#ifndef GTKSUDOKU_H
#define GTKSUDOKU_H

/* Number of cells on the side of a square */
#define SIDES 3
/* Number of digits and cells on the side of a board */
#define DIGITS (SIDES * SIDES)
/* Bit set with all digits enabled */
#define ALL ((1 << DIGITS) - 1)

/* Minimum aspect ration of a board cell */
#define SUDOKU_BOARD_MIN_ASPECT 0.80
/* Maximum aspect ration of a board cell */
#define SUDOKU_BOARD_MAX_ASPECT 1.25

#endif
