/*
 * The main routine for GTK Sudoku.
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
 * The main window of GTK Sudoku contains a Sudoku board, a status
 * line, and a command entry line.  The function main builds the main
 * window, adds in a menu bar, and links the widgets with the command
 * interpreter used to drive this program.
 */

#include <stdlib.h>
#include <stdio.h>
#include <glib/gstdio.h>
#include "config.h"
#include "gtksudoku.h"
#include <gtk/gtk.h>
#include "sudokuboard.h"
#include "sudokuedit.h"
#include "showtext.h"
#include "board.h"
#include "interp.h"
#include "grid.h"

#if defined TEMPORARY_WIN32_INSTALLER_HACK
#include <glib.h>
GType
g_type_register_static_simple (GType             parent_type,
			       const gchar      *type_name,
			       guint             class_size,
			       GClassInitFunc    class_init,
			       guint             instance_size,
			       GInstanceInitFunc instance_init,
			       GTypeFlags flags)
{
  GTypeInfo info;

  info.class_size = class_size;
  info.base_init = NULL;
  info.base_finalize = NULL;
  info.class_init = class_init;
  info.class_finalize = NULL;
  info.class_data = NULL;
  info.instance_size = instance_size;
  info.n_preallocs = 0;
  info.instance_init = instance_init;
  info.value_table = NULL;

  return g_type_register_static (parent_type, type_name, &info, flags);
}
#endif

/* Size of the buffer used to read a board from a text file. */
#define NBOARD (DIGITS * DIGITS * DIGITS)

/* Functions provided to the command interpreter. */

/* Change the board pragmatically. */

static SudokuBoard *board;

void
interp_set_val(int row, int col, int val, int mode)
{
  SUDOKU_BOARD_GET_CLASS(board)->set_val(board, row, col, val, mode);
}

/* Edit a board with a GTK Sudoku editor dialog. */

static GtkWidget *window;

char *
interp_edit(const char *board)
{
  return sudoku_edit_dialog(window, board);
}

/* Show wrapped text in a dialog window. */

void
interp_show(char *text)
{
  show_text(window, text);
  free(text);
}

/* Functions provided to the command interpreter. */

static GtkEntry *status;

static void
set_status(char *malloced_message)
{
  if (malloced_message) {
    gtk_entry_set_text(status, malloced_message);
    free(malloced_message);
  }
  else
    gtk_entry_set_text(status, "");
}

/* Load a board from a file. */

static void
load_file(const char *file_name)
{
  char board[NBOARD + 1];
  FILE *in = g_fopen(file_name, "r");
  if (!in) {
    gtk_entry_set_text(status, "failed to open file");
    return;
  }
  size_t n = fread(board, 1, NBOARD, in);
  if (ferror(in)) {
    gtk_entry_set_text(status, "failed to read file");
    fclose(in);
    return;
  }
  fclose(in);
  board[n] = 0;
  set_status(interp_load(board));
}

/* Save a board to a file. */

static void
save_file(const char *file_name)
{
  char *board;
  char *msg = interp_save(&board);
  if (*board) {
    FILE *out = g_fopen(file_name, "w");
    if (!out) {
      gtk_entry_set_text(status, "failed to open file");
      free(board); free(msg);
      return;
    }
    fputs(board, out);
    fclose(out);
    free(board);
  }
  set_status(msg);
}

/* Evaluate a command and print the result in the status line. */

static void
entry_callback(GtkWidget *widget, GtkWidget *entry)
{
  const gchar *cmd = gtk_entry_get_text(GTK_ENTRY(entry));
  set_status(interp_eval(cmd));
  gtk_entry_set_text(GTK_ENTRY(entry), "");
}

/* Menu bar support. */

static void
open_file(void)
{
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new("Open File",
				       GTK_WINDOW(window),
				       GTK_FILE_CHOOSER_ACTION_OPEN,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				       NULL);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    load_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
}

static void
save_file_as(void)
{
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new("Save File",
				       GTK_WINDOW(window),
				       GTK_FILE_CHOOSER_ACTION_SAVE,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
						 TRUE);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    save_file(filename);
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
}

/* Help menu content. */

static const char intro[] =
  PACKAGE_NAME ": A Logic Puzzle Solving Aid"
  "\n\n"
  "A Sodoku logic puzzle is solved by filling each cell in a board "
  "so that every row, column, and 3x3 square contains the "
  "digits one through nine.  This program eliminates much of the "
  "drudgery of solving a puzzle and provides educational tips should "
  "the path to the solution become obscured.  It is different from "
  "most other programs in this category, because users specify the "
  "rule that justifies each change to the Sudoku board.  The program "
  "will fail to apply a rule if its preconditions are not met, thus "
  "detecting silly mistakes early."
  "\n\n"
  "Quick Start"
  "\n\n"
  "To enter a board, type \"edit\" and fill in the initial configuration. "
  "Next type \"help\" to learn how to solve the puzzle.";

static void
help_intro(void)
{
  show_text(window, intro);
}

static const char license[] =
  "Copyright (C) 2006 John D. Ramsdell for the non-Lua parts.  "
  "Copyright (C) 1994-2006 Lua.org, PUC-Rio for the Lua parts." 
  "\n\n"
  PACKAGE_NAME " License"
  "\n\n"
  "This program is free software; you can redistribute it and/or "
  "modify it under the terms of the GNU General Public License as "
  "published by the Free Software Foundation; either version 2 of the "
  "License, or (at your option) any later version."
  "\n\n"
  "This program is distributed in the hope that it will be useful, but "
  "WITHOUT ANY WARRANTY; without even the implied warranty of "
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
  "See the GNU General Public License for more details."
  "\n\n"
  "You should have received a copy of the GNU General Public License "
  "along with this program; if not, write to the Free Software "
  "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA "
  "02110-1301 USA"
  "\n\n"
  "Lua License"
  "\n\n"
  "Lua is licensed under the terms of the MIT license.  "
  "This means that Lua is free software and can be used for "
  "both academic and commercial purposes at absolutely no cost."
  "\n\n"
  "For details and rationale, see http://www.lua.org/license.html.";

static const char comments[] =
  "A logic puzzle solving aid";

static void
help_about(void)
{
  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), PACKAGE_NAME);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), PACKAGE_VERSION);
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
				 "Copyright (C) John D. Ramsdell");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), comments);
  gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(dialog), TRUE);
  gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), license);
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), PACKAGE_BUGREPORT);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

static GtkActionEntry main_entries[] = {
  {"FileMenu", 0, "_File"},
  {"HelpMenu", 0, "_Help"},
  {"Open", GTK_STOCK_OPEN, "_Open", "<control>O",
   "Open a file and load board", open_file},
  {"SaveAs", GTK_STOCK_SAVE_AS, "Save _As", "<control>S",
   "Save a board in a file", save_file_as},
  {"Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q",
   "Quit the program", gtk_main_quit},
  {"Intro", GTK_STOCK_HELP, "_Intro", "<control>I",
   "Help about this program", help_intro},
  {"About", GTK_STOCK_ABOUT, "_About", "<control>A",
   "About this program", help_about},
};

static const char *main_ui_description =
  "<ui>"
  "  <menubar name='MainMenu'>"
  "    <menu action='FileMenu'>"
  "      <menuitem action='Open'/>"
  "      <menuitem action='SaveAs'/>"
  "      <menuitem action='Quit'/>"
  "    </menu>"
  "    <menu action='HelpMenu'>"
  "      <menuitem action='Intro'/>"
  "      <menuitem action='About'/>"
  "    </menu>"
  "  </menubar>"
  "</ui>";

static GtkWidget *
get_menubar_menu(GtkWidget *window, const gchar *path,
		 GtkActionEntry *entries, gint n_entries,
		 const gchar* ui_description)
{
  GtkActionGroup *action_group = gtk_action_group_new("MenuActions");
  gtk_action_group_add_actions(action_group, entries, n_entries, window);

  GtkUIManager *ui_manager = gtk_ui_manager_new();
  gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);

  GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group(ui_manager);
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

  GError *error = NULL;
  if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description,
					 -1, &error)) {
    g_message("building menus failed: %s", error->message);
    g_error_free(error);
    exit(EXIT_FAILURE);
  }
  return gtk_ui_manager_get_widget(ui_manager, path);
}

int
main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)grid_xpm);
  GList *list = gtk_window_get_default_icon_list();
  list = g_list_prepend(list, pixbuf);
  gtk_window_set_default_icon_list(list);

  GtkWidget *box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), box);

  /* Menu */

  GtkWidget *menu_bar =
    get_menubar_menu(window, "/MainMenu", main_entries,
		     G_N_ELEMENTS(main_entries), main_ui_description);
  gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, 0);

  /* Main content */

  GtkWidget *cell = sudoku_board_new(FALSE);
  board = SUDOKU_BOARD(cell);
  gtk_box_pack_start(GTK_BOX(box), cell, TRUE, TRUE, 0);

  GtkWidget *widget = gtk_entry_new();
  GTK_WIDGET_UNSET_FLAGS(widget, GTK_CAN_FOCUS);
  gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
  status = GTK_ENTRY(widget);
  gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

  GtkWidget *entry = gtk_entry_new();
  g_signal_connect(G_OBJECT(entry), "activate",
		   G_CALLBACK(entry_callback),
		   (gpointer)entry);
  gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);

  char *msg = interp_init();
  if (msg) {
    printf("%s\n", msg);
    return EXIT_FAILURE;
  }

  if (argc > 1)
    load_file(argv[1]);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
