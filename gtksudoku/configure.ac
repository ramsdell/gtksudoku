AC_INIT(GTK Sudoku, 0.0, http://luaforge.net/projects/gtksudoku/, gtksudoku)

AC_CONFIG_SRCDIR(src/gtksudoku.c)

AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(src/config.h)

AC_PROG_CC
AC_STDC_HEADERS

# Add warning when using GCC
if test "X$GCC" = Xyes ; then
  CFLAGS="$CFLAGS -Wall"
# The program was developed with:
# CFLAGS="$CFLAGS -Wall -std=iso9899:1999 -Wstrict-prototypes -Wpointer-arith"
fi

AC_PROG_RANLIB

AM_PATH_GTK_2_0(2.8.0,, [AC_MSG_ERROR([Cannot find GTK+])])

AC_CONFIG_FILES([Makefile gtksudoku.spec src/Makefile])

AC_OUTPUT
