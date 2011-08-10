/* $Id$ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contact addresses for paper mail and Email:
 *  Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
 *  Thomas.Nau@rz.uni-ulm.de
 *
 */

/* This file written by Bill Wilson for the PCB Gtk port. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "global.h"

#include "change.h"
#include "crosshair.h"
#include "data.h"
#include "error.h"
#include "misc.h"
#include "set.h"
#include "pcb-printf.h"

#include "gui.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID ("$Id$");

typedef struct
{
  GtkWidget *name_entry,
    *line_width_coord_entry,
    *via_hole_coord_entry,
    *via_size_coord_entry,
    *clearance_coord_entry, *set_temp1_button, *set_temp2_button;
}
SizesDialog;

static SizesDialog route_sizes;

static gchar *
make_route_string(RouteStyleType * rs)
{
  gchar *str, *s, *t, *colon;
  gint i;

  str = g_strdup("");
  for (i = 0; i < NUM_STYLES; ++i, ++rs)
    {
      s = pcb_g_strdup_printf ("%s,%mc,%mc,%mc,%mc", rs->Name,
               rs->Thick, rs->Diameter, rs->Hole, rs->Keepaway);
      colon = (i == NUM_STYLES - 1) ? NULL : (gchar *)":";
      t = str;
      str = g_strconcat (str, s, colon, NULL);
      g_free (t);
	}
  return str;
}

static void
via_hole_cb (GtkPcbCoordEntry * entry, gpointer data)
{
  SizesDialog * sd = (SizesDialog *)data;
  gdouble via_hole_size, via_size;

  via_hole_size = gtk_pcb_coord_entry_get_value (entry);
  via_size = gtk_pcb_coord_entry_get_value
               (GTK_PCB_COORD_ENTRY (sd->via_size_coord_entry));

  if (via_size < via_hole_size + MIN_PINORVIACOPPER)
    gtk_pcb_coord_entry_set_value (GTK_PCB_COORD_ENTRY (sd->via_size_coord_entry),
			           via_hole_size + MIN_PINORVIACOPPER);
}

static void
via_size_cb (GtkPcbCoordEntry * entry, gpointer data)
{
  SizesDialog * sd = (SizesDialog *)data;
  gdouble via_hole_size, via_size;

  via_size = gtk_pcb_coord_entry_get_value (entry);
  via_hole_size = gtk_pcb_coord_entry_get_value
                    (GTK_PCB_COORD_ENTRY (sd->via_hole_coord_entry));

  if (via_hole_size > via_size - MIN_PINORVIACOPPER)
    gtk_pcb_coord_entry_set_value (GTK_PCB_COORD_ENTRY (sd->via_hole_coord_entry),
			           via_size - MIN_PINORVIACOPPER);
}

static void
use_temp_cb (GtkToggleButton * button, gpointer data)
{
  gint which = GPOINTER_TO_INT (data);
  gboolean active;

  active = gtk_toggle_button_get_active (button);
  if (which == 1 && active)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (route_sizes.set_temp2_button), FALSE);
  else if (which == 2 && active)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (route_sizes.set_temp1_button), FALSE);
}


  /* -----------------------------------------------------------------------
     |  style sizes dialog
   */
void
ghid_route_style_dialog (gint index, RouteStyleType * temp_rst)
{
  GtkWidget *dialog, *table, *vbox, *vbox1, *hbox, *label;
  GtkWidget *set_default_button = NULL;
  GHidPort *out = &ghid_port;
  RouteStyleType *rst;
  SizesDialog *sd;
  gchar *s, buf[64];
  gboolean set_temp1 = FALSE, set_temp2 = FALSE;
  gboolean editing_temp, set_default = FALSE;

  sd = &route_sizes;

  editing_temp = (index >= NUM_STYLES);
  rst = editing_temp ? temp_rst : &PCB->RouteStyle[index];

  if (!rst)
    return;

  snprintf (buf, sizeof (buf), _("%s Sizes"), rst->Name);
  dialog = gtk_dialog_new_with_buttons (buf,
					GTK_WINDOW (out->top_window),
					(GtkDialogFlags)(GTK_DIALOG_MODAL |
							 GTK_DIALOG_DESTROY_WITH_PARENT),
					GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
					GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  gtk_window_set_wmclass (GTK_WINDOW (dialog), "Sizes_dialog", "PCB");

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), vbox);

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);
  label = gtk_label_new (_("Route style name"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  sd->name_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), sd->name_entry, FALSE, FALSE, 0);

  vbox1 = ghid_category_vbox (vbox, _("Sizes"), 4, 2, TRUE, TRUE);
  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);

  ghid_table_coord_entry (table, 0, 0, &sd->line_width_coord_entry,
			  rst->Thick, MIN_LINESIZE, MAX_LINESIZE,
			  CE_SMALL, 0, NULL, sd, TRUE,
			  _("Line width"));
  ghid_table_coord_entry (table, 1, 0, &sd->via_hole_coord_entry,
			  rst->Hole, MIN_PINORVIAHOLE,
			  MAX_PINORVIASIZE - MIN_PINORVIACOPPER,
			  CE_SMALL, 0, via_hole_cb, sd, TRUE,
			  _("Via hole"));
  ghid_table_coord_entry (table, 2, 0, &sd->via_size_coord_entry,
			  rst->Diameter, MIN_PINORVIAHOLE + MIN_PINORVIACOPPER,
			  MAX_PINORVIASIZE,
			  CE_SMALL, 0, via_size_cb, sd, TRUE,
			  _("Via size"));
  ghid_table_coord_entry (table, 3, 0, &sd->clearance_coord_entry,
			  rst->Keepaway, MIN_LINESIZE, MAX_LINESIZE,
			  CE_SMALL, 0, NULL, sd, true,
			  _("Clearance"));
  gtk_box_pack_start (GTK_BOX (vbox1), table, FALSE, FALSE, 0);

  if (!editing_temp)
    {
      vbox1 = ghid_category_vbox (vbox, _("Temporary Styles"),
				  4, 2, TRUE, TRUE);
      label = gtk_label_new ("");
      gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
      s = g_strdup_printf (_("<small>"
			     "Use values in a temporary route style instead of <b>%s</b>."
			     "</small>"), rst->Name);
      gtk_label_set_markup (GTK_LABEL (label), s);
      g_free (s);
      gtk_box_pack_start (GTK_BOX (vbox1), label, FALSE, FALSE, 0);

      ghid_check_button_connected (vbox1, &sd->set_temp1_button, FALSE,
				   TRUE, FALSE, FALSE, 0,
				   use_temp_cb, GINT_TO_POINTER (1),
				   _("Temp1"));
      ghid_check_button_connected (vbox1, &sd->set_temp2_button, FALSE, TRUE,
				   FALSE, FALSE, 0, use_temp_cb,
				   GINT_TO_POINTER (2), _("Temp2"));

      vbox1 = ghid_category_vbox (vbox, _("Default Style"), 4, 2, TRUE, TRUE);
      label = gtk_label_new ("");
      gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
      s = g_strdup_printf (_("<small>"
			     "Use values as the default route style for new layouts."
			     "</small>"));
      gtk_label_set_markup (GTK_LABEL (label), s);
      g_free (s);
      gtk_box_pack_start (GTK_BOX (vbox1), label, FALSE, FALSE, 0);
      ghid_check_button_connected (vbox1, &set_default_button, FALSE,
				   TRUE, FALSE, FALSE, 0,
				   NULL, NULL, _("Set as default"));

    }

  gtk_entry_set_text (GTK_ENTRY (sd->name_entry), rst->Name);
  if (editing_temp)
    gtk_editable_set_editable (GTK_EDITABLE (sd->name_entry), FALSE);

  gtk_widget_show_all (dialog);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      gchar *string;
      RouteStyleType rst_buf;

      if (!editing_temp)
	{
	  set_temp1 =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (sd->set_temp1_button));
	  set_temp2 =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (sd->set_temp2_button));
	  set_default =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (set_default_button));
	}
      if (set_temp1 || set_temp2)
	rst = &rst_buf;

      rst->Thick =
	gtk_pcb_coord_entry_get_value (GTK_PCB_COORD_ENTRY
				        (sd->line_width_coord_entry));
      rst->Hole =
	gtk_pcb_coord_entry_get_value (GTK_PCB_COORD_ENTRY
				        (sd->via_hole_coord_entry));
      rst->Diameter =
	gtk_pcb_coord_entry_get_value (GTK_PCB_COORD_ENTRY
				        (sd->via_size_coord_entry));
      rst->Keepaway =
	gtk_pcb_coord_entry_get_value (GTK_PCB_COORD_ENTRY
				        (sd->clearance_coord_entry));

      if (index < NUM_STYLES && !set_temp1 && !set_temp2)
	{
	  string = ghid_entry_get_text (sd->name_entry);
	  free (rst->Name);
	  rst->Name = StripWhiteSpaceAndDup (string);
	  pcb_use_route_style (rst);
	  SetChangedFlag (true);
	  ghid_route_style_set_button_label (rst->Name, index);
	}
      else
	{
	  pcb_use_route_style (rst);
	  ghid_route_style_set_temp_style (rst, set_temp1 ? 0 :
					   (set_temp2 ? 1 : index -
					    NUM_STYLES));
	}
      if (set_default)
	{
	  gchar *s;

	  Settings.RouteStyle[index] = *rst;
	  ghidgui->config_modified = TRUE;
	  s = make_route_string (&Settings.RouteStyle[0]);
	  g_free (Settings.Routes);
	  Settings.Routes = s;
	}
    }

  gtk_widget_destroy (dialog);
  ghid_window_set_name_label (PCB->Name);
  ghid_set_status_line_label ();
}
