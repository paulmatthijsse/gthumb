/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  GThumb
 *
 *  Copyright (C) 2010 Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <gthumb.h>
#include "dlg-convert-format.h"
#include "preferences.h"


#define GET_WIDGET(name) _gtk_builder_get_widget (data->builder, (name))
#define DEFAULT_MIME_TYPE "image/jpeg"


enum {
	MIME_TYPE_COLUMN_ICON = 0,
	MIME_TYPE_COLUMN_TYPE,
	MIME_TYPE_COLUMN_DESCRIPTION
};


typedef struct {
	GthBrowser *browser;
	GList      *file_list;
	GtkBuilder *builder;
	GtkWidget  *dialog;
	gboolean    use_destination;
} DialogData;


static void
dialog_destroy_cb (GtkWidget  *widget,
		   DialogData *data)
{
	gth_browser_set_dialog (data->browser, "convert_format", NULL);

	g_object_unref (data->builder);
	_g_object_list_unref (data->file_list);
	g_free (data);
}


static void
help_button_clicked_cb (GtkWidget  *widget,
			DialogData *data)
{
	show_help_dialog (GTK_WINDOW (data->dialog), "convert-format");
}


static void
convert_step (GthPixbufTask *pixbuf_task)
{
	pixbuf_task->dest = gdk_pixbuf_copy (pixbuf_task->src);
}


static void
ok_button_clicked_cb (GtkWidget  *widget,
		      DialogData *data)
{
	GtkTreeIter  iter;
	char        *mime_type;
	GthTask     *convert_task;
	GthTask     *list_task;

	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (GET_WIDGET ("mime_type_combobox")), &iter);
	gtk_tree_model_get (GTK_TREE_MODEL (GET_WIDGET ("mime_type_liststore")), &iter,
			    MIME_TYPE_COLUMN_TYPE, &mime_type,
			    -1);
	eel_gconf_set_string (PREF_CONVERT_FORMAT_MIME_TYPE, mime_type);

	convert_task = gth_pixbuf_task_new (_("Converting images"),
					    TRUE,
					    NULL,
					    convert_step,
					    NULL,
					    NULL,
					    NULL);
	list_task = gth_pixbuf_list_task_new (data->browser,
					      data->file_list,
					      GTH_PIXBUF_TASK (convert_task));
	gth_pixbuf_list_task_set_overwrite_mode (GTH_PIXBUF_LIST_TASK (list_task), GTH_OVERWRITE_ASK);
	gth_pixbuf_list_task_set_output_mime_type (GTH_PIXBUF_LIST_TASK (list_task), mime_type);
	if (data->use_destination) {
		GFile *destination;

		destination = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (GET_WIDGET ("destination_filechooserbutton")));
		gth_pixbuf_list_task_set_destination (GTH_PIXBUF_LIST_TASK (list_task), destination);

		g_object_unref (destination);
	}
	gth_browser_exec_task (data->browser, list_task, FALSE);

	g_object_unref (list_task);
	g_object_unref (convert_task);
	g_free (mime_type);
	gtk_widget_destroy (data->dialog);
}


void
dlg_convert_format (GthBrowser *browser,
		    GList      *file_list)
{
	DialogData *data;
	GArray     *savers;

	if (gth_browser_get_dialog (browser, "convert_format") != NULL) {
		gtk_window_present (GTK_WINDOW (gth_browser_get_dialog (browser, "convert_format")));
		return;
	}

	data = g_new0 (DialogData, 1);
	data->browser = browser;
	data->builder = _gtk_builder_new_from_file ("convert-format.ui", "convert_format");
	data->file_list = gth_file_data_list_dup (file_list);
	data->use_destination = GTH_IS_FILE_SOURCE_VFS (gth_browser_get_location_source (browser));

	/* Get the widgets. */

	data->dialog = _gtk_builder_get_widget (data->builder, "convert_format_dialog");
	gth_browser_set_dialog (browser, "convert_format", data->dialog);
	g_object_set_data (G_OBJECT (data->dialog), "dialog_data", data);

	/* Set widgets data. */

	savers = gth_main_get_type_set ("pixbuf-saver");
	if (savers != NULL) {
		char         *default_mime_type;
		GthIconCache *icon_cache;
		GtkListStore *list_store;
		int           i;

		default_mime_type = eel_gconf_get_string (PREF_CONVERT_FORMAT_MIME_TYPE, DEFAULT_MIME_TYPE);
		icon_cache = gth_icon_cache_new_for_widget (data->dialog, GTK_ICON_SIZE_MENU);
		list_store = (GtkListStore *) GET_WIDGET ("mime_type_liststore");
		for (i = 0; i < savers->len; i++) {
			GType           saver_type;
			GthPixbufSaver *saver;
			const char     *mime_type;
			GdkPixbuf      *pixbuf;
			GtkTreeIter     iter;

			saver_type = g_array_index (savers, GType, i);
			saver = g_object_new (saver_type, NULL);
			mime_type = gth_pixbuf_saver_get_mime_type (saver);
			pixbuf = gth_icon_cache_get_pixbuf (icon_cache, g_content_type_get_icon (mime_type));
			gtk_list_store_append (list_store, &iter);
			gtk_list_store_set (list_store, &iter,
					    MIME_TYPE_COLUMN_ICON, pixbuf,
					    MIME_TYPE_COLUMN_TYPE, mime_type,
					    MIME_TYPE_COLUMN_DESCRIPTION, g_content_type_get_description (mime_type),
					    -1);

			if (strcmp (default_mime_type, mime_type) == 0)
				gtk_combo_box_set_active_iter (GTK_COMBO_BOX (GET_WIDGET ("mime_type_combobox")), &iter);

			g_object_unref (pixbuf);
			g_object_unref (saver);
		}

		gth_icon_cache_free (icon_cache);
		g_free (default_mime_type);
	}

	if (data->use_destination) {
		gtk_file_chooser_set_file (GTK_FILE_CHOOSER (GET_WIDGET ("destination_filechooserbutton")), gth_browser_get_location (browser), NULL);
		gtk_widget_show (GET_WIDGET ("saving_box"));
	}
	else
		gtk_widget_hide (GET_WIDGET ("saving_box"));

	/* Set the signals handlers. */

	g_signal_connect (G_OBJECT (data->dialog),
			  "destroy",
			  G_CALLBACK (dialog_destroy_cb),
			  data);
	g_signal_connect (GET_WIDGET ("ok_button"),
			  "clicked",
			  G_CALLBACK (ok_button_clicked_cb),
			  data);
        g_signal_connect (GET_WIDGET ("help_button"),
                          "clicked",
                          G_CALLBACK (help_button_clicked_cb),
                          data);
	g_signal_connect_swapped (GET_WIDGET ("cancel_button"),
				  "clicked",
				  G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (data->dialog));

	/* Run dialog. */

	gtk_window_set_transient_for (GTK_WINDOW (data->dialog), GTK_WINDOW (browser));
	gtk_window_set_modal (GTK_WINDOW (data->dialog), FALSE);
	gtk_widget_show (data->dialog);
}