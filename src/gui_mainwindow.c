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

#include <sys/types.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks_mainwindow_menu.h"
#include "callbacks_mainwindow_toolbar.h"
#include "callbacks_mainwindow.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "gui_mainwindow.h"
#include "gui_aboutdialog.h"
#include "gui_b4window.h"
#include "gui_closedialog.h"
#include "gui_defaultsdialog.h"
#include "gui_dialogsdialog.h"
#include "gui_dupecheckdialog.h"
#include "gui_dxcccheck.h"
#include "gui_dxcclist.h"
#include "gui_openlogdialog.h"
#include "gui_helpdialog.h"
#include "gui_keys.h"
#include "gui_logeditordialog.h"
#include "gui_newlogdialog.h"
#include "gui_gtkprintdialog.h"
#include "gui_preferencesdialog.h"
#include "gui_saveasdialog.h"
#include "gui_scorewindow.h"
#include "gui_searchdialog.h"
#include "gui_netkeyer.h"
#include "gui_mergedialog.h"
#include "gui_importdialog.h"
#include "gui_exportdialog.h"
#include "gui_tracedialog.h"
#include "gui_awards_dxcc.h"
#include "gui_awards_wac.h"
#include "gui_awards_was.h"
#include "gui_awards_waz.h"
#include "gui_awards_iota.h"
#include "gui_awards_locator.h"
#include "gui_countrymap.h"
#include "support.h"
#include "cfg.h"
#include "gui_tracedialog.h"

GtkUIManager *ui_manager;
extern preferencestype preferences;

#ifndef G_OS_WIN32
static struct {
	gchar *filename;
	gchar *stock_id;
} stock_icons[] = {
	{ DATADIR"/pixmaps/xlog/cwdaemon.png", "xlog_stock_cwdaemon" },
	{ DATADIR"/pixmaps/xlog/jigsaw.png", "xlog_stock_merge" },
	{ DATADIR"/pixmaps/xlog/countrymap.png", "xlog_stock_map" },
};

static gint n_stock_icons = G_N_ELEMENTS (stock_icons);

static void
register_my_stock_icons (void)
{
	GtkIconFactory *icon_factory;
	GtkIconSet *icon_set;
	GtkIconSource *icon_source;
	gint i;

	icon_factory = gtk_icon_factory_new ();

	for (i = 0; i < n_stock_icons; i++)
	{
		icon_set = gtk_icon_set_new ();
		icon_source = gtk_icon_source_new ();
		gtk_icon_source_set_filename (icon_source, stock_icons[i].filename);
		gtk_icon_set_add_source (icon_set, icon_source);
		gtk_icon_source_free (icon_source);
		gtk_icon_factory_add (icon_factory, stock_icons[i].stock_id, icon_set);
		gtk_icon_set_unref (icon_set);
	}
	gtk_icon_factory_add_default (icon_factory);
	g_object_unref (icon_factory);
}
#endif

GtkWidget*
create_mainwindow (void)
{
	GtkWidget *window, *mainvbox, *menubar;
	GdkPixbuf *window_icon_pixbuf;
	GtkWidget *hpaned, *qsomainvbox, *handlebox, *toolbar, *qsoscrolledwindow,
		*qsoviewport, *qsoframesvbox;
	GtkWidget *qsoframe, *qsofieldsvbox,
		*datehbox, *datebutton, *dateentry,
		*gmthbox, *gmtbutton, *gmtentry,
		*endhbox, *endbutton, *endentry,
		*callhbox, *calllabel, *callentry,
		*bandhbox, *mhzhbox, *mhzbutton, *mhzlabel, *bandhbox2, *bandentry,
		*modehbox, *modehbox1, *modebutton, *modelabel,	*modehbox2, *modeentry,
		*hrsthbox, *hbox1, *rstbutton, *rstlabel, *rstentry,
		*mrsthbox, *myrstlabel, *myrstentry,
		*awardshbox, *awardslabel, *awardsentry,
		*qslhbox, *outcheckbutton, *incheckbutton,
		*powerhbox, *hbox2, *powerbutton, *powerlabel, *powerentry,
		*namehbox, *namelabel, *nameentry,
		*qthhbox, *qthlabel, *qthentry,
		*locatorhbox, *locatorlabel, *locatorentry,
		*unknown1hbox, *unknownlabel1, *unknownentry1,
		*unknown2hbox, *unknownlabel2, *unknownentry2,
		*remarksvbox, *rembox1, *rembox2, *remarkslabel, *remscr, *remtv;
	GtkTextBuffer *b;
	GtkWidget *label1;
	GtkWidget *statushbox, *statusbar, *frequencyhandlebox, *frequencyhbox,
		*frequencypixmap, *frequencylabel, *smeterhandlebox, *smeterhbox,
		*smeterpixmap, *smeterdrawingarea, *clockhandlebox, *clockhbox,
		*clockpixmap, *clocklabel;
	GtkAccelGroup *accel_group;
	GtkActionGroup *action_group;
	GError *error;
	gint merge_id;
	gchar *temp;

/* Normal items */
static GtkActionEntry entries[] =
{
  { "LogMenu", NULL, N_("Log") },
  { "EditMenu", NULL, N_("Edit") },
  { "OptionMenu", NULL, N_("Options") },
  { "ToolsMenu", NULL, N_("Tools") },
  { "TabsMenu", NULL, N_("Page") },
  { "SettingsMenu", NULL, N_("Settings") },
  { "HelpMenu", NULL, N_("Help") },

  { "New", GTK_STOCK_NEW, N_("New..."), "<control>N", "New log", G_CALLBACK(on_menu_new_activate) },
  { "Open", GTK_STOCK_OPEN, N_("Open"), "<control>O", "Open a log", G_CALLBACK(on_menu_open_activate) },
  { "Print", GTK_STOCK_PRINT, N_("Print"), "<control>P", "Print log", G_CALLBACK(on_menu_print_activate) },
  { "PageSetup", GTK_STOCK_PRINT_PREVIEW, N_("Page Setup"), "<control><shift>P", "Page setup for printing", G_CALLBACK(on_menu_pagesetup_activate) },
  { "Save", GTK_STOCK_SAVE, N_("Save"), "<control>S", "Save log", G_CALLBACK(on_menu_save_activate) },
  { "Save As", GTK_STOCK_SAVE_AS, N_("Save As..."), "<control><shift>S", "Save log as", G_CALLBACK(on_menu_saveas_activate) },
  { "Close", GTK_STOCK_CLOSE, N_("Close"), "<control><shift>W", "Close log", G_CALLBACK(on_menu_close_activate) },
  { "Export", GTK_STOCK_SAVE_AS, N_("Export..."), "<control>E", "Export a log", G_CALLBACK(on_menu_export_activate) },
  { "Import", GTK_STOCK_CONVERT, N_("Import..."), "<control>I", "Import a log", G_CALLBACK(on_menu_import_activate) },
  { "Merge", "xlog_stock_merge", N_("Merge"), "<control>M", "Merge logs", G_CALLBACK(on_menu_merge_activate) },
  { "Quit", GTK_STOCK_QUIT, N_("Quit"), "<control>Q", "Quit program", G_CALLBACK(on_menu_exit_activate) },

  { "Write", GTK_STOCK_ADD, N_("Write"), "<control>W",
		N_("Write a QSO to the log [Ctrl-W]"), G_CALLBACK(on_abutton_clicked) },
  { "Clear All", GTK_STOCK_CLEAR, N_("Clear All"), "<control>Y",
		"Clear all fields", G_CALLBACK(on_clearframe_activate) },
  { "Click All", GTK_STOCK_APPLY, N_("Click All"), "<control>K",
		"Click all buttons", G_CALLBACK(on_clickall_activate) },
  { "Write and Click All", GTK_STOCK_ADD, N_("Write and Click All"), "<control>space",
		"Write a QSO to the log and Click All", G_CALLBACK(on_addclickall_activate) },
  { "Delete", GTK_STOCK_REMOVE, N_("Delete"), "<control>D",
		N_("Delete a selected log entry [Ctrl-D]"), G_CALLBACK(on_dbutton_clicked) },
  { "Find", GTK_STOCK_FIND, N_("Find"), "<control>F",
		"Find callsign", G_CALLBACK(on_menu_search_activate) },
  { "Update", GTK_STOCK_REFRESH, N_("Update"), "<control>U",
		N_("Update a modified log entry [Ctrl-U]"), G_CALLBACK(on_ubutton_clicked) },

  { "CountryMap", "xlog_stock_map", N_("Country Map"), "<control><shift>M", "Show/hide the country map", G_CALLBACK(on_countrymap_activate) },
#ifndef G_OS_WIN32
  { "Keyer", "xlog_stock_cwdaemon", N_("Keyer"), "<control>R", "Activate keyer interface", G_CALLBACK(on_keyer_activate) },
#endif

  { "DXCC Check", GTK_STOCK_SPELL_CHECK, N_("Find unknown countries..."), "<control><shift>F", NULL, G_CALLBACK(on_menu_dxcccheck_activate) },
  { "Dupe Check", GTK_STOCK_COPY, N_("Dupe Check..."), "<control><shift>C", "Check the log(s) for dupes", G_CALLBACK(on_menu_dupecheck_activate) },
  { "Log Editor", GTK_STOCK_PROPERTIES, N_("Log Editor"), "<control><shift>E", "Format fields in the log", G_CALLBACK(on_menu_logeditor_activate) },
  { "Trace Hamlib", GTK_STOCK_EXECUTE, N_("Trace Hamlib"), "<control><shift>T", "Show hamlib debugging output", G_CALLBACK(on_trace_hamlib_activate) },
  { "Sort", GTK_STOCK_SORT_ASCENDING, N_("Sort by Date"), "<control><shift>O", "Sort log by date", G_CALLBACK(on_sort_log_activate) },
  { "Awards", GTK_STOCK_INDEX, N_("Awards"), "", "Overview of Awards", NULL },
  { "DXCC", GTK_STOCK_INFO, "DXCC", "<control>1", "DXCC worked/confirmed", G_CALLBACK(on_awards_dxcc_activate) },
  { "WAC", GTK_STOCK_INFO, "WAC", "<control>2", "WAC worked/confirmed", G_CALLBACK(on_awards_wac_activate) },
  { "WAS", GTK_STOCK_INFO, "WAS", "<control>3", "WAS worked/confirmed", G_CALLBACK(on_awards_was_activate) },
  { "WAZ", GTK_STOCK_INFO, "WAZ", "<control>4", "WAZ worked/confirmed", G_CALLBACK(on_awards_waz_activate) },
  { "IOTA", GTK_STOCK_INFO, "IOTA", "<control>5", "IOTA worked/confirmed", G_CALLBACK(on_awards_iota_activate) },
  { "Locator", GTK_STOCK_INFO, N_("Locator List"), "<control>6", "Locator worked/confirmed", G_CALLBACK(on_awards_locator_activate) },
  { "LocatorMap", GTK_STOCK_INFO, N_("Locator Map"), "<control>7", "Locator worked/confirmed map", G_CALLBACK(on_awards_locatormap_activate) },

  { "Defaults", GTK_STOCK_INDEX, N_("Defaults"), "<control>L", "Default values to use for logging", G_CALLBACK(on_menu_defaults_activate) },
  { "Dialogs", GTK_STOCK_JUSTIFY_FILL, N_("Dialogs and Windows"), "<control>B", "Settings for dialogs and windows used", G_CALLBACK(on_menu_dialogs_activate) },
  { "Preferences", GTK_STOCK_PREFERENCES, N_("Preferences..."), "<control>G", "Preferences for xlog", G_CALLBACK(on_menu_preferences_activate) },

  { "Documentation", GTK_STOCK_HELP, N_("Documentation"), "", "Important documents", NULL },
  { "DXCC List", GTK_STOCK_JUSTIFY_FILL, N_("DXCC list"), "<control><shift>D", "Show cty.dat", G_CALLBACK(on_menu_dxcclist_activate) },
  { "Keys", GTK_STOCK_DIALOG_INFO, N_("Keys"), "<control>Z", "Xlog keys", G_CALLBACK(on_helpkeys_activate) },
  { "Manual", GTK_STOCK_DIALOG_INFO, N_("Manual"), "<control>H", "Xlog manual", G_CALLBACK(on_helpmanual_activate) },
  { "About", GTK_STOCK_ABOUT, N_("About..."), "<control><shift>I", "About xlog", G_CALLBACK(on_menu_about_activate) },

  { "Bugs", GTK_STOCK_DIALOG_INFO, N_("Bugs"), "", "How to report bugs", G_CALLBACK(on_helpbugs_activate) },
  { "Changelog", GTK_STOCK_DIALOG_INFO, N_("Changelog"), "", "Read the changelog", G_CALLBACK(on_helpchangelog_activate) },
  { "Todo", GTK_STOCK_DIALOG_INFO, N_("Todo"), "", "Things to do for future versions", G_CALLBACK(on_helptodo_activate) },

  { "1", GTK_STOCK_JUMP_TO, "Log 1", "<alt>1", "", G_CALLBACK(on_menu_log_activate) },
  { "2", GTK_STOCK_JUMP_TO, "Log 2", "<alt>2", "", G_CALLBACK(on_menu_log_activate) },
  { "3", GTK_STOCK_JUMP_TO, "Log 3", "<alt>3", "", G_CALLBACK(on_menu_log_activate) },
  { "4", GTK_STOCK_JUMP_TO, "Log 4", "<alt>4", "", G_CALLBACK(on_menu_log_activate) },
  { "5", GTK_STOCK_JUMP_TO, "Log 5", "<alt>5", "", G_CALLBACK(on_menu_log_activate) },
  { "6", GTK_STOCK_JUMP_TO, "Log 6", "<alt>6", "", G_CALLBACK(on_menu_log_activate) },
  { "7", GTK_STOCK_JUMP_TO, "Log 7", "<alt>7", "", G_CALLBACK(on_menu_log_activate) },
  { "8", GTK_STOCK_JUMP_TO, "Log 8", "<alt>8", "", G_CALLBACK(on_menu_log_activate) },
  { "9", GTK_STOCK_JUMP_TO, "Log 9", "<alt>9", "", G_CALLBACK(on_menu_log_activate) },
  { "10", GTK_STOCK_JUMP_TO, "Log 10", "<alt>0", "", G_CALLBACK(on_menu_log_activate) },
};

static GtkToggleActionEntry toggle_entries[] =
{
  { "Scoring", NULL, N_("Scoring Window"), "<control>J", "Show/hide the scoring window", G_CALLBACK(on_scoring_activate) },
  { "ShowToolbar", NULL, N_("Show Toolbar"), "<control>T", "Show/hide the toolbar", G_CALLBACK(on_view_toolbar_activate) },
  { "WorkedBefore", NULL, N_("Worked Before"), "<control><shift>B", "Show/hide the worked before window", G_CALLBACK(on_menu_worked_activate) },
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"	 <menu action='LogMenu'>"
"		<menuitem action='New'/>"
"		<menuitem action='Open'/>"
"		<separator name='sep1'/>"
"		<menuitem action='Save'/>"
"		<menuitem action='Save As'/>"
"		<separator name='sep2'/>"
"		<menuitem action='Export'/>"
"		<menuitem action='Import'/>"
"		<menuitem action='Merge'/>"
#ifndef G_OS_WIN32
"		<separator name='sep2b'/>"
"		<menuitem action='PageSetup'/>"
"		<menuitem action='Print'/>"
#endif
"		<separator name='sep3'/>"
"		<menuitem action='Close'/>"
"		<menuitem action='Quit'/>"
"	 </menu>"
"	 <menu action='EditMenu'>"
"		<menuitem action='Write'/>"
"		<menuitem action='Update'/>"
"		<menuitem action='Delete'/>"
"		<separator name='sep4'/>"
"		<menuitem action='Clear All'/>"
"		<menuitem action='Click All'/>"
"		<separator name='sep4a'/>"
"		<menuitem action='Write and Click All'/>"
"		<separator name='sep5'/>"
"		<menuitem action='Find'/>"
"	 </menu>"
"	 <menu action='OptionMenu'>"
"		<menuitem action='CountryMap'/>"
#ifndef G_OS_WIN32
"		<menuitem action='Keyer'/>"
#endif
"		<separator name='sep5a'/>"
"		<menuitem action='Scoring'/>"
"		<menuitem action='WorkedBefore'/>"
"		<separator name='sep5b'/>"
"		<menuitem action='ShowToolbar'/>"
"	 </menu>"
"	 <menu action='ToolsMenu'>"
"		<menuitem action='Dupe Check'/>"
"		<menuitem action='DXCC Check'/>"
"		<menuitem action='Log Editor'/>"
"		<menuitem action='Trace Hamlib'/>"
"		<menuitem action='Sort'/>"
"		<separator name='sep6'/>"
"		<menu action='Awards'>"
"		  <menuitem action='DXCC'/>"
"		  <menuitem action='WAC'/>"
"		  <menuitem action='WAS'/>"
"		  <menuitem action='WAZ'/>"
"		  <menuitem action='IOTA'/>"
"		  <menuitem action='Locator'/>"
"		  <menuitem action='LocatorMap'/>"
"		</menu>"
"	 </menu>"
"	 <menu action='TabsMenu'>"
"		<menuitem action='1'/>"
"		<menuitem action='2'/>"
"		<menuitem action='3'/>"
"		<menuitem action='4'/>"
"		<menuitem action='5'/>"
"		<menuitem action='6'/>"
"		<menuitem action='7'/>"
"		<menuitem action='8'/>"
"		<menuitem action='9'/>"
"		<menuitem action='10'/>"
"	 </menu>"
"	 <menu action='SettingsMenu'>"
"		<menuitem action='Defaults'/>"
"		<menuitem action='Dialogs'/>"
"		<separator name='sep7'/>"
"		<menuitem action='Preferences'/>"
"	 </menu>"
"	 <menu action='HelpMenu'>"
"		<menu action='Documentation'>"
"		  <menuitem action='Bugs'/>"
"		  <menuitem action='Changelog'/>"
"		  <menuitem action='Todo'/>"
"		</menu>"
"	  <separator name='sep8'/>"
"	  <menuitem action='DXCC List'/>"
"	  <menuitem action='Keys'/>"
"	  <menuitem action='Manual'/>"
"		 <separator name='sep9'/>"
"		 <menuitem action='About'/>"
"	 </menu>"
"  </menubar>"
"  <toolbar name='ToolBar'>"
"	 <toolitem action='Write'/>"
"	 <toolitem action='Update'/>"
"	 <toolitem action='Delete'/>"
"  </toolbar>"
"</ui>";

#ifndef G_OS_WIN32
	register_my_stock_icons ();
#endif

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "xlog");
	window_icon_pixbuf = create_pixbuf ("xlog.png");
	if (window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		g_object_unref (window_icon_pixbuf);
	}

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, PACKAGE);
	gtk_action_group_add_actions (action_group, entries,
		G_N_ELEMENTS (entries), window);

	if (preferences.viewtoolbar == 1)
		toggle_entries[1].is_active = TRUE;
	if (preferences.viewb4 == 1)
		toggle_entries[2].is_active = TRUE;
	gtk_action_group_add_toggle_actions (action_group,
		toggle_entries, G_N_ELEMENTS (toggle_entries), window);

	ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	error = NULL;
	merge_id = gtk_ui_manager_add_ui_from_string
		(ui_manager, ui_description, -1, &error);
	if (!merge_id)
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		return (NULL);
	}
	menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (mainvbox), menubar, FALSE, FALSE, 0);

	hpaned = gtk_hpaned_new ();
	gtk_box_pack_start (GTK_BOX (mainvbox), hpaned, TRUE, TRUE, 0);
	qsomainvbox = gtk_vbox_new (FALSE, 0);
	gtk_paned_pack1 (GTK_PANED (hpaned), qsomainvbox, FALSE, TRUE);

	/* toolbar */
	handlebox = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (qsomainvbox), handlebox, FALSE, FALSE, 0);
	toolbar = gtk_ui_manager_get_widget (ui_manager, "/ToolBar");
	gtk_container_add (GTK_CONTAINER (handlebox), toolbar);

	/* scrolledwindow for the QSO information fields and frames */
	qsoscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (qsomainvbox), qsoscrolledwindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (qsoscrolledwindow),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	qsoviewport = gtk_viewport_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (qsoscrolledwindow), qsoviewport);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (qsoviewport), GTK_SHADOW_NONE);
	qsoframesvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (qsoviewport), qsoframesvbox);

	/* QSO information fields */
	qsoframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (qsoframesvbox), qsoframe, FALSE, FALSE, 0);
	gtk_frame_set_label_align (GTK_FRAME (qsoframe), 0.5, 0.5);
	qsofieldsvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (qsoframe), qsofieldsvbox);

	datehbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), datehbox, FALSE, FALSE, 0);
	datebutton = gtk_button_new_with_mnemonic (_("_Date"));
	gtk_box_pack_start (GTK_BOX (datehbox), datebutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (datebutton, 100, -1);
	dateentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (datehbox), dateentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (dateentry), 15);

	gmthbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), gmthbox, FALSE, FALSE, 0);
	gmtbutton = gtk_button_new_with_mnemonic ("_UTC");
	gtk_box_pack_start (GTK_BOX (gmthbox), gmtbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (gmtbutton, 100, -1);
	gmtentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (gmthbox), gmtentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (gmtentry), 8);

	endhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), endhbox, FALSE, FALSE, 0);
	endbutton = gtk_button_new_with_mnemonic (_("_End (UTC )"));
	gtk_box_pack_start (GTK_BOX (endhbox), endbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (endbutton, 100, -1);
	endentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (endhbox), endentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (endentry), 8);

	callhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), callhbox, FALSE, FALSE, 0);
	calllabel = gtk_label_new_with_mnemonic (_("_Call"));
	gtk_box_pack_start (GTK_BOX (callhbox), calllabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (calllabel, 100, 17);
	callentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(calllabel), callentry);
	gtk_box_pack_start (GTK_BOX (callhbox), callentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (callentry), 15);
	GtkWidget *callbutton = gtk_button_new_with_mnemonic ("_?");
	gtk_box_pack_start (GTK_BOX (callhbox), callbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (callbutton, 20, -1);
	bandhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), bandhbox, FALSE, FALSE, 0);
	mhzhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (bandhbox), mhzhbox, FALSE, FALSE, 0);
	mhzbutton = gtk_button_new_with_mnemonic ("_MHz");
	gtk_box_pack_start (GTK_BOX (mhzhbox), mhzbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (mhzbutton, 100, -1);
	mhzlabel = gtk_label_new_with_mnemonic ("_MHz");
	gtk_box_pack_start (GTK_BOX (mhzhbox), mhzlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (mhzlabel, 100, -1);
	bandhbox2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (bandhbox), bandhbox2, TRUE, TRUE, 0);
	bandentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (bandhbox2), bandentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (bandentry), 15);

	modehbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), modehbox, FALSE, FALSE, 0);
	modehbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (modehbox), modehbox1, FALSE, FALSE, 0);
	modebutton = gtk_button_new_with_mnemonic (_("M_ode"));
	gtk_box_pack_start (GTK_BOX (modehbox1), modebutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (modebutton, 100, -1);
	modelabel = gtk_label_new_with_mnemonic (_("M_ode"));
	gtk_box_pack_start (GTK_BOX (modehbox1), modelabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (modelabel, 100, -1);
	modehbox2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (modehbox), modehbox2, TRUE, TRUE, 0);
	modeentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (modehbox2), modeentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (modeentry), 8);

	hrsthbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), hrsthbox, FALSE, FALSE, 0);
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hrsthbox), hbox1, FALSE, FALSE, 0);
	rstbutton = gtk_button_new_with_mnemonic ("_TX(RST)");
	gtk_box_pack_start (GTK_BOX (hbox1), rstbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (rstbutton, 100, -1);
	rstlabel = gtk_label_new_with_mnemonic ("_TX(RST)");
	gtk_box_pack_start (GTK_BOX (hbox1), rstlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (rstlabel, 100, -1);
	rstentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(rstlabel), rstentry);
	gtk_box_pack_start (GTK_BOX (hrsthbox), rstentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (rstentry), 15);

	mrsthbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), mrsthbox, FALSE, FALSE, 0);
	myrstlabel = gtk_label_new_with_mnemonic ("_RX(RST)");
	gtk_box_pack_start (GTK_BOX (mrsthbox), myrstlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (myrstlabel, 100, -1);
	myrstentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(myrstlabel), myrstentry);
	gtk_box_pack_start (GTK_BOX (mrsthbox), myrstentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (myrstentry), 15);

	qslhbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), qslhbox, FALSE, FALSE, 0);
	outcheckbutton = gtk_check_button_new_with_mnemonic (_("_QSL out"));
	gtk_box_pack_start (GTK_BOX (qslhbox), outcheckbutton, FALSE, FALSE, 0);
	incheckbutton = gtk_check_button_new_with_mnemonic (_("Q_SL in"));
	gtk_box_pack_start (GTK_BOX (qslhbox), incheckbutton, FALSE, FALSE, 0);

	awardshbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), awardshbox, FALSE, FALSE, 0);
	awardslabel = gtk_label_new_with_mnemonic (_("_Awards"));
	gtk_box_pack_start (GTK_BOX (awardshbox), awardslabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (awardslabel, 100, -1);
	awardsentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(awardslabel), awardsentry);
	gtk_box_pack_start (GTK_BOX (awardshbox), awardsentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (awardsentry), 30);

	powerhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), powerhbox, FALSE, FALSE, 0);
	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (powerhbox), hbox2, FALSE, FALSE, 0);
	powerbutton = gtk_button_new_with_mnemonic (_("_Power"));
	gtk_box_pack_start (GTK_BOX (hbox2), powerbutton, FALSE, FALSE, 0);
	gtk_widget_set_size_request (powerbutton, 100, -1);
	powerlabel = gtk_label_new_with_mnemonic (_("_Power"));
	gtk_box_pack_start (GTK_BOX (hbox2), powerlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (powerlabel, 100, -1);
	powerentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(powerlabel), powerentry);
	gtk_box_pack_start (GTK_BOX (powerhbox), powerentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (powerentry), 8);

	namehbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), namehbox, FALSE, FALSE, 0);
	namelabel = gtk_label_new_with_mnemonic (_("_Name"));
	gtk_box_pack_start (GTK_BOX (namehbox), namelabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (namelabel, 100, -1);
	nameentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(namelabel), nameentry);
	gtk_box_pack_start (GTK_BOX (namehbox), nameentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (nameentry), 30);

	qthhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), qthhbox, FALSE, FALSE, 0);
	qthlabel = gtk_label_new_with_mnemonic ("QT_H");
	gtk_box_pack_start (GTK_BOX (qthhbox), qthlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (qthlabel, 100, -1);
	qthentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(qthlabel), qthentry);
	gtk_box_pack_start (GTK_BOX (qthhbox), qthentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (qthentry), 30);

	locatorhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), locatorhbox, FALSE, FALSE, 0);
	locatorlabel = gtk_label_new_with_mnemonic (_("_Locator"));
	gtk_box_pack_start (GTK_BOX (locatorhbox), locatorlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (locatorlabel, 100, -1);
	locatorentry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(locatorlabel), locatorentry);
	gtk_box_pack_start (GTK_BOX (locatorhbox), locatorentry, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (locatorentry), 8);

	unknown1hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), unknown1hbox, FALSE, FALSE, 0);
	unknownlabel1 = gtk_label_new_with_mnemonic ("");
	gtk_box_pack_start (GTK_BOX (unknown1hbox), unknownlabel1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (unknownlabel1, 100, -1);
	unknownentry1 = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(unknownlabel1), unknownentry1);
	gtk_box_pack_start (GTK_BOX (unknown1hbox), unknownentry1, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (unknownentry1), 30);

	unknown2hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), unknown2hbox, FALSE, FALSE, 0);
	unknownlabel2 = gtk_label_new_with_mnemonic ("");
	gtk_box_pack_start (GTK_BOX (unknown2hbox), unknownlabel2, FALSE, FALSE, 0);
	gtk_widget_set_size_request (unknownlabel2, 100, -1);
	unknownentry2 = gtk_entry_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(unknownlabel2), unknownentry2);
	gtk_box_pack_start (GTK_BOX (unknown2hbox), unknownentry2, TRUE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (unknownentry2), 30);

	remarksvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (qsofieldsvbox), remarksvbox, FALSE, FALSE, 0);
	rembox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (remarksvbox), rembox1, FALSE, FALSE, 0);
	remarkslabel = gtk_label_new_with_mnemonic (_("Remar_ks"));
	gtk_box_pack_start (GTK_BOX (rembox1), remarkslabel, TRUE, TRUE, 0);
	rembox2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (remarksvbox), rembox2, FALSE, FALSE, 0);
	remscr = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (remscr, 100, 60);
	gtk_container_add (GTK_CONTAINER (rembox2), remscr);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (remscr),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type
		(GTK_SCROLLED_WINDOW (remscr), GTK_SHADOW_ETCHED_IN);
	remtv = gtk_text_view_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL(remarkslabel), remtv);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(remtv), GTK_WRAP_WORD);
	gtk_container_add (GTK_CONTAINER (remscr), remtv);

	temp = g_strdup_printf ("<b>%s</b>", _("New QSO"));
	label1 = gtk_label_new (NULL);
	gtk_widget_set_size_request (label1, 100, 17);
	gtk_label_set_markup (GTK_LABEL (label1), temp);
	g_free (temp);
	gtk_frame_set_label_widget (GTK_FRAME (qsoframe), label1);

	/* status bar */
	statushbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (mainvbox), statushbox, FALSE, FALSE, 0);
	statusbar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (statushbox), statusbar, TRUE, TRUE, 0);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);
	frequencyhandlebox = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (statushbox), frequencyhandlebox, FALSE, FALSE, 0);
	frequencyhbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frequencyhandlebox), frequencyhbox);
	frequencypixmap = create_pixmap (window, "mini-trx.xpm");
	gtk_box_pack_start (GTK_BOX (frequencyhbox), frequencypixmap, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (frequencypixmap), 5, 0);
	frequencylabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (frequencyhbox), frequencylabel, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (frequencylabel), 5, 0);
	smeterhandlebox = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (statushbox), smeterhandlebox, FALSE, FALSE, 0);
	smeterhbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (smeterhandlebox), smeterhbox);
	gtk_container_set_border_width (GTK_CONTAINER (smeterhbox), 2);
	smeterpixmap = create_pixmap (window, "s.xpm");
	gtk_box_pack_start (GTK_BOX (smeterhbox), smeterpixmap, FALSE, FALSE, 0);
	smeterdrawingarea = gtk_drawing_area_new ();
	gtk_box_pack_start (GTK_BOX (smeterhbox), smeterdrawingarea, TRUE, TRUE, 0);
	gtk_widget_set_size_request (smeterdrawingarea, 144, 20);
	clockhandlebox = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (statushbox), clockhandlebox, FALSE, FALSE, 0);
	clockhbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (clockhandlebox), clockhbox);
	clockpixmap = create_pixmap (window, "mini-clock.xpm");
	gtk_box_pack_start (GTK_BOX (clockhbox), clockpixmap, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (clockpixmap), 5, 0);
	clocklabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (clockhbox), clocklabel, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (clocklabel), 5, 0);

	gtk_widget_show_all (mainvbox);
	g_signal_connect ((gpointer) window, "delete_event",
		G_CALLBACK (on_mainwindow_delete_event), NULL);
	g_signal_connect ((gpointer) window, "key_press_event",
		G_CALLBACK (on_mainwindow_keypress), NULL);
	g_signal_connect ((gpointer) datebutton, "clicked",
		G_CALLBACK (on_datebutton_clicked), NULL);
	g_signal_connect ((gpointer) gmtbutton, "clicked",
		G_CALLBACK (on_gmtbutton_clicked), NULL);
	g_signal_connect ((gpointer) endbutton, "clicked",
		G_CALLBACK (on_endbutton_clicked), NULL);
	g_signal_connect ((gpointer) callentry, "insert_text",
		G_CALLBACK (on_callentry_insert_text), NULL);
	g_signal_connect ((gpointer) callentry, "changed",
		G_CALLBACK (on_callentry_changed), NULL);
	g_signal_connect ((gpointer) callentry, "focus-out-event",
		G_CALLBACK (on_callentry_unfocus), NULL);
	g_signal_connect ((gpointer) callbutton, "clicked",
		G_CALLBACK (on_callbutton_clicked), NULL);
	g_signal_connect ((gpointer) locatorentry, "changed",
		G_CALLBACK (on_locatorentry_changed), NULL);
	b = gtk_text_view_get_buffer (GTK_TEXT_VIEW(remtv));
	g_signal_connect (G_OBJECT(b), "changed", G_CALLBACK (tv_changed), NULL);
	g_signal_connect ((gpointer) awardsentry, "insert_text",
		G_CALLBACK (on_awardsentry_insert_text), NULL);
	g_signal_connect ((gpointer) awardsentry, "changed",
		G_CALLBACK (on_awardsentry_changed), NULL);

	g_signal_connect (G_OBJECT(bandentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(1));
	g_signal_connect (G_OBJECT(modeentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(2));
	g_signal_connect (G_OBJECT(rstentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(3));
	g_signal_connect (G_OBJECT(myrstentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(4));
	g_signal_connect (G_OBJECT(awardsentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(5));
	g_signal_connect (G_OBJECT(powerentry), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(6));
	g_signal_connect (G_OBJECT(unknownentry1), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(7));
	g_signal_connect (G_OBJECT(unknownentry2), "mnemonic-activate",
		G_CALLBACK (entry_mnemonic_activate), GINT_TO_POINTER(8));

	g_signal_connect (G_OBJECT(remtv), "mnemonic-activate",
		G_CALLBACK (tv_mnemonic_activate), NULL);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (window, window, "window");

	GLADE_HOOKUP_OBJECT (window, handlebox, "handlebox");
	GLADE_HOOKUP_OBJECT (window, hpaned, "hpaned");
	GLADE_HOOKUP_OBJECT (window, qsoframe, "qsoframe");
	GLADE_HOOKUP_OBJECT (window, datebutton, "datebutton");
	GLADE_HOOKUP_OBJECT (window, dateentry, "dateentry");
	GLADE_HOOKUP_OBJECT (window, gmtbutton, "gmtbutton");
	GLADE_HOOKUP_OBJECT (window, gmtentry, "gmtentry");
	GLADE_HOOKUP_OBJECT (window, endhbox, "endhbox");
	GLADE_HOOKUP_OBJECT (window, endbutton, "endbutton");
	GLADE_HOOKUP_OBJECT (window, endentry, "endentry");
	GLADE_HOOKUP_OBJECT (window, calllabel, "calllabel");
	GLADE_HOOKUP_OBJECT (window, callentry, "callentry");
	GLADE_HOOKUP_OBJECT (window, mhzbutton, "mhzbutton");
	GLADE_HOOKUP_OBJECT (window, mhzlabel, "mhzlabel");
	GLADE_HOOKUP_OBJECT (window, bandhbox2, "bandhbox2");
	GLADE_HOOKUP_OBJECT (window, bandentry, "bandentry");
	GLADE_HOOKUP_OBJECT (window, modebutton, "modebutton");
	GLADE_HOOKUP_OBJECT (window, modelabel, "modelabel");
	GLADE_HOOKUP_OBJECT (window, modehbox2, "modehbox2");
	GLADE_HOOKUP_OBJECT (window, modeentry, "modeentry");
	GLADE_HOOKUP_OBJECT (window, rstbutton, "rstbutton");
	GLADE_HOOKUP_OBJECT (window, rstlabel, "rstlabel");
	GLADE_HOOKUP_OBJECT (window, rstentry, "rstentry");
	GLADE_HOOKUP_OBJECT (window, myrstlabel, "myrstlabel");
	GLADE_HOOKUP_OBJECT (window, myrstentry, "myrstentry");
	GLADE_HOOKUP_OBJECT (window, awardshbox, "awardshbox");
	GLADE_HOOKUP_OBJECT (window, awardslabel, "awardslabel");
	GLADE_HOOKUP_OBJECT (window, awardsentry, "awardsentry");
	GLADE_HOOKUP_OBJECT (window, qslhbox, "qslhbox");
	GLADE_HOOKUP_OBJECT (window, outcheckbutton, "outcheckbutton");
	GLADE_HOOKUP_OBJECT (window, incheckbutton, "incheckbutton");
	GLADE_HOOKUP_OBJECT (window, powerhbox, "powerhbox");
	GLADE_HOOKUP_OBJECT (window, powerbutton, "powerbutton");
	GLADE_HOOKUP_OBJECT (window, powerlabel, "powerlabel");
	GLADE_HOOKUP_OBJECT (window, powerentry, "powerentry");
	GLADE_HOOKUP_OBJECT (window, namehbox, "namehbox");
	GLADE_HOOKUP_OBJECT (window, namelabel, "namelabel");
	GLADE_HOOKUP_OBJECT (window, nameentry, "nameentry");
	GLADE_HOOKUP_OBJECT (window, qthhbox, "qthhbox");
	GLADE_HOOKUP_OBJECT (window, qthlabel, "qthlabel");
	GLADE_HOOKUP_OBJECT (window, qthentry, "qthentry");
	GLADE_HOOKUP_OBJECT (window, locatorhbox, "locatorhbox");
	GLADE_HOOKUP_OBJECT (window, locatorlabel, "locatorlabel");
	GLADE_HOOKUP_OBJECT (window, locatorentry, "locatorentry");
	GLADE_HOOKUP_OBJECT (window, unknown1hbox, "unknown1hbox");
	GLADE_HOOKUP_OBJECT (window, unknownlabel1, "unknownlabel1");
	GLADE_HOOKUP_OBJECT (window, unknownentry1, "unknownentry1");
	GLADE_HOOKUP_OBJECT (window, unknown2hbox, "unknown2hbox");
	GLADE_HOOKUP_OBJECT (window, unknownlabel2, "unknownlabel2");
	GLADE_HOOKUP_OBJECT (window, unknownentry2, "unknownentry2");
	GLADE_HOOKUP_OBJECT (window, remarksvbox, "remarksvbox");
	GLADE_HOOKUP_OBJECT (window, remarkslabel, "remarkslabel");
	GLADE_HOOKUP_OBJECT (window, remtv, "remtv");

	GLADE_HOOKUP_OBJECT (window, statusbar, "statusbar");
	GLADE_HOOKUP_OBJECT (window, frequencyhandlebox, "frequencyhandlebox");
	GLADE_HOOKUP_OBJECT (window, frequencylabel, "frequencylabel");
	GLADE_HOOKUP_OBJECT (window, smeterhandlebox, "smeterhandlebox");
	GLADE_HOOKUP_OBJECT (window, smeterhbox, "smeterhbox");
	GLADE_HOOKUP_OBJECT (window, smeterpixmap, "smeterpixmap");
	GLADE_HOOKUP_OBJECT (window, smeterdrawingarea, "smeterdrawingarea");
	GLADE_HOOKUP_OBJECT (window, clockhandlebox, "clockhandlebox");
	GLADE_HOOKUP_OBJECT (window, clocklabel, "clocklabel");

	/* prevent F10 from using the menu */
	GtkSettings *settings = gtk_settings_get_default();
	g_object_set(settings, "gtk-menu-bar-accel", "<control>F10", NULL);

	return window;
}
