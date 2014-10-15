/*
FRACTAL - A program using drawing fractals to benchmark parallelization and
drawing libraries.

Copyright 2009-2013, Javier Burguete Tolosa.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Javier Burguete Tolosa ``AS IS'' AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL Javier Burguete Tolosa OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * \file simulator.c
 * \brief Source file to define the windows data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2013, Javier Burguete Tolosa.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <libintl.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <GL/freeglut.h>
#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

/**
 * \fn void set_perspective()
 * \brief Function to set the view perspective.
 * \param
 */
void set_perspective()
{
	float k1, k2;
	phid = gtk_range_get_value(GTK_RANGE(dialog_simulator.hscale));
	thetad = gtk_range_get_value(GTK_RANGE(dialog_simulator.vscale));
	phi = phid * M_PI / 180.;
	theta = thetad * M_PI / 180.;
	if (!fractal_3D)
	{
		k1 = width;
		k2 = height;
	}
	else
	{
		perspective(0, 0, 0, &k1, &k2);
		xmin = k1;
		perspective(length, width, 0, &k1, &k2);
		xmax = k1;
		perspective(length, 0, 0, &k1, &k2);
		ymin = k2;
		perspective(0, width, height, &k1, &k2);
		ymax = k2;
		k1 = xmax - xmin;
		k2 = ymax - ymin;
	}
	draw();
}

/**
 * \fn void dialog_draw_save()
 * \brief Function to show a dialog to save the graphical view in a PNG file.
 * \param
 */
void dialog_draw_save()
{
	char *buffer = NULL;
	GtkFileFilter *filter;
	GtkFileChooserDialog *dlg;
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "PNG file");
	gtk_file_filter_add_pattern(filter, "*.png");
	gtk_file_filter_add_pattern(filter, "*.PNG");
	dlg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new
		(gettext("Save graphical"), dialog_simulator.window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dlg), filter);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg),1);
	if (gtk_dialog_run((GtkDialog*)dlg) == GTK_RESPONSE_OK)
		buffer = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	gtk_widget_destroy(GTK_WIDGET(dlg));
	if (buffer)
	{
		draw();
		while (gtk_events_pending()) gtk_main_iteration();
		draw_save(buffer);
		g_free(buffer);
	}
}

/**
 * \fn void dialog_options_update()
 * \brief Function to update the enabled fractal characteristics.
 * \param
 */
void dialog_options_update()
{
	int i;
	DialogOptions *dlg = &dialog_options;
	i = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->button_3D));
	gtk_widget_set_sensitive(GTK_WIDGET(dlg->label_length), i);
	gtk_widget_set_sensitive(GTK_WIDGET(dlg->entry_length), i);
}

/**
 * \fn void dialog_options_create()
 * \brief Function to create a dialog to set the fractal options.
 * \param
 */
void dialog_options_create()
{
	int i;
	const char *array_char[3]=
		{gettext("_Tree"), gettext("_Forest"), gettext("_Neuron")};
	DialogOptions *dlg = &dialog_options;

	dlg->button_diagonal = (GtkCheckButton*)gtk_check_button_new_with_mnemonic
		(gettext("_Diagonal movement"));
	dlg->button_3D = (GtkCheckButton*)gtk_check_button_new_with_mnemonic("3_D");
	dlg->label_length = (GtkLabel*)gtk_label_new(gettext("Length"));
	dlg->label_width = (GtkLabel*)gtk_label_new(gettext("Width"));
	dlg->label_height = (GtkLabel*)gtk_label_new(gettext("Height"));
	dlg->entry_length =
		(GtkSpinButton*)gtk_spin_button_new_with_range(320., 2400., 1.);
	dlg->entry_width =
		(GtkSpinButton*)gtk_spin_button_new_with_range(320., 2400., 1.);
	dlg->entry_height =
		(GtkSpinButton*)gtk_spin_button_new_with_range(200., 2400., 1.);

	dlg->box_fractal = (GtkVBox*)gtk_vbox_new(1, 0);
	dlg->array_buttons[0] = NULL;
	for (i = 0; i < 3; ++i)
	{
		dlg->array_buttons[i] =
			(GtkRadioButton*)gtk_radio_button_new_with_mnemonic_from_widget
				(dlg->array_buttons[0], array_char[i]);
		gtk_container_add(GTK_CONTAINER(dlg->box_fractal),
			GTK_WIDGET(dlg->array_buttons[i]));
	}
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(dlg->array_buttons[fractal_type]), 1);

	dlg->frame_fractal = (GtkFrame*)gtk_frame_new(gettext("Fractal type"));
	gtk_container_add(GTK_CONTAINER(dlg->frame_fractal),
		GTK_WIDGET(dlg->box_fractal));

	dlg->table = (GtkTable*)gtk_table_new(0, 0, 0);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->button_diagonal), 0, 2, 0, 1);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->button_3D), 0, 2, 1, 2);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_length), 0, 1, 2, 3);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->entry_length), 1, 2, 2, 3);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_width), 0, 1, 3, 4);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->entry_width), 1, 2, 3, 4);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_height), 0, 1, 4, 5);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->entry_height), 1, 2, 4, 5);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->frame_fractal), 0, 2, 5, 6);

	dlg->dialog = (GtkDialog*)gtk_dialog_new_with_buttons(
		gettext("Options"),
		dialog_simulator.window,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(dlg->dialog)),
		GTK_WIDGET(dlg->table));
	gtk_widget_show_all(GTK_WIDGET(dlg->dialog));

	g_signal_connect(dlg->button_3D, "clicked", dialog_options_update, NULL);
	gtk_toggle_button_set_active
		((GtkToggleButton*)dlg->button_diagonal, fractal_diagonal);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->button_3D), fractal_3D);
	gtk_spin_button_set_value(dlg->entry_length, length);
	gtk_spin_button_set_value(dlg->entry_width, width);
	gtk_spin_button_set_value(dlg->entry_height, height);
	dialog_options_update();

	if (gtk_dialog_run(dlg->dialog) == GTK_RESPONSE_OK)
	{
		fractal_diagonal = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(dlg->button_diagonal));
		fractal_3D =
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->button_3D));
		for (i = 0; i < 3; ++i)
			if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(dlg->array_buttons[i])))
					fractal_type = i;
		length = gtk_spin_button_get_value_as_int(dlg->entry_length);
		width = gtk_spin_button_get_value_as_int(dlg->entry_width);
		height = gtk_spin_button_get_value_as_int(dlg->entry_height);
		medium_start();
		set_perspective();
		breaking = 1;
	}
	gtk_widget_destroy(GTK_WIDGET(dlg->dialog));
}

/**
 * \fn void dialog_simulator_help()
 * \brief Function to show a help dialog.
 * \param
 */
void dialog_simulator_help()
{
	gchar *authors[] =
	{
		"Javier Burguete Tolosa (jburguete@eead.csic.es)",
		NULL
	};
	gtk_show_about_dialog(dialog_simulator.window,
		"program_name",
		"Fractal",
		"authors",
		authors,
		"version",
		"2.2.0",
		"copyright",
		gettext("Copyright 2009-2013 Javier Burguete Tolosa."),
		"logo",
		dialog_simulator.logo,
		"website-label",
		"Website",
		"website",
		"http://www.eead.csic.es",
		NULL);
}

/**
 * \fn void dialog_simulator_update()
 * \brief Function to update the enabled window properties.
 * \param
 */
void dialog_simulator_update()
{
	gtk_widget_set_sensitive
		(GTK_WIDGET(dialog_simulator.item_start), !simulating);
	gtk_widget_set_sensitive
		(GTK_WIDGET(dialog_simulator.item_stop), simulating);
}

/**
 * \fn void dialog_simulator_progress()
 * \brief Function to show the fractal progress.
 * \param
 */
void dialog_simulator_progress()
{
	register int k;
	register float x;
	if (fractal_3D)
	{
		switch (fractal_type)
		{
		case 0:
		case 1:
			x = max_d / (float)(height - 1);
			break;
		default:
			k = length;
			if (width < k) k = width;
			if (height < k) k = height;
			k = k / 2 - 1;
			x = fmin(1., max_d / (float)k);
		}
	}
	else
	{
		switch (fractal_type)
		{
		case 0:
		case 1:
			x = max_d / (float)(height - 1);
			break;
		default:
			k = width;
			if (height < k) k = height;
			k = k / 2 - 1;
			x = fmin(1., max_d / (float)k);
		}
	}
	gtk_progress_bar_set_fraction(dialog_simulator.progress, x);
	gtk_spin_button_set_value
		(dialog_simulator.entry_time, difftime(time(NULL), t0));
	while (gtk_events_pending()) gtk_main_iteration();
}

/**
 * \fn void dialog_simulator_save()
 * \brief Function to save the graphical view on a PNG file.
 * \param
 */
void dialog_simulator_save()
{
	char *filename = NULL;
	GtkFileChooserDialog *dlg;
	dlg = (GtkFileChooserDialog*)gtk_file_chooser_dialog_new(
		gettext("Save graphical"), dialog_simulator.window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), 1);
	if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
	gtk_widget_destroy(GTK_WIDGET(dlg));
	if (filename)
	{
		draw_save(filename);
		g_free(filename);
	}
}

/**
 * \fn void dialog_simulator_create()
 * \brief Function to create the main window.
 * \param
 */
void dialog_simulator_create()
{
	static char *str_exit, *str_options, *str_start, *str_stop, *str_save,
		*str_help;
	DialogSimulator *dlg = &dialog_simulator;

	str_exit = gettext("E_xit");
	str_options = gettext("_Options");
	str_start = gettext("S_tart");
	str_stop = gettext("Sto_p");
	str_save = gettext("_Save");
	str_help = gettext("_Help");

	dlg->toolbar = (GtkToolbar*)gtk_toolbar_new();
	dlg->action_exit =
		(GtkAction*)gtk_action_new(str_exit, str_exit, NULL, GTK_STOCK_QUIT);
	dlg->item_exit =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_exit);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_exit, -1);
	g_signal_connect(dlg->action_exit, "activate", glutLeaveMainLoop, NULL);

	dlg->action_options = (GtkAction*)gtk_action_new
		(str_options, str_options, NULL, GTK_STOCK_PREFERENCES);
	dlg->item_options =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_options);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_options, -1);
	g_signal_connect
		(dlg->action_options, "activate", dialog_options_create, NULL);

	dlg->action_start = (GtkAction*)gtk_action_new
		(str_start, str_start, NULL, GTK_STOCK_EXECUTE);
	dlg->item_start =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_start);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_start, -1);
	g_signal_connect(dlg->action_start, "activate", fractal, NULL);

	dlg->action_stop = (GtkAction*)gtk_action_new
		(str_stop, str_stop, NULL, GTK_STOCK_STOP);
	dlg->item_stop =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_stop);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_stop, -1);
	g_signal_connect(dlg->action_stop, "activate", fractal_stop, NULL);

	dlg->action_save = (GtkAction*)gtk_action_new
		(str_save, str_save, NULL, GTK_STOCK_SAVE);
	dlg->item_save =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_save);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_save, -1);
	g_signal_connect(dlg->action_save, "activate", dialog_simulator_save, NULL);

	dlg->action_help =
		(GtkAction*)gtk_action_new(str_help, str_help, NULL, GTK_STOCK_HELP);
	dlg->item_help =
		(GtkToolItem*)gtk_action_create_tool_item(dlg->action_help);
	gtk_toolbar_insert(dlg->toolbar, dlg->item_help, -1);
	g_signal_connect(dlg->action_help, "activate", dialog_simulator_help, NULL);

	dlg->label_time = (GtkLabel*)gtk_label_new(gettext("Calculating time"));
	dlg->entry_time =
		(GtkSpinButton*)gtk_spin_button_new_with_range(0., 1.e6, 0.1);
	gtk_widget_set_sensitive(GTK_WIDGET(dlg->entry_time), 0);

	dlg->progress = (GtkProgressBar*)gtk_progress_bar_new();
	gtk_progress_bar_set_text(dlg->progress, gettext("Progress"));

	dlg->hscale = (GtkHScale*)gtk_hscale_new_with_range(-90., 0., 1.);
	dlg->vscale = (GtkHScale*)gtk_hscale_new_with_range(0., 90., 1.);
	gtk_scale_set_digits(GTK_SCALE(dlg->hscale), 0);
	gtk_scale_set_digits(GTK_SCALE(dlg->vscale), 0);
	gtk_range_set_value(GTK_RANGE(dlg->hscale), phid);
	gtk_range_set_value(GTK_RANGE(dlg->vscale), thetad);
	g_signal_connect(dlg->hscale, "value-changed", set_perspective, NULL);
	g_signal_connect(dlg->vscale, "value-changed", set_perspective, NULL);

	dlg->label_horizontal = (GtkLabel*)gtk_label_new
		(gettext("Horizontal perspective angle (º)"));
	dlg->label_vertical = (GtkLabel*)gtk_label_new
		(gettext("Vertical perspective angle (º)"));

	dlg->table=(GtkVBox*)gtk_table_new(0, 0, 0);
	gtk_table_attach_defaults(dlg->table, GTK_WIDGET(dlg->toolbar), 0, 3, 0, 1);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->progress), 0, 1, 1, 2);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_time), 1, 2, 1, 2);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->entry_time), 2, 3, 1, 2);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_horizontal), 0, 1, 2, 3);
	gtk_table_attach_defaults(dlg->table, GTK_WIDGET(dlg->hscale), 1, 3, 2, 3);
	gtk_table_attach_defaults
		(dlg->table, GTK_WIDGET(dlg->label_vertical), 0, 1, 3, 4);
	gtk_table_attach_defaults(dlg->table, GTK_WIDGET(dlg->vscale), 1, 3, 3, 4);

	dlg->logo = gtk_image_get_pixbuf
		(GTK_IMAGE(gtk_image_new_from_file("logo.png")));
	dlg->logo_min = gtk_image_get_pixbuf
		(GTK_IMAGE(gtk_image_new_from_file("logo2.png")));

	dlg->window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(dlg->window, gettext("Fractal growing"));
	gtk_window_set_icon(dlg->window, dlg->logo_min);
	gtk_container_add(GTK_CONTAINER(dlg->window), GTK_WIDGET(dlg->table));
	gtk_widget_show_all(GTK_WIDGET(dlg->window));
	g_signal_connect(dlg->window, "delete_event", glutLeaveMainLoop, NULL);

	set_perspective();
	dialog_simulator_update();
}
