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
 * remote.c - remote data support
 */

/*
 * Messages send to xlog should consist of fields, separated by the '\1' character.
 * Fields are started with a descriptor, followed by a colon (':'). Possible fields are:
 *
 ****************************************************************************
 * program:<name of program which sends the QSO information>
 * version:<version string for the message queue, must be '1' for now>
 * date:<date of QSO, preferable 'dd mmm yyyy'>
 * time:<start time of QSO, preferably in GMT >
 * endtime:<end time of QSO, preferably GMT (mandatory in some countries)>
 * call:<callsign of your contact (will be converted to uppercase)>
 * mhz:<frequency in MHz>
 * mode:<any one of CW,SSB,RTTY,PSK31,etc (will be converted to uppercase)>
 * tx:<report (RST) which you have send>
 * rx:<report (RST) which you have received>
 * awards:<awards string, see the manual>
 * name:<name of the operator you have contacted>
 * qth:<town of the operator you have contacted>
 * notes:<additional notes>
 * power:<power you have used (mandatory in some countries)>
 * locator:<QRA locator, as used in VHF QSO's>
 * free1: <information to put in freefield1>
 * free2: <information to put in freefield2>
 * ?: special command to use for dupechecking, used by eepkeyer, see:
   http://www.hamsoftware.org/, requires data to be added to the qsoframe
 ****************************************************************************
 *
 * See sendtoxlog.c, included with the sources for examples.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <string.h>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#include "remote.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "support.h"
#include "main.h"
#include "hamlib-utils.h"
#include "gui_warningdialog.h"

#ifndef G_OS_WIN32
/* Socket related includes */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

msgtype msgbuf;
remotetype remote;
gint server_sockfd;
extern programstatetype programstate;
extern preferencestype preferences;
extern glong msgid;
extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern gchar **qso;
extern gchar **bandsplit;
extern gchar **modesplit;
extern GList *logwindowlist;

/* find colon in a string and return the right part */
static gchar *
getargument (gchar * remotestring)
{
	gchar **split, *found = NULL;

	split = g_strsplit (remotestring, ":", 2);
	if (split[1])
		found = g_strdup (split[1]);
	g_strfreev (split);
	return (g_strdup (found));
}


static void
addtolog_or_qsoframe (gint type, gchar * entry)
{
	GtkWidget *bandoptionmenu, *modeoptionmenu,
		*bandentry, *modeentry, *endhbox, *namehbox, *qthhbox,
		*locatorhbox, *powerhbox, *unknown1hbox, *unknown2hbox,
		*dateentry, *gmtentry, *callentry, *endentry, *rstentry,
		*myrstentry, *powerentry, *nameentry, *qthentry,
		*locatorentry, *unknownentry1, *unknownentry2, *remtv,
		*remarksvbox, *awardshbox, *awardsentry;
	GtkTextBuffer *b;
	gchar *temp, **remoteinfo, *argument = NULL, *remarks, *label;
	gint i, j = 0, bandindex, modeindex, err = 0;
	logtype *logw;
	GtkTreeIter iter;
	GtkListStore *model;
	GtkTreePath *path;
	GString *digits = g_string_new ("");

	if (type == 88 && entry && (strlen (entry) > 0))
	{
		gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
		if (page == -1)
		{
			if (!programstate.warning_nologopen)
			{
				warningdialog (_("xlog - error"),
				_("No log open while receiving remote data"),
				"gtk-dialog-error");
				programstate.warning_nologopen = TRUE;
			}
			return;
		}
		logw = g_list_nth_data (logwindowlist, page);

		if (logw->readonly)
		{
			update_statusbar (_("Can not add QSO, log is read-only"));
			return;
		}

		for (i = 0; i < QSO_FIELDS; i++) qso[i] = g_strdup ("");
		remarks = g_strdup ("");

		bandentry = lookup_widget (mainwindow, "bandentry");
		bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
		modeentry = lookup_widget (mainwindow, "modeentry");
		modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
		endhbox = lookup_widget (mainwindow, "endhbox");
		namehbox = lookup_widget (mainwindow, "namehbox");
		qthhbox = lookup_widget (mainwindow, "qthhbox");
		locatorhbox = lookup_widget (mainwindow, "locatorhbox");
		powerhbox = lookup_widget (mainwindow, "powerhbox");
		unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
		unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
		awardshbox = lookup_widget (mainwindow, "awardshbox");

		entry = my_strreplace (entry, "\n", " ");
		entry = my_strreplace (entry, "\t", " ");
		remoteinfo = g_strsplit (entry, "\1", 0);
		gint dchk_only = 0;
		for (;;)
		{
			if (remoteinfo[j] == NULL)
			break;

			if (g_ascii_strncasecmp (remoteinfo[j], "version:", 8) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
					remote.version = atoi (argument);
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "program:", 8) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
					remote.program = g_strdup (argument);
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "date:", 5) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
						qso[DATE] = g_strndup (argument, 15);
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "time:", 5) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
					qso[GMT] = g_strndup (argument, 8);
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "call:", 5) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					g_ascii_strup (argument, -1);
					qso[CALL] = g_strndup (argument, 15);
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "mhz:", 4) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (g_ascii_strcasecmp (argument, "HAMLIB") == 0)
					{
						get_frequency ();
						digits = convert_frequency ();
						qso[BAND] = g_strdup (digits->str);
						g_string_free (digits, TRUE);
					}
					else
						qso[BAND] = g_strndup (argument, 15);
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "mode:", 5) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (g_ascii_strcasecmp (argument, "HAMLIB") == 0)
					{
						get_mode ();
						qso[MODE] = rigmode (programstate.rigmode);
					}
					else
					{
						g_ascii_strup (argument, -1);
						qso[MODE] = g_strndup (argument, 8);
					}
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "tx:", 3) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (g_ascii_strcasecmp (argument, "HAMLIB") == 0)
					{
						get_smeter ();
						qso[RST] = g_strdup (programstate.rigrst);
					}
					else
						qso[RST] = g_strndup (argument, 15);
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "rx:", 3) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
					qso[MYRST] = g_strndup (argument, 15);
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "endtime:", 8) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (endhbox))
						qso[GMTEND] = g_strndup (argument, 8);
				} /* don't bother adding endtime to remarks field */
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "awards:", 7) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (awardshbox))
						qso[AWARDS] = g_strndup (argument, 30);
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "name:", 5) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (namehbox))
						qso[NAME] = g_strndup (argument, 30);
					else
					{
						if (g_ascii_strcasecmp (remarks, "") == 0)
							remarks = g_strdup (argument);
						else
							remarks = g_strconcat (remarks, ", ", argument, NULL);
					}
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "qth:", 4) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (qthhbox))
						qso[QTH] = g_strndup (argument, 30);
					else
					{
						if (g_ascii_strcasecmp (remarks, "") == 0)
							remarks = g_strdup (argument);
						else
							remarks = g_strconcat (remarks, ", ", argument, NULL);
					}
				}
			}	

			else if (g_ascii_strncasecmp (remoteinfo[j], "notes:", 6) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (g_ascii_strcasecmp (remarks, "") == 0)
						remarks = g_strdup (argument);
					else
					remarks = g_strconcat (remarks, ", ", argument, NULL);
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "power:", 6) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (g_ascii_strcasecmp (argument, "HAMLIB") == 0)
					{
						get_powerlevel ();
						if (gtk_widget_get_visible (powerhbox))
							qso[POWER] = g_strdup_printf ("%d", programstate.rigpower / 1000);
					}
					else
					{
						if (gtk_widget_get_visible (powerhbox))
							qso[POWER] = g_strndup (argument, 8);
					} /* don't bother adding power to the remarks field */
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "locator:", 8) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (locatorhbox))
						qso[LOCATOR] = g_strndup (argument, 30);
					else
					{
						if (g_ascii_strcasecmp (remarks, "") == 0)
							remarks = g_strdup (argument);
						else
							remarks = g_strconcat (remarks, ", ", argument, NULL);
					}
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "free1:", 6) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{	
	 				if (gtk_widget_get_visible (unknown1hbox))
						qso[U1] = g_strndup (argument, 30);
					else
					{
						if (g_ascii_strcasecmp (remarks, "") == 0)
							remarks = g_strdup (argument);
						else
							remarks = g_strconcat (remarks, ", ", argument, NULL);
					}
				}
			}

			else if (g_ascii_strncasecmp (remoteinfo[j], "free2:", 6) == 0)
			{
				if ((argument = getargument (remoteinfo[j])))
				{
					if (gtk_widget_get_visible (unknown2hbox))
						qso[U2] = g_strndup (argument, 30);
					else
					{
						if (g_ascii_strcasecmp (remarks, "") == 0)
							remarks = g_strdup (argument);
						else
							remarks = g_strconcat (remarks, ", ", argument, NULL);
					}
				}
			}
			else if (g_ascii_strncasecmp (remoteinfo[j], "?:", 2) == 0)
			{
				dchk_only = 1;
			}
		j++;
		}
		g_strfreev (remoteinfo);
		g_free (argument);

		if (g_ascii_strcasecmp (remarks, "") != 0)
			qso[REMARKS] = g_strndup (remarks, 80);
		g_free (remarks);

			/* try to fill in empty fields */
		if (g_ascii_strcasecmp (qso[DATE], "") == 0)
			qso[DATE] = xloggetdate ();
		if (g_ascii_strcasecmp (qso[GMT], "") == 0)
			qso[GMT] = xloggettime ();
		if (g_ascii_strcasecmp (qso[BAND], "") == 0)
		{
			if (preferences.bandseditbox == 0)
			{
				bandindex = gtk_combo_box_get_active (GTK_COMBO_BOX(bandoptionmenu));
				qso[BAND] = lookup_band (bandindex);
			}
			else
				qso[BAND] = gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1);
		}
		if (g_ascii_strcasecmp (qso[MODE], "") == 0)
		{
			if (preferences.modeseditbox == 0)
			{
				modeindex = gtk_combo_box_get_active (GTK_COMBO_BOX(modeoptionmenu));
				qso[MODE] = lookup_mode (modeindex);
			}
			else
				qso[MODE] = gtk_editable_get_chars (GTK_EDITABLE (modeentry), 0, -1);
		}
		
		if (dchk_only == 1)
		{
			/* Only do a dupe check and exit */
			/* qso[CALL] holds callsign to check */
			if (strlen(qso[CALL]) > 0)
			{
				callentry = lookup_widget (mainwindow, "callentry");
				gtk_entry_set_text (GTK_ENTRY (callentry), qso[CALL]);
			}
			return;
		}
			
		if (preferences.remoteadding == 1 && logw)
		{
			qso[NR] = g_strdup_printf ("%d", ++logw->qsos);
	
			/* add the QSO */
			model = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview)));
			gtk_list_store_prepend (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				NR, qso[NR], DATE, qso[DATE], GMT, qso[GMT],
				GMTEND, qso[GMTEND], CALL, qso[CALL],
				BAND, qso[BAND], MODE, qso[MODE], RST, qso[RST],
				MYRST, qso[MYRST], QSLOUT, qso[QSLOUT],
				QSLIN, qso[QSLIN], AWARDS, qso[AWARDS],
				POWER, qso[POWER], NAME, qso[NAME],
				QTH, qso[QTH], LOCATOR, qso[LOCATOR],
				U1, qso[U1], U2, qso[U2], REMARKS, qso[REMARKS], -1);
	
			/* scroll there */
			path = gtk_tree_path_new_from_string ("0");
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(logw->treeview),
				path, NULL, TRUE, 1.0, 0.0);
			gtk_tree_path_free (path);
			
			
			
			


      /* save with every log change */
			if (preferences.saving == 2)
			{
				savelog (logw, logw->filename, TYPE_FLOG, 1, logw->qsos);
				logw->logchanged = FALSE;
				temp = g_strdup_printf (_("QSO %s added to %s log, log saved"), qso[NR], logw->logname);
			}
			else
			{ /* autosave */
				temp = g_strdup_printf (_("Remote data received from %s (#%d), QSO %s added"), remote.program, ++remote.nr, qso[NR]);
				logw->logchanged = TRUE;
				label = g_strdup_printf ("<b>%s*</b>", logw->logname);
				gtk_label_set_markup (GTK_LABEL (logw->label), label);
				g_free (label);
			}
			update_statusbar (temp);
		}
		else
		{
			if (g_ascii_strcasecmp (qso[DATE], "") == 0)
				qso[DATE] = xloggetdate ();
			if (g_ascii_strcasecmp (qso[GMT], "") == 0)
				qso[GMT] = xloggettime ();
			dateentry = lookup_widget (mainwindow, "dateentry");
			if (strlen(qso[DATE]) > 0)
				gtk_entry_set_text (GTK_ENTRY (dateentry), qso[DATE]);
			gmtentry = lookup_widget (mainwindow, "gmtentry");
			if (strlen(qso[GMT]) > 0)
				gtk_entry_set_text (GTK_ENTRY (gmtentry), qso[GMT]);
			if (gtk_widget_get_visible (endhbox))
			{
				endentry = lookup_widget (mainwindow, "endentry");
				if (strlen(qso[GMTEND]) > 0)
					gtk_entry_set_text (GTK_ENTRY (endentry), qso[GMTEND]);
			}
			callentry = lookup_widget (mainwindow, "callentry");
			if (strlen(qso[CALL]) > 0)
				gtk_entry_set_text (GTK_ENTRY (callentry), qso[CALL]);
			if (preferences.modeseditbox == 1)
				if (strlen(qso[MODE]) > 0)
					gtk_entry_set_text (GTK_ENTRY (modeentry), qso[MODE]);
			if (preferences.bandseditbox == 1)
				if (strlen(qso[BAND]) > 0)
					gtk_entry_set_text (GTK_ENTRY (bandentry), qso[BAND]);
			rstentry = lookup_widget (mainwindow, "rstentry");
			if (strlen(qso[RST]) > 0)
				gtk_entry_set_text (GTK_ENTRY (rstentry), qso[RST]);
			myrstentry = lookup_widget (mainwindow, "myrstentry");
			if (strlen(qso[MYRST]) > 0)
				gtk_entry_set_text (GTK_ENTRY (myrstentry), qso[MYRST]);
			if (gtk_widget_get_visible (awardshbox))
			{
				awardsentry = lookup_widget (mainwindow, "awardsentry");
				if (strlen(qso[AWARDS]) > 0)
					gtk_entry_set_text (GTK_ENTRY (awardsentry), qso[AWARDS]);
			}
			if (gtk_widget_get_visible (powerhbox))
			{
				powerentry = lookup_widget (mainwindow, "powerentry");
				if (strlen(qso[POWER]) > 0)
					gtk_entry_set_text (GTK_ENTRY (powerentry), qso[POWER]);
			}
			if (gtk_widget_get_visible (namehbox))
			{
				nameentry = lookup_widget (mainwindow, "nameentry");
				if (strlen(qso[NAME]) > 0)
					gtk_entry_set_text (GTK_ENTRY (nameentry), qso[NAME]);
			}
			if (gtk_widget_get_visible (qthhbox))
			{
				qthentry = lookup_widget (mainwindow, "qthentry");
				if (strlen(qso[QTH]) > 0)
					gtk_entry_set_text (GTK_ENTRY (qthentry), qso[QTH]);
			}
			if (gtk_widget_get_visible (locatorhbox))
			{
				locatorentry = lookup_widget (mainwindow, "locatorentry");
				if (strlen(qso[LOCATOR]) > 0)
					gtk_entry_set_text (GTK_ENTRY (locatorentry), qso[LOCATOR]);
			}
			if (gtk_widget_get_visible (unknown1hbox))
			{
				unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
				if (strlen(qso[U1]) > 0)
					gtk_entry_set_text (GTK_ENTRY (unknownentry1), qso[U1]);
			}
			if (gtk_widget_get_visible (unknown2hbox))
			{
				unknownentry2 = lookup_widget (mainwindow, "unknownentry2");
				if (strlen(qso[U2]) > 0)
					gtk_entry_set_text (GTK_ENTRY (unknownentry2), qso[U2]);
			}
			remarksvbox = lookup_widget (mainwindow, "remarksvbox");
			if (gtk_widget_get_visible (remarksvbox))
			{
				remtv = lookup_widget (mainwindow, "remtv");
				b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
				if (strlen(qso[REMARKS]) > 0)
					gtk_text_buffer_set_text (b, qso[REMARKS], -1);
			}
			temp = g_strdup_printf (_("Remote data received from %s (#%d)"), remote.program, ++remote.nr);
		}
		if (err == 1)
			update_statusbar (_("Warning: No hamlib support for remote data"));
		else
			update_statusbar (temp);
		g_free (temp);

		remote.program = g_strdup ("unknown");
		remote.version = 0;
	}
}

gint
remote_entry (void)
{
	ssize_t status = -1;
#if HAVE_SYS_IPC_H
	glong msgtyp = 0;

	status = msgrcv (msgid, (void *) &msgbuf, 1024, msgtyp,
			 MSG_NOERROR | IPC_NOWAIT);
#endif
	if (status != -1)
		addtolog_or_qsoframe (msgbuf.mtype, msgbuf.mtext);
	return 1;
}

#ifndef G_OS_WIN32
static gboolean 
tcp_client_activity (GIOChannel *source, GIOCondition cond, gpointer data)
{
	gchar buf[1024];
	gsize num_read = 0;
	GIOStatus res;

	if (cond == G_IO_IN)
	{
		res = g_io_channel_read_chars (source, buf, sizeof (buf), &num_read, NULL);
		buf[num_read] = '\0';
		addtolog_or_qsoframe (88, buf);
	}
	close (g_io_channel_unix_get_fd (source));
	return FALSE;
}
#endif

#ifndef G_OS_WIN32
gboolean
socket_entry (GIOChannel * channel, GIOCondition cond, gpointer data)
{
	gint new; /* new socket descriptor */
	socklen_t client;
	GIOChannel  *new_channel;
	struct sockaddr_in client_addr;

	if (cond == G_IO_IN)
	{
		if ((new = accept (g_io_channel_unix_get_fd (channel),
			(struct sockaddr *)&client_addr, &client)) < 0)
		{
			update_statusbar (_("Unable to accept new connection"));
			return FALSE;
		}
		new_channel = g_io_channel_unix_new (new);
		g_io_add_watch (new_channel, G_IO_IN, tcp_client_activity, NULL);
	}
	return TRUE;
}
#endif

/* Socket Server AF_INET Setup */
#ifndef G_OS_WIN32
gint
remote_socket_setup (void)
{
	gint server_len, result, tmp = 1;
	struct sockaddr_in server_address;

	server_sockfd = socket (AF_INET, SOCK_STREAM, 0);
	result = setsockopt (server_sockfd, SOL_SOCKET, SO_REUSEADDR, (gchar *)&tmp, sizeof (tmp));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons (7311);
	server_len = sizeof (server_address);
	if (result == 0)
		result = bind (server_sockfd, (struct sockaddr *)&server_address, server_len);
	if (result == 0)
		result = listen (server_sockfd, 5);
	return result;
}
#endif
