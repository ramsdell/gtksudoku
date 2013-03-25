/*
 * Displays text in dialog window.  Word wrap is enabled.
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

#include <gtk/gtk.h>
#include "config.h"
#include "gtksudoku.h"
#include "showtext.h"

/* A dialog window based on the one that displays a license in the
   GtkAboutDialog widget. */

void
show_text(GtkWidget *window, const char *text)
{
  GtkWidget *dialog, *content_area, *view, *sw;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  dialog = gtk_dialog_new_with_buttons(PACKAGE_NAME " Help",
				       GTK_WINDOW(window),
				       flags,
				       GTK_STOCK_CLOSE,
				       GTK_RESPONSE_CANCEL,
				       NULL);
  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
				      GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				 GTK_POLICY_NEVER,
				 GTK_POLICY_AUTOMATIC);
  gtk_container_add_with_properties(GTK_CONTAINER(content_area), sw,
				    "expand", TRUE,
				    "fill", TRUE,
				    NULL);

  view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);

  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)),
			   text, -1);

  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);

  /* Use font metrics to set the size of the text view. */

  gtk_widget_ensure_style(view);
  PangoContext *context = gtk_widget_get_pango_context(view);
  PangoFontMetrics *metrics;
  metrics = pango_context_get_metrics(context,
				      gtk_widget_get_style(view)->font_desc,
				      pango_context_get_language(context));
  gint char_width = pango_font_metrics_get_approximate_char_width(metrics);
  gint ascent = pango_font_metrics_get_ascent(metrics);
  gint descent = pango_font_metrics_get_descent(metrics);
  gint height = PANGO_PIXELS(ascent + descent);
  gint width = PANGO_PIXELS(char_width);

  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 2 * width);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 2 * width);
  gtk_widget_set_size_request(view, 70 * width, 15 * height);

  gtk_container_add(GTK_CONTAINER(sw), view);

  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_destroy(dialog);
}
