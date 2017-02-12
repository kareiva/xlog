/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>

   This file is part of xlog.

   Xlog is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Xlog is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with xlog.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 * gui_tracedialog.c - creation and destruction of the trace dialog
 *
 * a dialog is created with a text widget. We restart hamlib with tracing
 * enabled and monitor stderr for changes. If there is a change the textwindow
 * is updated.
 */

#include <gtk/gtk.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <hamlib/rig.h>

#include "gui_tracedialog.h"
#include "gui_warningdialog.h"
#include "support.h"
#include "cfg.h"
#include "hamlib-utils.h"

typedef struct
{
	time_t modified;		/* Last Modified Time */				
	gchar *filename;		/* File Name */
	FILE *fd;				/* File Descriptor */
	unsigned long filesize; /* File Size */
} fileinfo;

gint saved_fd;
gint tracetimer = -1;
fileinfo finfo;

extern gchar *xlogdir;
extern preferencestype preferences;
extern GtkWidget *mainwindow;

static void
stop_tracing (void)
{
	gint hamlibresult;

	stop_hamlib ();
	g_source_remove (tracetimer);
	fclose (finfo.fd);
	dup2 (saved_fd, STDERR_FILENO);
	close (saved_fd);
#ifndef __NetBSD__
#ifndef G_OS_WIN32
	stderr = fdopen (STDERR_FILENO, "w");
#endif
#endif
	hamlibresult = start_hamlib (preferences.rigid, preferences.device,
		RIG_DEBUG_NONE, preferences.polltime);
}

static gboolean
on_tracedialog_delete_event (GtkWidget * widget, GdkEvent * event,
	 gpointer user_data)
{
	stop_tracing ();
	return FALSE;
}

/* check if tracefile has changed */
static int fileupdated(void) 
{
	struct stat s;
	gint status;

	if (finfo.filename) 
	{
		status = g_stat (finfo.filename, &s);
			if (finfo.modified != s.st_mtime) 
				return 1;
	}
	return -1;
}

static gint
updatetrace (gpointer data)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	GtkTextMark *mark;
	FILE *fd;
	gint numread = 0;
	gchar buf[1025];
 
	fflush(stderr);
	if (fileupdated ()) 
		{
			if ((fd = g_fopen(finfo.filename, "r"))) 
				{ 
					buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
					gtk_text_buffer_get_bounds (buffer, &start, &end);
					fseek (fd, finfo.filesize, SEEK_SET);
					while (!feof(fd))	
						{
							numread = fread(buf, 1, 1024, fd);
							gtk_text_buffer_insert (buffer, &end, buf, numread);
							mark = gtk_text_buffer_get_mark (buffer, "insert");
							gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(data),
								mark, 0.0, FALSE, 0.0, 1.0);
						}
					fclose (fd);
				}
		}
	return 1;
}

void
on_trace_hamlib_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *tracedialog, *tracelabel, *vbox,	*tracedialog_hseparator,
		*tracedialog_scrolledwindow, *tracedialog_textview, *close_button;
	gint hamlibresult, response;
	gchar *tracestr;
	
	if (preferences.hamlib == 0)
	{
		warningdialog ("xlog - trace hamlib", 
			_("Please enable hamlib from the Preferences -> Settings dialog"),
			"gtk-dialog-warning");
	}
	else
	{
	/* redirect stderr to a file and start the file monitor,
		 stop and start hamlib with tracing enabled */
		tracedialog = gtk_dialog_new_with_buttons (_("xlog - trace hamlib"),
			GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
		close_button = gtk_dialog_add_button (GTK_DIALOG (tracedialog),
			GTK_STOCK_CLOSE, GTK_RESPONSE_OK);
		gtk_widget_grab_focus (close_button);
        gtk_widget_set_size_request (tracedialog, 400, 300);
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER
			(GTK_DIALOG (tracedialog)->vbox), vbox);
		tracedialog_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
		gtk_box_pack_start
			(GTK_BOX (vbox), tracedialog_scrolledwindow, TRUE, TRUE, 0);
		gtk_scrolled_window_set_policy
			(GTK_SCROLLED_WINDOW (tracedialog_scrolledwindow),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		tracedialog_textview = gtk_text_view_new ();
		gtk_text_view_set_editable (GTK_TEXT_VIEW(tracedialog_textview), FALSE);
		gtk_text_view_set_cursor_visible
			(GTK_TEXT_VIEW(tracedialog_textview), FALSE);
		gtk_container_add
			(GTK_CONTAINER (tracedialog_scrolledwindow), tracedialog_textview);
		tracedialog_hseparator = gtk_hseparator_new ();
		gtk_box_pack_start
			(GTK_BOX (vbox), tracedialog_hseparator, FALSE, FALSE, 0);

		saved_fd = dup (STDERR_FILENO);
		finfo.filename = g_strdup_printf ( "%s/hamlib.out", xlogdir);
		tracestr = g_strdup_printf (_("Saving to %s"), finfo.filename);
		tracelabel = gtk_label_new (tracestr);
		gtk_box_pack_start
			(GTK_BOX (vbox), tracelabel, FALSE, FALSE, 0);
		g_free (tracestr);
		stop_hamlib();
		finfo.fd = freopen (finfo.filename, "w", stderr);
		hamlibresult = start_hamlib (preferences.rigid, preferences.device, 
			RIG_DEBUG_TRACE, preferences.polltime);
		tracetimer = g_timeout_add
			(1000, (GSourceFunc) updatetrace, tracedialog_textview);

		g_signal_connect ((gpointer) tracedialog, "delete_event",
			G_CALLBACK (on_tracedialog_delete_event), NULL);
		
		gtk_widget_show_all (tracedialog);
		response = gtk_dialog_run (GTK_DIALOG(tracedialog));
		stop_tracing ();
		gtk_widget_destroy (tracedialog);
	}
}
