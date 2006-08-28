/* Boards as character strings. */

#ifndef BOARD_H
#define BOARD_H

/* Is character a valid cell descriptor. */
int isboardchar(int c);

/* Returns the number of valid cell descriptors in the string. */
size_t boardlen(const char *s);

/* Converts a valid cell descriptor to a set of digits. */
int boardchar2val(int c);

/* Converts a set of digits to a cell descriptor. */
int val2boardchar(int val);

#endif
