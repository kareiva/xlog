/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2016 Andy Stewart <kb1oiq@arrl.net>
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

/* gui_aboutdialog.c - creation of the about dialog */

#include <gtk/gtk.h>

#include "gui_aboutdialog.h"
#include "gui_utils.h"
#include "support.h"
#include "cfg.h"
#include "dxcc.h"
#include "utils.h"

extern preferencestype preferences;
extern GtkWidget *mainwindow;

static void
handle_url (GtkAboutDialog *about, const char *link, gpointer data)
{
	open_url (link);
}

static void
handle_email (GtkAboutDialog *about, const char *link, gpointer data)
{
	gchar *command[] = {"xdg-email", NULL, NULL};

	command[1] = g_strdup_printf ("mailto:%s", link);
	g_spawn_async
		(NULL, command, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
	g_free (command[1]);
}

void
on_menu_about_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	const gchar *authors[] = { "Andy Stewart KB1OIQ <kb1oiq@arrl.net>",
		"Joop Stakenborg PG4I <pg4i@amsat.org>",
		"Stephane Fillod F8CFE <f8cfe@free.fr>", NULL };
	const gchar *documenters[] = { "Andy Stewart KB1OIQ <kb1oiq@arrl.net>",
				       "Joop Stakenborg PG4I <pg4i@amsat.org>",
				       "Chris Story K6RWJ <ke6rwj@gmail.com>", 
				       NULL };

	GdkPixbuf *pixbuf = create_pixbuf ("xlog.png");

	gtk_about_dialog_set_url_hook (handle_url, NULL, NULL);
	gtk_about_dialog_set_email_hook (handle_email, NULL, NULL);

	gint ctyversion = readctyversion ();
	gchar *str;
	if (ctyversion > 0)
		str = g_strdup_printf ("%s\n%s %d", 
		_("logbook program for amateur radio operators"),
		_("using cty.dat version"), ctyversion);
	else
		str = g_strdup_printf ("%s", 
		_("logbook program for amateur radio operators"));

	gtk_show_about_dialog (GTK_WINDOW(mainwindow),
		"authors", authors,
		"documenters", documenters,
		"comments", str,
		"copyright", "Copyright (C) 2012 - 2016 Andy Stewart <kb1oiq@arrl.net>\n 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>",
		"license",
"Xlog is free software: you can redistribute it and/or modify "
"it under the terms of the GNU General Public License as published by "
"the Free Software Foundation, either version 3 of the License, or "
"(at your option) any later version."
"\n\n"
"Xlog is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of "
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
"GNU General Public License for more details."
"\n\n"
"You should have received a copy of the GNU General Public License "
"along with xlog.  If not, see <http://www.gnu.org/licenses>."
"\n\n"
"Full text of the GPL can be seen in the xlog handbook under "
"the chapter \"License\".",
		"wrap-license", TRUE,
		"website", "http://www.nongnu.org/xlog",
		"website-label", "Xlog Homepage",
//		"logo-icon-name", "xlog",
		"logo", pixbuf,
		"translator-credits",
"Czech:	Jan Straka OK2JTB <bach@email.cz>\n"
"		Martin Kratoska OK1RR <martin@ok1rr.com>\n"
"Dutch:	Joop Stakenborg PG4I <pg4i@amsat.org>\n"
"French:	Jean-Luc Coulon F5IBH <jean-luc.coulon@wanadoo.fr>\n"
"German:	Martin Hautsch DL5MRH <moadin@hautsch.net>\n"
"Italian:	Carlo Dormeletti I-3929 <carlo.dormeletti@email.it>\n"
"		Luca Monducci <luca.mo@tiscali.it>\n"
"Polish:	Boguslaw Ciastek SQ5TB <boguslaw@ciastek.eu>\n"
"Portuguese:	David Quental CT1DRB <ct1drb@gmail.com>\n"
"Spanish:	Jaime Robles EA4TV <jaime@robles.es>\n"
"Slovak:	Michal Karas OM4AA <zoliqe@gmail.com>\n"
"Swedish:	Peter Landgren SM4ABE <peter.talken@telia.com>\n",
		"version", VERSION,
		NULL);
	g_free (str);
}
