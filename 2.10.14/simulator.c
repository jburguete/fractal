/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2018, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2018, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <libintl.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gtk/gtk.h>

// Enabling OpenGL containers
#include <GL/glew.h>
#if HAVE_FREEGLUT
#include <GL/freeglut.h>
#elif HAVE_SDL
#include <SDL.h>
#elif HAVE_GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow *window;
#endif

#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

#if HAVE_SDL
SDL_Event exit_event[1];
#endif

/**
 * Function to set the view perspective.
 */
void
set_perspective ()
{
  float k1, k2;
  phid = gtk_range_get_value (GTK_RANGE (dialog_simulator->hscale));
  thetad = gtk_range_get_value (GTK_RANGE (dialog_simulator->vscale));
  phi = phid * M_PI / 180.;
  theta = thetad * M_PI / 180.;
  if (fractal_3D)
    {
      perspective (0, 0, 0, &k1, &k2);
      xmin = k1;
      perspective (length, width, 0, &k1, &k2);
      xmax = k1;
      perspective (length, 0, 0, &k1, &k2);
      ymin = k2;
      perspective (0, width, height, &k1, &k2);
      ymax = k2;
    }
  draw ();
}

/**
 * Function to show a dialog to save the graphical view in a PNG file.
 */
void
dialog_draw_save ()
{
  char *buffer = NULL;
  GtkFileFilter *filter;
  GtkFileChooserDialog *dlg;
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "PNG file");
  gtk_file_filter_add_pattern (filter, "*.png");
  gtk_file_filter_add_pattern (filter, "*.PNG");
  dlg = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new
    (gettext ("Save graphical"), dialog_simulator->window,
     GTK_FILE_CHOOSER_ACTION_SAVE,
     gettext ("_OK"), GTK_RESPONSE_CANCEL,
     gettext ("_Cancel"), GTK_RESPONSE_OK, NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dlg), filter);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg), 1);
  if (gtk_dialog_run ((GtkDialog *) dlg) == GTK_RESPONSE_OK)
    buffer = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));
  gtk_widget_destroy (GTK_WIDGET (dlg));
  if (buffer)
    {
      draw ();
      while (gtk_events_pending ())
        gtk_main_iteration ();
      draw_save (buffer);
      g_free (buffer);
    }
}

/**
 * Function to update the enabled fractal characteristics.
 */
void
dialog_options_update ()
{
  int i;
  DialogOptions *dlg = dialog_options;
  i = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->button_3D));
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->label_length), i);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_length), i);
  for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->array_seeds[i])))
      break;
  i = (i == RANDOM_SEED_TYPE_FIXED) ? 1 : 0;
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->label_seed), i);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_seed), i);
}

/**
 * Function to create a dialog to set the fractal options.
 */
void
dialog_options_create ()
{
  int i;
  const char *array_fractals[N_FRACTAL_TYPES] =
    { gettext ("_Tree"), gettext ("_Forest"), gettext ("_Neuron") };
  const char *array_algorithms[N_RANDOM_TYPES] = {
    "_mt19937",
    "_ranlxs0",
    "r_anlxs1",
    "ra_nlxs2",
    "ran_lxd1",
    "ranlx_d2",
    "ranlu_x",
    "ranlux_389",
    "_cmrg",
    "mr_g",
    "_taus2",
    "g_fsr4"
  };
  const char *array_seeds[N_RANDOM_SEED_TYPES] =
    { gettext ("_Default"), gettext ("_Clock based"), gettext ("_Fixed") };
  DialogOptions *dlg = dialog_options;

  dlg->button_diagonal = (GtkCheckButton *) gtk_check_button_new_with_mnemonic
    (gettext ("_Diagonal movement"));
  dlg->button_3D =
    (GtkCheckButton *) gtk_check_button_new_with_mnemonic ("3_D");
  g_signal_connect (dlg->button_3D, "clicked", dialog_options_update, NULL);
  dlg->label_length = (GtkLabel *) gtk_label_new (gettext ("Length"));
  dlg->label_width = (GtkLabel *) gtk_label_new (gettext ("Width"));
  dlg->label_height = (GtkLabel *) gtk_label_new (gettext ("Height"));
  dlg->label_seed = (GtkLabel *) gtk_label_new (gettext ("Random seed"));
  dlg->entry_length =
    (GtkSpinButton *) gtk_spin_button_new_with_range (320., 2400., 1.);
  dlg->entry_width =
    (GtkSpinButton *) gtk_spin_button_new_with_range (320., 2400., 1.);
  dlg->entry_height =
    (GtkSpinButton *) gtk_spin_button_new_with_range (200., 2400., 1.);
  dlg->entry_seed =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 4294967295., 1.);

  dlg->grid_fractal = (GtkGrid *) gtk_grid_new ();
  dlg->array_fractals[0] = NULL;
  for (i = 0; i < N_FRACTAL_TYPES; ++i)
    {
      dlg->array_fractals[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_fractals[0], array_fractals[i]);
      gtk_grid_attach (dlg->grid_fractal, GTK_WIDGET (dlg->array_fractals[i]),
                       0, i, 1, 1);
    }
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (dlg->array_fractals[fractal_type]), 1);
  dlg->frame_fractal = (GtkFrame *) gtk_frame_new (gettext ("Fractal type"));
  gtk_container_add (GTK_CONTAINER (dlg->frame_fractal),
                     GTK_WIDGET (dlg->grid_fractal));

  dlg->button_animate = (GtkCheckButton *) gtk_check_button_new_with_mnemonic
    (gettext ("_Animate"));
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (dlg->button_animate), animating);

  dlg->grid_algorithm = (GtkGrid *) gtk_grid_new ();
  dlg->array_algorithms[0] = NULL;
  for (i = 0; i < N_RANDOM_TYPES; ++i)
    {
      dlg->array_algorithms[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_algorithms[0], array_algorithms[i]);
      gtk_grid_attach (dlg->grid_algorithm,
                       GTK_WIDGET (dlg->array_algorithms[i]), 0, i, 1, 1);
    }
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (dlg->array_algorithms[random_algorithm]), 1);
  dlg->frame_algorithm
    = (GtkFrame *) gtk_frame_new (gettext ("Random algorithm"));
  gtk_container_add (GTK_CONTAINER (dlg->frame_algorithm),
                     GTK_WIDGET (dlg->grid_algorithm));

  dlg->grid_seed = (GtkGrid *) gtk_grid_new ();
  dlg->array_seeds[0] = NULL;
  for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
    {
      dlg->array_seeds[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_seeds[0], array_seeds[i]);
      gtk_grid_attach (dlg->grid_seed, GTK_WIDGET (dlg->array_seeds[i]),
                       0, i, 1, 1);
      g_signal_connect (dlg->array_seeds[i], "clicked", dialog_options_update,
                        NULL);
    }
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (dlg->array_seeds[random_seed_type]), 1);
  dlg->frame_seed = (GtkFrame *) gtk_frame_new (gettext ("Random seed type"));
  gtk_container_add (GTK_CONTAINER (dlg->frame_seed),
                     GTK_WIDGET (dlg->grid_seed));

  dlg->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->button_diagonal), 0, 0, 2, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->button_3D), 0, 1, 2, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_length), 0, 2, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_length), 1, 2, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_width), 0, 3, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_width), 1, 3, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_height), 0, 4, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_height), 1, 4, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->frame_fractal), 0, 5, 2, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->button_animate), 0, 6, 2, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->frame_algorithm), 2, 0, 1, 8);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->frame_seed), 0, 7, 2, 2);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_seed), 0, 9, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_seed), 1, 9, 1, 1);

  dlg->dialog
    = (GtkDialog *) gtk_dialog_new_with_buttons (gettext ("Options"),
                                                 dialog_simulator->window,
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 gettext ("_OK"),
                                                 GTK_RESPONSE_OK,
                                                 gettext ("_Cancel"),
                                                 GTK_RESPONSE_CANCEL, NULL);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (dlg->dialog)),
                     GTK_WIDGET (dlg->grid));
  gtk_widget_show_all (GTK_WIDGET (dlg->dialog));

  gtk_toggle_button_set_active
    ((GtkToggleButton *) dlg->button_diagonal, fractal_diagonal);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->button_3D), fractal_3D);
  gtk_spin_button_set_value (dlg->entry_length, length);
  gtk_spin_button_set_value (dlg->entry_width, width);
  gtk_spin_button_set_value (dlg->entry_height, height);
  gtk_spin_button_set_value (dlg->entry_seed, random_seed);
  dialog_options_update ();

  if (gtk_dialog_run (dlg->dialog) == GTK_RESPONSE_OK)
    {
      fractal_diagonal = gtk_toggle_button_get_active
        (GTK_TOGGLE_BUTTON (dlg->button_diagonal));
      fractal_3D =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->button_3D));
      for (i = 0; i < N_FRACTAL_TYPES; ++i)
        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dlg->array_fractals[i])))
          fractal_type = i;
      length = gtk_spin_button_get_value_as_int (dlg->entry_length);
      width = gtk_spin_button_get_value_as_int (dlg->entry_width);
      height = gtk_spin_button_get_value_as_int (dlg->entry_height);
#if HAVE_GTKGLAREA
	    gtk_widget_set_size_request (GTK_WIDGET (dialog_simulator->gl_area),
			                             width, height);
#endif
      random_seed = gtk_spin_button_get_value_as_int (dlg->entry_seed);
      animating = gtk_toggle_button_get_active
        (GTK_TOGGLE_BUTTON (dlg->button_animate));
      for (i = 0; i < N_RANDOM_TYPES; ++i)
        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dlg->array_algorithms[i])))
          random_algorithm = i;
      for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dlg->array_seeds[i])))
          random_seed_type = i;
      medium_start ();
      set_perspective ();
      breaking = 1;
    }
  gtk_widget_destroy (GTK_WIDGET (dlg->dialog));
}

/**
 * Function to show a help dialog.
 */
void
dialog_simulator_help ()
{
  gchar *authors[] = {
    "Javier Burguete Tolosa (jburguete@eead.csic.es)",
    NULL
  };
  gtk_show_about_dialog (dialog_simulator->window,
                         "program_name", "Fractal",
                         "comments",
                         gettext
                         ("A program growing fractals to benchmark "
                          "parallelization and drawing libraries"),
                         "authors", authors,
                         "translator-credits",
                         gettext
                         ("Javier Burguete Tolosa (jburguete@eead.csic.es)"),
                         "version", "2.10.14",
                         "copyright",
                         "Copyright 2009-2018 Javier Burguete Tolosa",
                         "logo", dialog_simulator->logo,
                         "website-label", gettext ("Website"),
                         "website", "https://github.com/jburguete/fractal",
                         NULL);
}

/**
 * Function to update the enabled window properties.
 */
void
dialog_simulator_update ()
{
  gtk_widget_set_sensitive
    (GTK_WIDGET (dialog_simulator->button_start), !simulating);
  gtk_widget_set_sensitive
    (GTK_WIDGET (dialog_simulator->button_stop), simulating);
}

/**
 * Function to show the fractal progress.
 */
void
dialog_simulator_progress ()
{
  register unsigned int k;
  register float x;
  if (fractal_3D)
    {
      switch (fractal_type)
        {
        case 0:
        case 1:
          x = max_d / (float) (height - 1);
          break;
        default:
          k = length;
          if (width < k)
            k = width;
          if (height < k)
            k = height;
          k = k / 2;
					if (k)
						--k;
          x = fmin (1., max_d / (float) k);
        }
    }
  else
    {
      switch (fractal_type)
        {
        case 0:
        case 1:
          x = max_d / (float) (height - 1);
          break;
        default:
          k = width;
          if (height < k)
            k = height;
          k = k / 2 - 1;
          x = fmin (1., max_d / (float) k);
        }
    }
  gtk_progress_bar_set_fraction (dialog_simulator->progress, x);
  gtk_spin_button_set_value
    (dialog_simulator->entry_time, difftime (time (NULL), t0));
  while (gtk_events_pending ())
    gtk_main_iteration ();
}

/**
 * Function to save the graphical view on a PNG file.
 */
void
dialog_simulator_save ()
{
  char *filename = NULL;
  GtkFileChooserDialog *dlg;
  dlg = (GtkFileChooserDialog *)
    gtk_file_chooser_dialog_new (gettext ("Save graphical"),
                                 dialog_simulator->window,
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 gettext ("_Cancel"), GTK_RESPONSE_CANCEL,
                                 gettext ("_Open"), GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg), 1);
  if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));
  gtk_widget_destroy (GTK_WIDGET (dlg));
  if (filename)
    {
      draw_save (filename);
      g_free (filename);
    }
}

#if HAVE_GTKGLAREA

void
dialog_simulator_draw_init (GtkGLArea *gl_area)
{
	gtk_gl_area_make_current (gl_area);
}

void
dialog_simulator_draw_render ()
{
	draw ();
}

#elif HAVE_GLFW

/**
 * Function to close the GLFW window.
 */
void
window_close ()
{
  glfwSetWindowShouldClose (window, GL_TRUE);
}

#endif

/**
 * Function to create the main window.
 */
void
dialog_simulator_create ()
{
  static char *str_exit, *str_options, *str_start, *str_stop, *str_save,
    *str_help;
  static char *tip_exit, *tip_options, *tip_start, *tip_stop, *tip_save,
    *tip_help;
  DialogSimulator *dlg;

#if DEBUG
  printf ("dialog_simulator_create: start\n");
#endif

  dlg = dialog_simulator;
#if HAVE_SDL
  exit_event->type = SDL_QUIT;
#endif

  str_options = gettext ("_Options");
  str_start = gettext ("S_tart");
  str_stop = gettext ("Sto_p");
  str_save = gettext ("_Save");
  str_help = gettext ("_Help");
  str_exit = gettext ("E_xit");
  tip_options = gettext ("Fractal options");
  tip_start = gettext ("Start fractal growing");
  tip_stop = gettext ("Stop fractal growing");
  tip_save = gettext ("Save graphical");
  tip_help = gettext ("Help");
  tip_exit = gettext ("Exit");

  dlg->toolbar = (GtkToolbar *) gtk_toolbar_new ();

  dlg->button_options = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("preferences-system", GTK_ICON_SIZE_SMALL_TOOLBAR), str_options);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_options), tip_options);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_options), -1);
  g_signal_connect
    (dlg->button_options, "clicked", dialog_options_create, NULL);

  dlg->button_start = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("system-run", GTK_ICON_SIZE_SMALL_TOOLBAR), str_start);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_start), tip_start);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_start), -1);
  g_signal_connect (dlg->button_start, "clicked", fractal, NULL);

  dlg->button_stop = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("process-stop", GTK_ICON_SIZE_SMALL_TOOLBAR), str_stop);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_stop), tip_stop);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_stop), -1);
  g_signal_connect (dlg->button_stop, "clicked", fractal_stop, NULL);

  dlg->button_save = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("document-save", GTK_ICON_SIZE_SMALL_TOOLBAR), str_save);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_save), tip_save);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_save), -1);
  g_signal_connect (dlg->button_save, "clicked", dialog_simulator_save, NULL);

  dlg->button_help = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("help-about", GTK_ICON_SIZE_SMALL_TOOLBAR), str_help);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_help), tip_help);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_help), -1);
  g_signal_connect (dlg->button_help, "clicked", dialog_simulator_help, NULL);

  dlg->button_exit = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name
     ("application-exit", GTK_ICON_SIZE_SMALL_TOOLBAR), str_exit);
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_exit), tip_exit);
  gtk_toolbar_insert (dlg->toolbar, GTK_TOOL_ITEM (dlg->button_exit), -1);
#if HAVE_GTKGLAREA
  g_signal_connect (dlg->button_exit, "clicked", gtk_main_quit, NULL);
#elif HAVE_FREEGLUT
  g_signal_connect (dlg->button_exit, "clicked", glutLeaveMainLoop, NULL);
#elif HAVE_SDL
  g_signal_connect_swapped (dlg->button_exit, "clicked",
                            (void (*)) SDL_PushEvent, exit_event);
#elif HAVE_GLFW
  g_signal_connect (dlg->button_exit, "clicked", (void (*)) window_close, NULL);
#endif

  dlg->label_time = (GtkLabel *) gtk_label_new (gettext ("Calculating time"));
  dlg->entry_time =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1.e6, 0.1);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_time), 0);

  dlg->progress = (GtkProgressBar *) gtk_progress_bar_new ();
  gtk_progress_bar_set_text (dlg->progress, gettext ("Progress"));

  dlg->hscale = (GtkScale *) gtk_scale_new_with_range
    (GTK_ORIENTATION_HORIZONTAL, -90., 0., 1.);
  dlg->vscale = (GtkScale *) gtk_scale_new_with_range
    (GTK_ORIENTATION_HORIZONTAL, 0., 90., 1.);
  gtk_scale_set_digits (dlg->hscale, 0);
  gtk_scale_set_digits (dlg->vscale, 0);
  gtk_range_set_value (GTK_RANGE (dlg->hscale), phid);
  gtk_range_set_value (GTK_RANGE (dlg->vscale), thetad);
  g_signal_connect (dlg->hscale, "value-changed", set_perspective, NULL);
  g_signal_connect (dlg->vscale, "value-changed", set_perspective, NULL);

  dlg->label_horizontal = (GtkLabel *) gtk_label_new
    (gettext ("Horizontal perspective angle (º)"));
  dlg->label_vertical = (GtkLabel *) gtk_label_new
    (gettext ("Vertical perspective angle (º)"));

  dlg->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->toolbar), 0, 0, 3, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->progress), 0, 1, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_time), 1, 1, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_time), 2, 1, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_horizontal), 0, 2, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->hscale), 1, 2, 2, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_vertical), 0, 3, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->vscale), 1, 3, 2, 1);

#if HAVE_GTKGLAREA
	dlg->gl_area = (GtkGLArea *) gtk_gl_area_new ();
	gtk_widget_set_size_request (GTK_WIDGET (dlg->gl_area),
			                         window_width, window_height);
	gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->gl_area), 0, 4, 3, 1);
	g_signal_connect (dlg->gl_area, "realize",
		              	(GCallback) dialog_simulator_draw_init, NULL);
	g_signal_connect (dlg->gl_area, "render", dialog_simulator_draw_render, NULL);
#endif

  dlg->logo = gtk_image_get_pixbuf
    (GTK_IMAGE (gtk_image_new_from_file ("logo.png")));
  dlg->logo_min = gtk_image_get_pixbuf
    (GTK_IMAGE (gtk_image_new_from_file ("logo2.png")));

  dlg->window = (GtkWindow *) gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (dlg->window, gettext ("Fractal growing"));
  gtk_window_set_icon (dlg->window, dlg->logo_min);
  gtk_container_add (GTK_CONTAINER (dlg->window), GTK_WIDGET (dlg->grid));
  gtk_widget_show_all (GTK_WIDGET (dlg->window));
#if HAVE_GTKGLAREA
  g_signal_connect (dlg->window, "delete_event", gtk_main_quit, NULL);
#elif HAVE_FREEGLUT
  g_signal_connect (dlg->window, "delete_event", glutLeaveMainLoop, NULL);
#elif HAVE_SDL
  g_signal_connect_swapped (dlg->window, "delete_event",
                            (void (*)) SDL_PushEvent, exit_event);
#elif HAVE_GLFW
  g_signal_connect (dlg->window, "delete_event", (void (*)) window_close, NULL);
#endif

#if DEBUG
  printf ("dialog_simulator_create: end\n");
#endif
}
