/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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
 * dxcc.c - dxcc lookups and creation of the hashtable
 */

//
// As described on this website:
// http://n1mm.hamdocs.com/tiki-index.php?page=Customizing+the+DXCC+List
//
// CTY.DAT file format
// The format from CTY.DAT is as follows:
// Column	Length	Description
// 1	26	Country name terminated by a colon character.
// 27	5	CQ zone terminated by a colon character.
// 32	5	ITU zone terminated by a colon character.
// 37	5	2-letter continent abbreviation terminated by a colon character.
// 42	9	Latitude in degrees, + for North terminated by a colon character.
// 51	9	Longitude in degrees, + for West terminated by a colon character.
// 61	9	Local time offset from GMT terminated by a colon character.
// 69	6	Primary DXCC Prefix terminated by a colon character.
// next line(s)	List of prefixes assigned to that country, each one separated
//              by a comma and terminated by a semicolon.
// 
// The fields are aligned in columns and spaced out for readability only.
// It is the ":" at the end of each field that acts as a delimiter for that field.
// 
// Alias DXCC prefixes (including the primary one) follow on consecutive lines,
// separated by ",". If there is more than one line, subsequent lines begin
// with the "&" continuation character. A ";" terminates the last prefix in 
// the list.
// 
// "Prefixes" which start with "=" are not prefixes, but are full callsigns
// and seem to be exceptions to the rules.
//
// If the country spans multiple zones, then the prefix may be followed by a 
// CQWW zone number in parenthesis, and it may also be followed by an ITU zone
// number in square brackets, or both, but the CQ zone number in parenthesis
// must precede the ITU zone number in square brackets.
// 
// The following special characters can be applied to an alias prefix:
// (#) Override CQ zone where # is the zone number
// [#] Override ITU zone where # is the zone number 
//

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <ctype.h>

#include "dxcc.h"
#include "gc.h"
#include "support.h"
#include "cfg.h"
#include "xlog_enum.h"
#include "awards_enum.h"
#include "utils.h"
#include "main.h"
#include "log.h"
#include "gui_countrymap.h"

extern GtkWidget *mainwindow, *scorewindow, *countrymap;
extern preferencestype preferences;
extern programstatetype programstate;
extern GList *logwindowlist;
GPtrArray *dxcc, *area;
GHashTable *prefixes, *full_callsign_exceptions;
gint excitu, exccq;

/* gushort: 0 - 65534, one extra element for all bands scoring */
gushort dxcc_w[400][MAX_BANDS + 1];
gushort dxcc_c[400][MAX_BANDS + 1];
gushort waz_w[MAX_ZONES][MAX_BANDS + 1];
gushort waz_c[MAX_ZONES][MAX_BANDS + 1];
gushort wac_w[MAX_CONTINENTS][MAX_BANDS + 1];
gushort wac_c[MAX_CONTINENTS][MAX_BANDS + 1];
gushort was_w[MAX_STATES][MAX_BANDS + 1];
gushort was_c[MAX_STATES][MAX_BANDS + 1];
GHashTable *iota_w[MAX_BANDS + 1];
GHashTable *iota_c[MAX_BANDS + 1];
GHashTable *loc_w[MAX_BANDS + 1];
GHashTable *loc_c[MAX_BANDS + 1];

/* free memory used by the dxcc array */
void
cleanup_dxcc (void)
{
  gint i;

  /* free the dxcc array */
  if (dxcc)
    {
      for (i = 0; i < dxcc->len; i++)
	{
	  dxcc_data *d = g_ptr_array_index (dxcc, i);
	  g_free (d->countryname);
	  g_free (d->px);
	  g_free (d->exceptions);
	  g_free (d);
	}
      g_ptr_array_free (dxcc, TRUE);
    }
  if (prefixes) g_hash_table_destroy (prefixes);
  if (full_callsign_exceptions) g_hash_table_destroy (full_callsign_exceptions);

  for (i = 0; i < MAX_BANDS; i++)
    {
      if (iota_w[i]) g_hash_table_destroy (iota_w[i]);
      if (iota_c[i]) g_hash_table_destroy (iota_c[i]);
      if (loc_w[i]) g_hash_table_destroy (loc_w[i]);
      if (loc_c[i]) g_hash_table_destroy (loc_c[i]);
    }
}

/* free memory used by the area array */
void
cleanup_area (void)
{
  gint i;

  /* free the dxcc array */
  if (area)
    {
      for (i = 0; i < area->len; i++)
	{
	  area_data *a = g_ptr_array_index (area, i);
	  g_free (a->countryname);
	  g_free (a->continent);
	  g_free (a->px);
	  g_free (a);
	}
      g_ptr_array_free (area, TRUE);
    }
}

/*
 * go through exception string and stop when end of prefix
 * is reached (BT3L(23)[33] -> BT3L)
 */
static gchar *
findpfx_in_exception (gchar * pfx)
{
  gchar *end, *j;

  g_strstrip (pfx);
  end = pfx + strlen (pfx);
  for (j = pfx; j < end; ++j)
    {
      switch (*j)
	{
	case '(':
	case '[':
	case ';':
	  *j = '\0';
	  break;
	}
    }

// BOZO!!!!!!!
  if (pfx[0] == '=')
    return pfx + 1;
  return pfx;
}

/* replace callsign area (K0AR/2 -> K2AR) so we can do correct lookups */
static gchar *
change_area (gchar *callsign, gint area)
{
  gchar *end, *j;

  end = callsign + strlen (callsign);
  for (j = callsign; j < end; ++j)
    {
      switch (*j)
	{
	case '0' ... '9':
	  if ((j - callsign) > 1)
	    *j = area + 48;
	  break;
	}
    }

  return(g_strdup(callsign));
}

/*
   extract prefix from a callsign with a forward slash:
   - check if callsign has a '/'
   - replace callsign area's (K0AR/2 -> K2AR)
   - skip /mm, /am and /qrp
   - return string after slash if it is shorter than string before
 */
static gchar *
getpx (gchar *checkcall)
{

  gchar *pxstr = NULL, **split;

  /* characters after '/' might contain a country */
  if (strchr(checkcall, '/'))
    {
      split = g_strsplit(checkcall, "/", 2);
      if (split[1]) /* we might be typing */
	{
	  if ((strlen(split[1]) > 1) && (strlen(split[1]) < strlen(split[0])))
	    /* this might be a candidate */
	    {
	      if ((g_ascii_strcasecmp(split[1], "AM") == 0)
		  || (g_ascii_strcasecmp(split[1], "MM") == 0))
		pxstr = NULL; /* don't know location */
	      else if (g_ascii_strcasecmp(split[1], "QRP") == 0)
		pxstr = g_strdup(split[0]);
	      else pxstr = g_strdup(split[1]);
	    }
	  else if ((strlen(split[1]) == 1) &&
		   split[1][0] >= '0' && split[1][0] <= '9')
	    /* callsign area changed */
	    {
	      pxstr = change_area(split[0], atoi(split[1]));
	    }
	  else pxstr = g_strdup(split[0]);
	}
      else pxstr = g_strdup(split[0]);
      g_strfreev(split);
    }
  else
    pxstr = g_strdup(checkcall);

  return (pxstr);
}


/* parse an exception and extract the CQ and ITU zone */
static gchar *
findexc(gchar *exception)
{
  gchar *end, *j;

  excitu = 0;
  exccq = 0;
  end = exception + strlen (exception);
  for (j = exception; j < end; ++j)
    {
      switch (*j)
	{
	case '(':
	  if (*(j+2) == 41)
	    exccq = *(j+1) - 48;
	  else if (*(j+3) == 41)
	    exccq = ((*(j+1) - 48) * 10) + (*(j+2) - 48);
	case '[':
	  if (*(j+2) == 93)
	    excitu = *(j+1) - 48;
	  else if (*(j+3) == 93)
	    excitu = ((*(j+1) - 48) * 10) + (*(j+2) - 48);
	case ';':
	  *j = '\0';
	  break;
	}
    }
  return (exception);
}

/*
 * return the country number cq zone and itu zone directly from the array
 */
struct info
lookupcountry_by_prefix (gchar *px)
{
  gint index;
  struct info lookup;

  lookup.country = 0;
  lookup.itu = 0;
  lookup.cq = 0;
  lookup.continent = 99;

  for (index = 0; index < dxcc->len; index++)
    {
      dxcc_data *d = g_ptr_array_index (dxcc, index);
      if (g_ascii_strcasecmp (d->px, px) == 0)
	{
	  lookup.country = index;
	  lookup.itu = d -> itu;
	  lookup.cq = d -> cq;
	  lookup.continent = d -> continent;
	  break;
	}
    }
  return lookup;
}

/*
 * go through the hashtable with the current callsign and return the country number
 * cq zone and itu zone - this also goes through the exceptionlist
 */
struct info
lookupcountry_by_callsign (gchar * callsign)
{
  gint ipx, iexc;
  gchar *px;
  gchar **excsplit, *exc;
  gchar *searchpx = NULL;
  struct info lookup;

  lookup.country = 0;

  /* first check complete callsign exceptions list*/
  lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (full_callsign_exceptions, callsign));

  if (lookup.country == 0) {
    /* Next, check the list of prefixes */
    lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (prefixes, callsign));
  }

  if (lookup.country == 0 && (px = getpx (callsign)))
    {	/* start with full callsign and truncate it until a correct lookup */
      for (ipx = strlen (px); ipx > 0; ipx--)
	{
	  searchpx = g_strndup (px, ipx);
	  lookup.country = GPOINTER_TO_INT(g_hash_table_lookup (prefixes, searchpx));
	  if (lookup.country > 0) break;
	}
      g_free (px);
    }
  else
    searchpx = g_strdup (callsign);

  dxcc_data *d = g_ptr_array_index (dxcc, lookup.country);
  lookup.itu = d -> itu;
  lookup.cq = d -> cq;
  lookup.continent = d -> continent;

  /* look for CQ/ITU zone exceptions */
  if (strchr(d->exceptions, '(') || strchr(d->exceptions, '['))
    {
      excsplit = g_strsplit (d->exceptions, ",", -1);
      for (iexc = 0 ;; iexc++)
	{
	  if (!excsplit[iexc]) break;
	  exc = findexc (excsplit[iexc]);
	  if (g_ascii_strcasecmp (searchpx, exc) == 0)
	    {
	      if (excitu > 0) lookup.itu = excitu;
	      if (exccq > 0) lookup.cq = exccq;
	    }
	}
      g_strfreev(excsplit);
    }
  return lookup;
}

/* add an item from cty.dat to the dxcc array */
static void
dxcc_add (gchar *c, gint w, gint i, gint cont, gint lat, gint lon,
	  gint tz, gchar *p, gchar *e)
{
  dxcc_data *new_dxcc = g_new (dxcc_data, 1);

  new_dxcc -> countryname = g_strdup (c);
  new_dxcc -> cq = w;
  new_dxcc -> itu = i;
  new_dxcc -> continent = cont;
  new_dxcc -> latitude = lat;
  new_dxcc -> longitude = lon;
  new_dxcc -> timezone = tz;
  new_dxcc -> px = g_strdup (p);
  new_dxcc -> exceptions = g_strdup (e);
  g_ptr_array_add (dxcc, new_dxcc);
}

/* add an item from area.dat to the area array */
static void
area_add (gchar *c, gint w, gint i, gchar *cont, gint lat, gint lon,
	  gint tz, gchar *p)
{
  area_data *new_area = g_new (area_data, 1);

  new_area -> countryname = g_strdup (c);
  new_area -> cq = w;
  new_area -> itu = i;
  new_area -> continent = g_strdup (cont);
  new_area -> latitude = lat;
  new_area -> longitude = lon;
  new_area -> timezone = tz;
  new_area -> px = g_strdup (p);
  g_ptr_array_add (area, new_area);
}

gint
readctyversion (void)
{
  gchar buf[131072], *ver, *ch, *cty_location;
  FILE *fp;

#ifdef G_OS_WIN32
  cty_location = g_strconcat ("dxcc", G_DIR_SEPARATOR_S, "cty.dat", NULL);
#else
  cty_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S, "dxcc", G_DIR_SEPARATOR_S, "cty.dat", NULL);
#endif
  if ((fp = g_fopen (cty_location, "r")) == NULL)
    {
      g_free (cty_location);
      return (1);
    }
  g_free (cty_location);
  gint n = fread (buf, 1, 131072, fp);
  buf[n] = '\0';
  ver = strstr (buf, "VER2");
  if (ver)
    {
      ch = strstr (ver, ",");
      if (ch)
	{
	  *ch = '\0';
	  fclose (fp);
	  return atoi(ver+3);
	}
    }
  fclose (fp);
  return 0;
}

/* fill the hashtable with all of the prefixes from cty.dat */
gint
readctydata (void)
{

  gchar buf[65536], *cty_location, *pfx, **split, **pfxsplit;
  gint ichar = 0, dxccitem = 0, ipfx = 0, ch = 0;
  gboolean firstcolon = FALSE;
  gchar tmp[20];
  gint i;
  FILE *fp;

#ifdef G_OS_WIN32
  cty_location = g_strconcat ("dxcc", G_DIR_SEPARATOR_S, "cty.dat", NULL);
#else
  cty_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S, "dxcc", G_DIR_SEPARATOR_S, "cty.dat", NULL);
#endif

  if ((fp = g_fopen (cty_location, "r")) == NULL)
    {
      g_free (cty_location);
      return (1);
    }
  g_free (cty_location);

  dxcc = g_ptr_array_new ();
  prefixes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  full_callsign_exceptions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  /* first field in case hash_table_lookup returns NULL */
  dxcc_add (_("Unknown"), 0, 0, 99, 0, 0, 0, "", "");
  programstate.countries = 1;

  while (!feof (fp))
    {
      while (ch != ';')
	{
	  ch = fgetc (fp);
	  /* ignore space (exept in the country string), new line, carriage return and semicolon */
	  if (ch == EOF) break;
	  if (ch == ':') firstcolon = TRUE;
	  if (ch == ' ' && firstcolon) continue;
	  if (ch == '\r') continue;
	  if (ch == '\n') continue;
	  if (ch == ';') continue;
	  buf[ichar++] = ch;
	}
      if (ch == EOF) break;

      tmp[0] = '\0';
      buf[ichar] = '\0';
      ichar = 0;
      ch = 0;
      firstcolon = FALSE;

      /* split up the first line */
      split = g_strsplit (buf, ":", 9);

      /* ignore WAE countries */
      /* WAE countries count for CQ contests, but not for ARRL contests. */
      if (!g_strrstr (split[7], "*"))
	{
	  for (dxccitem = 0; dxccitem < 9; dxccitem++)
	    g_strstrip (split[dxccitem]);

	  dxcc_add (split[0], atoi(split[1]), atoi(split[2]), cont_to_enum(split[3]),
		    (gint)(strtod(split[4], NULL) * 100), (gint)(strtod(split[5], NULL) * 100),
		    (gint)(strtod(split[6], NULL) * 10), split[7], split[8]);

	  /* NOTE: split[7] is the description prefix, it is not added to the hashtable */
	  /* With this addition, the user can enter the "callsign" like "3D2/R" and the */
	  /* scoring window locator box will show the proper DXCC entity, even though   */
	  /* this isn't a "real" callsign.  AMS 24-feb-2013 */
 
	  if (strchr(split[7], '/')) 
	    {
	      for (i=0; i<strlen(split[7]); i++)
		{
		  tmp[i] = toupper(split[7][i]);
		}
	      tmp[i] = '\0';
	      g_hash_table_insert (prefixes, g_strdup (tmp), GINT_TO_POINTER (programstate.countries));	      
	    }

	  /* split up the second line */
	  /* The second line is made up of prefixes AND exceptions which start with '=' */
	  /* As of 2.0.11, there are two hashes, one for prefixes and one for           */
	  /* callsign exceptions due to a recently discovered bug.                      */

	  pfxsplit = g_strsplit (split[8], ",", 0);
	  for (ipfx = 0;; ipfx++)
	  {
	    if (!pfxsplit[ipfx]) break;

	    pfx = findpfx_in_exception (pfxsplit[ipfx]);
	    if (!strncmp(pfxsplit[ipfx], "=", 1)) {
	      g_hash_table_insert (full_callsign_exceptions, g_strdup (pfx), GINT_TO_POINTER (programstate.countries));
	    } else {
	      g_hash_table_insert (prefixes, g_strdup (pfx), GINT_TO_POINTER (programstate.countries));
	    }
	  }
	  g_strfreev (pfxsplit);
	  programstate.countries++;
	}
	g_strfreev (split);
    }
  fclose (fp);
  return (0);
}

/* fill the hashtable with all of the prefixes from area.dat */
gint
readareadata (void)
{

  gchar buf[4096], *area_location, **split;
  gint ichar = 0, ch = 0;
  FILE *fp;

#ifdef G_OS_WIN32
  area_location = g_strconcat ("dxcc", G_DIR_SEPARATOR_S, "area.dat", NULL);
#else
  area_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S, "dxcc", G_DIR_SEPARATOR_S, "area.dat", NULL);
#endif
  if ((fp = g_fopen (area_location, "r")) == NULL)
    {
      g_free (area_location);
      return (1);
    }

  area = g_ptr_array_new ();

  while (!feof (fp))
    {
      while (ch != 59)
	{
	  ch = fgetc (fp);
	  if (ch == EOF) break;
	  if (ch == 59) continue;
	  buf[ichar++] = ch;
	}
      if (ch == EOF) break;

      buf[ichar] = '\0';
      ichar = 0;
      ch = 0;

      /* split up the line */
      split = g_strsplit (buf, ":", 9);
      area_add (split[0], atoi(split[1]), atoi(split[2]), split[3],
		(gint)(strtod(split[4], NULL) * 100), (gint)(strtod(split[5], NULL) * 100),
		(gint)(strtod(split[6], NULL) * 10), split[7]);
      g_strfreev (split);
    }
  fclose (fp);
  g_free (area_location);
  return (0);
}

/* search a callsign and return the callsign area */
gchar lookuparea (gchar *callsign)
{
  gchar *end, *j, *slash;

  end = callsign + strlen (callsign);
  if ((slash = strchr(callsign, '/')))
    {
      for (j = slash; j < end; ++j)
	switch (*j)
	  {
	  case '0' ... '9':
	    return (*j);
	  }
    }
  end = callsign + strlen (callsign);
  for (j = callsign; j < end; ++j)
    {
      switch (*j)
	{
	case '0' ... '9':
	  return (*j);
	}
    }
  return '?';
}

/* update information of the DXCC frame with the current callsign */
void
updatedxccframe (gchar * item, gboolean byprefix, gint st, gint zone, gint cont, guint iota)
{
  GtkWidget *dxcclabel1, *dxcclabel3, *dxcclabel4, *contlabel, *itulabel,
    *cqlabel, *dxcclabel5, *dxccframe, *framelabel, *countrytreeview;
  gchar *labeltext1, *labeltext3, *labeltext4, *gcresult, *conttext, *itutext,
    *cqtext, *temp;
  GtkTreeIter iter;
  GtkTreeModel *model;
  struct info lookup;
  gchar iter_num[2] = "0";
  gint j;
  gdouble lat, lon;

  dxcclabel1 = lookup_widget (scorewindow, "dxcclabel1");
  dxcclabel3 = lookup_widget (scorewindow, "dxcclabel3");
  dxcclabel4 = lookup_widget (scorewindow, "dxcclabel4");
  dxcclabel5 = lookup_widget (scorewindow, "dxcclabel5");
  dxccframe = lookup_widget (scorewindow, "dxccframe");
  contlabel = lookup_widget (scorewindow, "contlabel");
  itulabel = lookup_widget (scorewindow, "itulabel");
  cqlabel = lookup_widget (scorewindow, "cqlabel");
  labeltext1 = g_strdup ("");
  labeltext3 = g_strdup ("");
  labeltext4 = g_strdup ("");
  gcresult = g_strdup ("");
  conttext = g_strdup ("");
  itutext = g_strdup ("");
  cqtext = g_strdup ("");

  if (byprefix)
    lookup = lookupcountry_by_prefix (item);
  else
    lookup = lookupcountry_by_callsign (item);

  if (cont != 99)
    lookup.continent = cont;

  /* update the frame */
  framelabel = gtk_frame_get_label_widget (GTK_FRAME(dxccframe));
  temp = g_strdup_printf ("<b>%s</b>", item);
  gtk_label_set_markup (GTK_LABEL (framelabel), temp);

  countrytreeview = lookup_widget (scorewindow, "countrytreeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (countrytreeview));

  /* update DXCC */
  dxcc_data *d = g_ptr_array_index (dxcc, lookup.country);
  if (lookup.country != 0)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, d->px, -1);
      if (countrymap)
	{
	  if (strcmp (programstate.px, d->px))
	    {
	      countrymap_refresh (d->px);
	      programstate.px = g_strdup (d->px);
	    }
	}
      iter_num[0]++;
      for (j = 0; j < MAX_BANDS; j++)
	{
	  if (dxcc_c[lookup.country][j] > 0)
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	  else if (dxcc_w[lookup.country][j] > 0)
	    {
	      temp = g_strdup_printf ("%d", dxcc_w[lookup.country][j]);
	      gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	    }
	  else
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	}
    }

  /* update WAC */
  if (preferences.awardswac == 1 && lookup.country != 0)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));
      iter_num[0]++;
      temp = g_strdup_printf ("WAC-%s", enum_to_cont (lookup.continent));
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, temp, -1);
      for (j = 0; j < MAX_BANDS; j++)
	{
	  if (wac_c[lookup.continent][j] > 0)
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	  else if (wac_w[lookup.continent][j] > 0)
	    {
	      temp = g_strdup_printf ("%d", wac_w[lookup.continent][j]);
	      gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	    }
	  else
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	}
    }

  /* update WAS */
  if (preferences.awardswas == 1 && st != 99)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));

      temp = g_strdup_printf ("WAS-%s", enum_to_state (st));
      iter_num[0]++;
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, temp, -1);
      for (j = 0; j < MAX_BANDS; j++)
	{
	  if (was_c[st][j] > 0)
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	  else if (was_w[st][j] > 0)
	    {
	      temp = g_strdup_printf ("%d", was_w[st][j]);
	      gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	    }
	  else
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	}
    }

  /* update WAZ */
  if (preferences.awardswaz == 1 && lookup.cq != 0)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));
      iter_num[0]++;
      temp = g_strdup_printf ("WAZ-%d", lookup.cq);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, temp, -1);
      for (j = 0; j < MAX_BANDS; j++)
	{
	  if (waz_c[lookup.cq-1][j] > 0)
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	  else if (waz_w[lookup.cq-1][j] > 0)
	    {
	      temp = g_strdup_printf ("%d", waz_w[lookup.cq-1][j]);
	      gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	    }
	  else
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	}
    }

  /* update IOTA */
  if (preferences.awardsiota == 1 && iota != NOT_AN_IOTA)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));

      gchar *iotastr = num_to_iota(iota);
      iter_num[0]++;

      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, iotastr, -1);
      for (j = 0; j < MAX_BANDS; j++)
	{
	  gpointer p_iota_w = g_hash_table_lookup (iota_w[j], iotastr);
	  gpointer p_iota_c = g_hash_table_lookup (iota_c[j], iotastr);

	  if (p_iota_c && GPOINTER_TO_INT(p_iota_c) > 0)
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	  else if (p_iota_w && GPOINTER_TO_INT(p_iota_w) > 0)
	    {
	      temp = g_strdup_printf ("%d", GPOINTER_TO_INT(p_iota_w));
	      gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	    }
	  else
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	}
      g_free(iotastr);
    }

  /* update Grid Locator */
  if (preferences.awardsloc == 1)
    {
      GtkWidget *locatorentry = lookup_widget (mainwindow, "locatorentry");
      gchar *locator = gtk_editable_get_chars (GTK_EDITABLE (locatorentry), 0, -1);
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));
      gchar *loc4 = loc_norm(locator);
      g_free (locator);

      if (loc4)
	{
	  temp = g_strdup_printf ("%s", loc4);
	  iter_num[0]++;

	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, temp, -1);
	  for (j = 0; j < MAX_BANDS; j++)
	    {
	      gpointer p_loc_w = g_hash_table_lookup (loc_w[j], loc4);
	      gpointer p_loc_c = g_hash_table_lookup (loc_c[j], loc4);

	      if (p_loc_c && GPOINTER_TO_INT(p_loc_c) > 0)
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "X", -1);
	      else if (p_loc_w && GPOINTER_TO_INT(p_loc_w) > 0)
		{
		  temp = g_strdup_printf ("%d", GPOINTER_TO_INT(p_loc_w));
		  gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
		}
	      else
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, "", -1);
	    }
	  g_free(loc4);
	}
    }

  /* Clear the rest of iter_num's */
  while (iter_num[0] < '0'+NB_AWARDS)
    {
      gtk_tree_model_get_iter (model, &iter, gtk_tree_path_new_from_string (iter_num));
      temp = g_strdup ("");
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, temp, -1);
      for (j = 0; j < MAX_BANDS; j++)
	{
	  gtk_list_store_set (GTK_LIST_STORE (model), &iter, j + 1, temp, -1);
	}
      iter_num[0]++;
    }

  if (g_strrstr(d->countryname, "&"))
    temp = my_strreplace (d->countryname, "&", "&amp;");
  else
    temp = g_strdup (d->countryname);

  /* override some items if there is area information available */
  /* TODO: if we add WAZ scoring we need to use this routine for scoring */
  /* TODO: ITU may contain strings like "07-08", right now it is an 'int' */
  if (!byprefix && (!strcmp(d->px, "K") || !strcmp(d->px, "VK")))
    {
      gchar callarea = lookuparea (item);
      if (callarea != '?')
	{
	  gint index;
	  gchar *areapx = g_strdup_printf ("%s%c", d->px, callarea);
	  for (index = 0; index < area->len; index++)
	    {
	      area_data *a = g_ptr_array_index (area, index);
	      if (g_ascii_strcasecmp (a->px, areapx) == 0)
		{
		  temp = g_strdup (a->countryname);
		  lookup.itu = a->itu;
		  lookup.cq = a->cq;
		  d->timezone = a->timezone;
		  d->latitude = a->latitude;
		  d->longitude = a->longitude;
		  break;
		}
	    }
	  g_free (areapx);
	}

    }

  labeltext1 = g_strdup_printf ("<b><i>%s</i></b>", temp);
  conttext = enum_to_cont (lookup.continent);
  itutext = g_strdup_printf ("ITU %d", lookup.itu);
  cqtext = g_strdup_printf ("CQ %d", lookup.cq);
  labeltext3 = g_strdup_printf (_("Timezone: %+1.1f"), (gdouble)d->timezone/10);

  /* to facilitate language translations */
  const char *north = _("N");
  const char *south = _("S");
  const char *east = _("E");
  const char *west = _("W");
  const char *loc = _("Location");

  if (d->latitude >= 0 && d->longitude >= 0)
    labeltext4 = g_strdup_printf ("%s: %1.2f%s %1.2f%s",
				  loc, (gdouble)d->latitude/100, north, (gdouble)d->longitude/100, west);
  else if (d->latitude >= 0 && d->longitude < 0)
    labeltext4 = g_strdup_printf ("%s: %1.2f%s %1.2f%s",
				  loc, (gdouble)d->latitude/100, north, -1 * (gdouble)d->longitude/100, east);
  else if (d->latitude < 0 && d->longitude >= 0)
    labeltext4 = g_strdup_printf ("%s: %1.2f%s %1.2f%s",
				  loc, -1 * (gdouble)d->latitude/100, south, (gdouble)d->longitude/100, west);
  else
    labeltext4 = g_strdup_printf ("%s: %1.2f%s %1.2f%s",
				  loc, -1 * (gdouble)d->latitude/100, south, -1 * (gdouble)d->longitude/100, east);
	
  if (lookup.country > 0)
    {
      if (preferences.NS == 1)
	lat = -1.0 * preferences.latitude;
      else
	lat = preferences.latitude;
      if (preferences.EW == 1)
	lon = -1.0 * preferences.longitude;
      else
	lon = preferences.longitude;

      gcresult = g_strdup_printf ("<small>%s</small>",
				  gcircle (preferences.units, lat, lon,
					   (gdouble)d->latitude/100, (gdouble)d->longitude/100));
    }
  gtk_label_set_markup (GTK_LABEL (dxcclabel1), labeltext1);
  gtk_label_set_text (GTK_LABEL (contlabel), conttext);
  gtk_label_set_text (GTK_LABEL (itulabel), itutext);
  gtk_label_set_text (GTK_LABEL (cqlabel), cqtext);
  gtk_label_set_text (GTK_LABEL (dxcclabel3), labeltext3);
  gtk_label_set_text (GTK_LABEL (dxcclabel4), labeltext4);
  gtk_label_set_markup (GTK_LABEL (dxcclabel5), gcresult);
  g_free (labeltext1);
  g_free (conttext);
  g_free (itutext);
  g_free (cqtext);
  g_free (labeltext3);
  g_free (labeltext4);
  g_free (gcresult);
  g_free (temp);
}

void update_dxccscoring (void)
{
  gint i, j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *dxcctreeview;
  GtkTreeModel *dxccmodel;
  GtkTreeIter dxcciter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (i = 0; i <= programstate.countries; i++)
    {
      for (j = 0; j <= MAX_BANDS; j++)
	{
	  if (dxcc_w[i][j] > 0) worked[j]++;
	  if (dxcc_c[i][j] > 0) confirmed[j]++;
	}
    }
  dxcctreeview = lookup_widget (scorewindow, "dxcctreeview");
  dxccmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (dxcctreeview));
  gtk_tree_model_get_iter (dxccmodel, &dxcciter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (dxccmodel), &dxcciter, j, str, -1);
    }
  g_free (str);
}

void update_wacscoring (void)
{
  gint i, j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *wactreeview;
  GtkTreeModel *wacmodel;
  GtkTreeIter waciter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (i = 0; i < MAX_CONTINENTS; i++)
    {
      for (j = 0; j <= MAX_BANDS; j++)
	{
	  if (wac_w[i][j] != 0) worked[j]++;
	  if (wac_c[i][j] != 0) confirmed[j]++;
	}
    }
  wactreeview = lookup_widget (scorewindow, "wactreeview");
  wacmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (wactreeview));
  gtk_tree_model_get_iter (wacmodel, &waciter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (wacmodel), &waciter, j, str, -1);
    }
  g_free (str);
}

void update_wasscoring (void)
{
  gint i, j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *wastreeview;
  GtkTreeModel *wasmodel;
  GtkTreeIter wasiter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (i = 0; i < MAX_STATES; i++)
    {
      for (j = 0; j <= MAX_BANDS; j++)
	{
	  if (was_w[i][j] != 0) worked[j]++;
	  if (was_c[i][j] != 0) confirmed[j]++;
	}
    }
  wastreeview = lookup_widget (scorewindow, "wastreeview");
  wasmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (wastreeview));
  gtk_tree_model_get_iter (wasmodel, &wasiter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (wasmodel), &wasiter, j, str, -1);
    }
  g_free (str);
}

void update_wazscoring (void)
{
  gint i, j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *waztreeview;
  GtkTreeModel *wazmodel;
  GtkTreeIter waziter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (i = 0; i < MAX_ZONES; i++)
    {
      for (j = 0; j <= MAX_BANDS; j++)
	{
	  if (waz_w[i][j] != 0) worked[j]++;
	  if (waz_c[i][j] != 0) confirmed[j]++;
	}
    }
  waztreeview = lookup_widget (scorewindow, "waztreeview");
  wazmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (waztreeview));
  gtk_tree_model_get_iter (wazmodel, &waziter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (wazmodel), &waziter, j, str, -1);
    }
  g_free (str);
}

void update_iotascoring (void)
{
  gint j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *iotatreeview;
  GtkTreeModel *iotamodel;
  GtkTreeIter iotaiter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = g_hash_table_size(iota_w[j]);
      confirmed[j] = g_hash_table_size(iota_c[j]);
    }

  iotatreeview = lookup_widget (scorewindow, "iotatreeview");
  iotamodel = gtk_tree_view_get_model (GTK_TREE_VIEW (iotatreeview));
  gtk_tree_model_get_iter (iotamodel, &iotaiter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (iotamodel), &iotaiter, j, str, -1);
    }
  g_free (str);
}


#if 0
static void loc_gh_add (gpointer key, gpointer value, gpointer user_data)
{
  gint *p_counter = (gint *)user_data;

  if (value && GPOINTER_TO_INT(value))
    (*p_counter) ++;
}
#endif

void update_locscoring (void)
{
  gint j, worked[MAX_BANDS + 1], confirmed[MAX_BANDS + 1];
  GtkWidget *loctreeview;
  GtkTreeModel *locmodel;
  GtkTreeIter lociter;
  gchar *str = g_strdup ("");;

  for (j = 0; j <= MAX_BANDS; j++)
    {
      worked[j] = 0;
      confirmed[j] = 0;
    }
  for (j = 0; j <= MAX_BANDS; j++)
    {
#if 0
      g_hash_table_foreach (loc_w[j], &loc_gh_add, (gpointer)&worked[j]);
      g_hash_table_foreach (loc_c[j], &loc_gh_add, (gpointer)&confirmed[j]);
#else
      worked[j] = g_hash_table_size(loc_w[j]);
      confirmed[j] = g_hash_table_size(loc_c[j]);
#endif
    }

  loctreeview = lookup_widget (scorewindow, "loctreeview");
  locmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (loctreeview));
  gtk_tree_model_get_iter (locmodel, &lociter, gtk_tree_path_new_from_string ("0"));
  for (j = 0; j <= MAX_BANDS; j++)
    {
      str = g_strdup_printf ("%d\n%d", worked[j], confirmed[j]);
      gtk_list_store_set (GTK_LIST_STORE (locmodel), &lociter, j, str, -1);
    }
  g_free (str);
}

void hash_inc(GHashTable *hash_table, const gchar *key)
{
  gpointer p, value;
  p = g_hash_table_lookup (hash_table, key);
  if (!p)
    value = GINT_TO_POINTER (1);
  else
    value = GINT_TO_POINTER(GPOINTER_TO_INT(p) + 1);

  g_hash_table_insert (hash_table, g_strdup(key), value);

}

void hash_dec(GHashTable *hash_table, const gchar *key)
{
  gpointer p, value;

  p = g_hash_table_lookup (hash_table, key);
  if (!p)
    return;

  value = GINT_TO_POINTER(GPOINTER_TO_INT(p) - 1);
  if (value > 0)
    g_hash_table_insert (hash_table, g_strdup(key), value);
  else
    g_hash_table_remove (hash_table, key);

}

gchar *loc_norm(const gchar *locator)
{
  gchar *loc4;

  if (!locator || strlen(locator) < 4)
    return NULL;

  /* Make all locators uppercase - no dupe,
   * and keep only first 4 chars
   */
  loc4 = g_ascii_strup(locator, -1);
  loc4[4] = '\0';

  return loc4;
}

void loc_new_qso(const gchar *locator, gint f, gboolean qslconfirmed)
{
  gchar *loc4;

  if (!locator || strlen(locator) < 4) return;
  loc4 = loc_norm(locator);
  if (!loc4) return;

  hash_inc(loc_w[f], loc4);
  hash_inc(loc_w[MAX_BANDS], loc4);

  if (qslconfirmed)
    {
      hash_inc(loc_c[f], loc4);
      hash_inc(loc_c[MAX_BANDS], loc4);
    }

  g_free(loc4);
}

void loc_del_qso(const gchar *locator, gint f, gboolean qslconfirmed)
{
  gchar *loc4;

  if (!locator || strlen(locator) < 4) return;
  loc4 = loc_norm(locator);
  if (!loc4) return;

  hash_dec(loc_w[f], loc4);
  hash_dec(loc_w[MAX_BANDS], loc4);

  if (qslconfirmed)
    {
      hash_dec(loc_c[f], loc4);
      hash_dec(loc_c[MAX_BANDS], loc4);
    }
}

void iota_new_qso(guint iota, gint f, gboolean qslconfirmed)
{
  gchar *iotastr;

  iotastr = num_to_iota(iota);
  if (!iotastr)
    return;

  hash_inc(iota_w[f], iotastr);
  hash_inc(iota_w[MAX_BANDS], iotastr);

  if (qslconfirmed)
    {
      hash_inc(iota_c[f], iotastr);
      hash_inc(iota_c[MAX_BANDS], iotastr);
    }

  g_free(iotastr);
}

void iota_del_qso(guint iota, gint f, gboolean qslconfirmed)
{
  gchar *iotastr;

  iotastr = num_to_iota(iota);
  if (!iotastr)
    return;

  hash_dec(iota_w[f], iotastr);
  hash_dec(iota_w[MAX_BANDS], iotastr);

  if (qslconfirmed)
    {
      hash_dec(iota_c[f], iotastr);
      hash_dec(iota_c[MAX_BANDS], iotastr);
    }
}

static void init_scoring (void)
{
  gint i, j;

  for (i = 0; i <= programstate.countries; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	dxcc_w[i][j] = 0;
	dxcc_c[i][j] = 0;
      }
  for (i = 0; i < MAX_CONTINENTS; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	wac_w[i][j] = 0;
	wac_c[i][j] = 0;
      }
  for (i = 0; i < MAX_STATES; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	was_w[i][j] = 0;
	was_c[i][j] = 0;
      }
  for (i = 0; i < MAX_ZONES; i++)
    for (j = 0; j <= MAX_BANDS; j++)
      {
	waz_w[i][j] = 0;
	waz_c[i][j] = 0;
      }
  for (j = 0; j <= MAX_BANDS; j++)
    {
      iota_w[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      iota_c[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    }
  for (j = 0; j <= MAX_BANDS; j++)
    {
      loc_w[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
      loc_c[j] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    }
}

void fill_scoring_arrays (void)
{
  gint i, f;
  guint st, zone, cont, iota;
  gboolean valid, qslconfirmed;
  logtype *logw;
  GtkTreeModel *logmodel;
  GtkTreeIter logiter;
  gchar *call, *freq, *qslin, *awstr, *locator;
  struct info lookup;


  init_scoring ();
  for (i = 0; i < g_list_length (logwindowlist); i++)
    {
      logw = g_list_nth_data (logwindowlist, i);
      logmodel = gtk_tree_view_get_model (GTK_TREE_VIEW(logw -> treeview));
      valid = gtk_tree_model_get_iter_first (logmodel, &logiter);
      while (valid)
	{
	  gtk_tree_model_get (logmodel, &logiter, CALL, &call, -1);
	  gtk_tree_model_get (logmodel, &logiter, BAND, &freq, -1);
	  gtk_tree_model_get (logmodel, &logiter, QSLIN, &qslin, -1);
	  gtk_tree_model_get (logmodel, &logiter, AWARDS, &awstr, -1);
	  gtk_tree_model_get (logmodel, &logiter, LOCATOR, &locator, -1);
	  f = freq2enum (freq);
	  gchar *result = valid_awards_entry (awstr, &st, &zone, &cont, &iota);
	  if (result)
	    {
	      lookup = lookupcountry_by_prefix (result);
	      g_free (result);
	    }
	  else
	    lookup = lookupcountry_by_callsign (call);

	  if (f >= 0 && preferences.scoringbands[f] == 1)
	    {
	      qslconfirmed = qslreceived (qslin);
	      if (lookup.country > 0)
		{
		  dxcc_w[lookup.country][f]++;
		  dxcc_w[lookup.country][MAX_BANDS]++;
		  if (qslconfirmed)
		    {
		      dxcc_c[lookup.country][f]++;
		      dxcc_c[lookup.country][MAX_BANDS]++;
		    }
		}
	      if (cont != 99 || lookup.continent != 99)
		{
		  if (cont == 99) cont = lookup.continent;
		  wac_w[cont][f]++;
		  wac_w[cont][MAX_BANDS]++;
		  if (qslconfirmed)
		    {
		      wac_c[cont][f]++;
		      wac_c[cont][MAX_BANDS]++;
		    }
		}
	      if (st != 99)
		{
		  was_w[st][f]++;
		  was_w[st][MAX_BANDS]++;
		  if (qslconfirmed)
		    {
		      was_c[st][f]++;
		      was_c[st][MAX_BANDS]++;
		    }
		}
	      if ((zone > 0 && zone < 99) || (lookup.cq > 0 && lookup.cq < 99))
		{
		  if (zone == 99) zone = lookup.cq;
		  waz_w[zone-1][f]++;
		  waz_w[zone-1][MAX_BANDS]++;
		  if (qslconfirmed)
		    {
		      waz_c[zone-1][f]++;
		      waz_c[zone-1][MAX_BANDS]++;
		    }
		}
	      if (iota != NOT_AN_IOTA)
		{
		  iota_new_qso(iota, f, qslconfirmed);
		}
	      if (strlen(locator) > 0)
		{
		  loc_new_qso(locator, f, qslconfirmed);
		}
	    }
	  valid = gtk_tree_model_iter_next (logmodel, &logiter);
	}
    }
}
