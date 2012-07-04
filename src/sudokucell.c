/*
 * A drawing area on which a Sudoku cell is drawn.
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
 * Much of what is in this file is fairly straightforward with one
 * exception.  This class draws one cell of a Sudoku board.  Normally,
 * if there is but one possible value for a cell, it draws a numeral,
 * otherwise it draws a dot pattern that represents the possible
 * values for the cell.  No matter what glyph is drawn, it also draws
 * the lines used to delineate the cells of a board.  Because some of
 * the lines are of differing thinkness, the lines drawn on each cell
 * depends on the cell's location within the board.  Furthermore, for
 * some cells, the place at which the glyph is drawn must be offset to
 * take into account of the differing line thinkness.  The tricky part
 * of this code is in the draw routine.
 *
 * When an editable cell is created, the non-zero digits, period, and
 * space bar keys change a cell.  A signal is emitted when a cell's
 * value changes via a key press.
 */

#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "config.h"
#include "gtksudoku.h"
#include "sudokuboard.h"
#include "sudokucell.h"

#define SUDOKU_CELL_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj), SUDOKU_CELL_TYPE, SudokuCellPrivate))

G_DEFINE_TYPE(SudokuCell, sudoku_cell, GTK_TYPE_DRAWING_AREA);

static void sudoku_cell_set_val(SudokuCell *cell, int val, int mode);
static int sudoku_cell_get_val(SudokuCell *cell);
static gboolean sudoku_cell_expose(GtkWidget *widget,
				   GdkEventExpose *event);
static void sudoku_cell_size_request(GtkWidget *widget,
				     GtkRequisition *requisition);
static gboolean sudoku_cell_focus_in(GtkWidget *widget,
				     GdkEventFocus *event);
static gboolean sudoku_cell_focus_out(GtkWidget *widget,
				      GdkEventFocus *event);
static gboolean sudoku_cell_button_press(GtkWidget *widget,
					 GdkEventButton *event);
static gboolean sudoku_cell_key_press(GtkWidget *widget,
				      GdkEventKey *event);

typedef struct _SudokuCellPrivate SudokuCellPrivate;

struct _SudokuCellPrivate
{
  /* The private instance variable val has a set of digits, and the
     mode is non-zero if a dot pattern is to be drawn when the cell
     has only one possible digit, otherwise a numeral is drawn. */
  int row, col, val, mode;
  gboolean editable;	     /* Can cell be changed by key presses? */
};

static void
sudoku_cell_class_init(SudokuCellClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  klass->set_val = sudoku_cell_set_val;
  klass->get_val = sudoku_cell_get_val;
  /* GtkWidget signals */
  widget_class->expose_event = sudoku_cell_expose;
  widget_class->size_request = sudoku_cell_size_request;
  widget_class->focus_in_event = sudoku_cell_focus_in;
  widget_class->focus_out_event = sudoku_cell_focus_out;
  widget_class->button_press_event = sudoku_cell_button_press;
  widget_class->key_press_event = sudoku_cell_key_press;

  g_type_class_add_private(obj_class, sizeof(SudokuCellPrivate));
}

static void
sudoku_cell_init(SudokuCell *cell)
{
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(cell);
  priv->val = ALL;
  priv->mode = 0;
  GtkWidget *widget = GTK_WIDGET(cell);	/* Set background */
  gtk_widget_ensure_style(widget); /* color to white */
  gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &widget->style->white);
}

/* Use the font metrics of the default font to determine the size of a
   cell.  Set the width and the height to two times the maximum
   vertical extent of the font. */
static void
sudoku_cell_size_request(GtkWidget *widget,
			 GtkRequisition *requisition)
{
  gtk_widget_ensure_style(widget);
  PangoLayout *layout = gtk_widget_create_pango_layout(widget, "0");
  pango_layout_set_font_description(layout, widget->style->font_desc);
  int width, height;
  pango_layout_get_pixel_size(layout, &width, &height);
  g_object_unref(layout);
  requisition->width = requisition->height = 2 * height;
}

static void
sudoku_cell_redraw_canvas(SudokuCell *cell)
{
  GtkWidget *widget = GTK_WIDGET(cell);
  if (!widget->window) return;

  GdkRegion *region = gdk_drawable_get_clip_region(widget->window);
  /* redraw the cairo canvas completely by exposing it */
  gdk_window_invalidate_region(widget->window, region, TRUE);
  gdk_window_process_updates(widget->window, TRUE);

  gdk_region_destroy(region);
}

static void
sudoku_cell_set_val(SudokuCell *cell, int val, int mode)
{
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(cell);
  val &= ALL;
  if (priv->val != val || priv->mode != mode) {
    priv->val = val;
    priv->mode = mode;
    sudoku_cell_redraw_canvas(cell);
  }
}

static int
sudoku_cell_get_val(SudokuCell *cell)
{
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(cell);
  return priv->val;
}

/* Change the background to gray when this widget has the focus. */

static const GdkColor gray = {0, 0xdcdc, 0xdada, 0xd5d5};

static gboolean
sudoku_cell_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
  gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &gray);
  sudoku_cell_redraw_canvas(SUDOKU_CELL(widget));
  return FALSE;
}

static gboolean
sudoku_cell_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &widget->style->white);
  sudoku_cell_redraw_canvas(SUDOKU_CELL(widget));
  return FALSE;
}

/* The mouse just grabs the focus. */

static gboolean
sudoku_cell_button_press(GtkWidget *widget, GdkEventButton *event)
{
  gtk_widget_grab_focus(widget);
  return FALSE;
}

/* The non-zero digits, period, and space bar change a cell when it is
   editable.  A signal is emitted when a cell's value changes. */

static gboolean
pressed_key(GtkWidget *widget, SudokuCellPrivate *priv, int digit)
{
  if (digit >= 1 && digit <= 9)
    digit = 1 << (digit - 1);
  else
    digit = ALL;
  int old = priv->val;
  sudoku_cell_set_val(SUDOKU_CELL(widget), digit, 0);
  if (digit != old)
    g_signal_emit_by_name(gtk_widget_get_parent(widget),
			  SUDOKU_BOARD_CHANGED_SIGNAL_NAME,
			  priv->row, priv->col,
			  priv->val, priv->mode);
  return FALSE;
}

static gboolean
sudoku_cell_key_press(GtkWidget *widget, GdkEventKey *event)
{
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(widget);
  if (priv->editable) {
    switch (event->keyval) {
    case GDK_KEY_1:
      return pressed_key(widget, priv, 1);
    case GDK_KEY_2:
      return pressed_key(widget, priv, 2);
    case GDK_KEY_3:
      return pressed_key(widget, priv, 3);
    case GDK_KEY_4:
      return pressed_key(widget, priv, 4);
    case GDK_KEY_5:
      return pressed_key(widget, priv, 5);
    case GDK_KEY_6:
      return pressed_key(widget, priv, 6);
    case GDK_KEY_7:
      return pressed_key(widget, priv, 7);
    case GDK_KEY_8:
      return pressed_key(widget, priv, 8);
    case GDK_KEY_9:
      return pressed_key(widget, priv, 9);
    case GDK_KEY_period:
    case GDK_KEY_space:
      return pressed_key(widget, priv, 0);
    }
  }
  return gtk_bindings_activate_event(GTK_OBJECT(widget), event);
}

/* Sudoku Cell drawing routines. */

#define DELTA 0.5

/* For text, set the scale relative to the size of the numeral
   zero. */
static void
scale_zero(cairo_t *cr, double cell_w, double cell_h)
{
  cairo_text_extents_t extends[1];
  cairo_text_extents(cr, "0", extends);
  double zero_h = extends->height;
  double sx = DELTA * cell_w / zero_h;
  double sy = DELTA * cell_h / zero_h;
  cairo_scale(cr, sx, sy);
}

/* Draw a UTF8 string centered on (0, 0). */

static void
draw_centered(cairo_t *cr, const char *utf8)
{
  cairo_text_extents_t extends[1];
  cairo_text_extents(cr, utf8, extends);
  double zero_w = extends->width;
  double zero_h = extends->height;
  cairo_move_to(cr, -(zero_w / 2 + extends->x_bearing),
		-(zero_h / 2 + extends->y_bearing));
  cairo_show_text(cr, utf8);
}

/* Sizes of lines are given relative to the size of the default line
   width. */

/* Relative thickness of the outside lines. */

#define OUTSIDE 4

/* Relative thickness of the inside lines. */

#define INSIDE 2

/* Relative distance to move text. */

#define OFFSET 1

/* Draw lines needed for each 3x3 cell.  You really have to draw
   pictures to understand this code.  Do it! */

static void
draw_square(GtkWidget *widget, cairo_t *cr)
{
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(widget);
  int row = priv->row % SIDES;
  int col = priv->col % SIDES;
  double cell_w = widget->allocation.width;
  double cell_h = widget->allocation.height;
  double line_w = cairo_get_line_width(cr);
  cairo_set_line_width(cr, INSIDE * line_w);
  switch (row) {
  case 0:
    switch (col) {
    case 0:
      cairo_move_to(cr, 0, cell_h);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr, cell_w, 0);
      cairo_stroke(cr);
      break;
    case 1:
      cairo_move_to(cr, 0, cell_h);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr, cell_w, 0);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_stroke(cr);
      break;
    case 2:
      cairo_move_to(cr, 0, 0);
      cairo_line_to(cr, cell_w, 0);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_stroke(cr);
      break;
    }
    break;
  case 1:
    switch (col) {
    case 0:
      cairo_move_to(cr, cell_w, 0);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr, 0, cell_h);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_stroke(cr);
      break;
    case 1:
      cairo_rectangle(cr, 0, 0, cell_w, cell_h);
      cairo_stroke(cr);
      break;
    case 2:
      cairo_move_to(cr, 0, 0);
      cairo_line_to(cr, cell_w, 0);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_line_to(cr, 0, cell_h);
      cairo_stroke(cr);
      break;
    }
    break;
  case 2:
    switch (col) {
    case 0:
      cairo_move_to(cr, 0, 0);
      cairo_line_to(cr, 0, cell_h);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_stroke(cr);
      break;
    case 1:
      cairo_move_to(cr, 0, 0);
      cairo_line_to(cr, 0, cell_h);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_line_to(cr, cell_w, 0);
      cairo_stroke(cr);
      break;
    case 2:
      cairo_move_to(cr, cell_w, 0);
      cairo_line_to(cr, cell_w, cell_h);
      cairo_line_to(cr, 0, cell_h);
      cairo_stroke(cr);
      break;
    }
    break;
  }
  cairo_set_line_width(cr, line_w);
}

/* Draw one cell.  This function draws the lines that handle the
   outside of the board, calls draw_square to handle the other lines,
   and then draws the appropriate glyph. */

static void
draw(GtkWidget *widget, cairo_t *cr)
{
  int d;
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(widget);
  double cell_w = widget->allocation.width;
  double cell_h = widget->allocation.height;
  double x = cell_w / 2;
  double y = cell_h / 2;
  double line_w = cairo_get_line_width(cr);
  switch (priv->row) {		/* This code handles the lines for */
  case 0:			/* the outer boarders of the board. */
    cairo_set_line_width(cr, OUTSIDE * line_w);	/* It also computes */
    cairo_move_to(cr, 0, 0);	/* the offset required to center */
    cairo_line_to(cr, cell_w, 0); /* content given the width of */
    cairo_stroke(cr);		/* the lines drawn in each cell. */
    cairo_set_line_width(cr, line_w);
    y += OFFSET * line_w;
    break;
  case 3:
  case 6:
    y += 0.5 * OFFSET * line_w;
    break;
  case 8:
    cairo_set_line_width(cr, OUTSIDE * line_w);
    cairo_move_to(cr, 0, cell_h);
    cairo_line_to(cr, cell_w, cell_h);
    cairo_stroke(cr);
    cairo_set_line_width(cr, line_w);
    y -= OFFSET * line_w;
    break;
  case 5:
  case 2:
    y -= 0.5 * OFFSET * line_w;
    break;
  }
  switch (priv->col) {		/* It's all just like the */
  case 0:			/* above case only rotated. */
    cairo_set_line_width(cr, OUTSIDE * line_w);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, 0, cell_h);
    cairo_stroke(cr);
    cairo_set_line_width(cr, line_w);
    x += OFFSET * line_w;
    break;
  case 3:
  case 6:
    x += 0.5 * OFFSET * line_w;
    break;
  case 8:
    cairo_set_line_width(cr, OUTSIDE * line_w);
    cairo_move_to(cr, cell_w, 0);
    cairo_line_to(cr, cell_w, cell_h);
    cairo_stroke(cr);
    cairo_set_line_width(cr, line_w);
    x -= OFFSET * line_w;
    break;
  case 5:
  case 2:
    x -= 0.5 * OFFSET * line_w;
    break;
  }
  draw_square(widget, cr);
  cairo_translate(cr, x, y);

  int val = priv->val;
  if (val == 0) {		/* Board is inconsistent! */
    cairo_set_source_rgb(cr, 1, 0, 0);
    scale_zero(cr, cell_w, cell_h);
    draw_centered(cr, "?");
    return;
  }
  else if (!priv->mode) {
    char buf[2];
    for (d = 0; d < DIGITS; d++)
      if (val == 1 << d) {
	buf[0] = '1' + d;	/* Draw numeral. */
	buf[1] = 0;
	scale_zero(cr, cell_w, cell_h);
	draw_centered(cr, buf);
	return;
      }
  }

  if (val == ALL)		/* If nothing has been eliminated */
    return;			/* draw a blank. */

  cairo_scale(cr, cell_w / 4, cell_h / 4);
  double pi = 8 * atan2(1, 1);  /* Otherwise draw a dot pattern. */
  for (d = 0; d < DIGITS; d++)
    if (val & (1 << d)) {
      x = d % SIDES - 1;
      y = d / SIDES - 1;
      cairo_arc(cr, x, y, 0.25, 0, pi);
      cairo_fill(cr);
    }
}

static gboolean
sudoku_cell_expose(GtkWidget *widget, GdkEventExpose *event)
{
  cairo_t *cr = gdk_cairo_create(widget->window);
  cairo_rectangle(cr, event->area.x, event->area.y,
		  event->area.width, event->area.height);
  cairo_clip(cr);
  draw(widget, cr);
  cairo_destroy(cr);
  return FALSE;
}

GtkWidget *
sudoku_cell_new(int row, int col, gboolean editable)
{
  SudokuCell *cell = g_object_new(SUDOKU_CELL_TYPE, NULL);
  SudokuCellPrivate *priv = SUDOKU_CELL_GET_PRIVATE(cell);
  priv->row = row;
  priv->col = col;
  priv->editable = editable;
  if (editable) {
    gtk_widget_add_events(GTK_WIDGET(cell),
			  GDK_KEY_PRESS_MASK |
			  GDK_BUTTON_PRESS_MASK |
			  GDK_ENTER_NOTIFY_MASK |
			  GDK_LEAVE_NOTIFY_MASK);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(cell), GTK_CAN_FOCUS);
  }
  return GTK_WIDGET(cell);
}
