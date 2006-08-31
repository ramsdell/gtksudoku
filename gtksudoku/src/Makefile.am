bin_PROGRAMS = gtksudoku
noinst_LIBRARIES = liblua.a
noinst_PROGRAMS = bin2c u2d

if HAVE_WINDRES
  grid_resource = grid.$(OBJEXT)
else
  grid_resource =
endif

gtksudoku_SOURCES = gtksudoku.h gtksudoku.c sudokuedit.h sudokuedit.c	\
sudokuboard.h sudokuboard.c sudokucell.h sudokucell.c interp.h		\
interp.c showtext.h showtext.c board.h board.c

nodist_gtksudoku_SOURCES = sudoku.h sudokuboardmarshallers.h	\
sudokuboardmarshallers.c grid.h

liblua_a_SOURCES = lapi.c ldebug.h liolib.c lobject.h lstring.h		\
luaconf.h lapi.h ldo.c llex.c lopcodes.c lstrlib.c lua.h print.c	\
lauxlib.c ldo.h llex.h lopcodes.h ltable.c lualib.h lauxlib.h ldump.c	\
llimits.h loslib.c ltable.h lundump.c lbaselib.c lfunc.c lmathlib.c	\
lparser.c ltablib.c lundump.h lcode.c lfunc.h lmem.c lparser.h ltm.c	\
lvm.c lcode.h lgc.c lmem.h lstate.c ltm.h lvm.h ldblib.c lgc.h		\
loadlib.c lstate.h lua.c lzio.c ldebug.c linit.c lobject.c lstring.c	\
luac.c lzio.h

gtksudoku_CFLAGS = @GTK_CFLAGS@
gtksudoku_LDADD = $(grid_resource) liblua.a @GTK_LIBS@
gtksudoku_DEPENDENCIES = $(grid_resource) liblua.a

grid.$(OBJEXT):	grid.rc grid.ico
	@WINDRES@ --include-dir=$(srcdir) $(srcdir)/grid.rc $@

u2d_SOURCES = u2d.c
bin2c_SOURCES = bin2c.c

sudoku.h:	bin2c$(EXEEXT) sudoku.lua
	./bin2c -o $@ -n sudoku.lua $(srcdir)/sudoku.lua

sudokuboardmarshallers.c: sudokuboardmarshallers.txt
	glib-genmarshal --prefix sudoku_board --body $< > $@

sudokuboardmarshallers.h: sudokuboardmarshallers.txt
	glib-genmarshal --prefix sudoku_board --header $< > $@

grid.h:	grid.xpm
	cp $(srcdir)/grid.xpm $@

BUILT_SOURCES = $(nodist_gtksudoku_SOURCES)

CLEANFILES = $(nodist_gtksudoku_SOURCES)

EXTRA_DIST = sudoku.lua sudokuboardmarshallers.txt grid.xpm grid.rc	\
grid.ico
