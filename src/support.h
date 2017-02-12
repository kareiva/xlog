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

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
	g_object_set_data_full (G_OBJECT (component), name, \
		g_object_ref (widget), (GDestroyNotify) g_object_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
	g_object_set_data (G_OBJECT (component), name, widget)

#ifdef G_OS_WIN32
#	include <libintl.h>
#	undef _
#	define _(String) dgettext (PACKAGE, String)
#	ifdef gettext_noop
#		define N_(String) gettext_noop (String)
#	else
#		define N_(String) (String)
#	endif
#else
#ifdef ENABLE_NLS
#	include <libintl.h>
#	undef _
#	define _(String) dgettext (PACKAGE, String)
#	ifdef gettext_noop
#		define N_(String) gettext_noop (String)
#	else
#		define N_(String) (String)
#	endif
#else
#	define _(String) (String)
#	define N_(String) (String)
#endif
#endif


GtkWidget* lookup_widget(GtkWidget *widget, const gchar *widget_name);
void add_pixmap_directory(const gchar *directory);
GtkWidget* create_pixmap(GtkWidget *widget, const gchar *filename);
GdkPixbuf* create_pixbuf(const gchar *filename);
