/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2022, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2022, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <libintl.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gtk/gtk.h>
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
#include "image.h"
#include "text.h"
#include "graphic.h"
#include "draw.h"
#include "simulator.h"

#if HAVE_SDL
SDL_Event exit_event[1];
#endif

static float phid = -45.;       ///< Horizontal perspective angle (in degrees).
static float thetad = 80.;      ///< Vertical perspective angle (in degrees).

/**
 * Function the set the perspective of a point.
 */
static void
perspective (int x,             ///< Point x-coordinate.
             int y,             ///< Point y-coordinate.
             int z,             ///< Point z-coordinate.
             float *X,          ///< Perspective X-coordinate.
             float *Y)          ///< Perspective Y-coordinate.
{
  float cp, sp, st, ct;
  sincosf (graphic->phi, &sp, &cp);
  sincosf (graphic->theta, &st, &ct);
  *X = x * cp - y * sp;
  *Y = z * st + (y * cp + x * sp) * ct;
}

/**
 * Function to set the view perspective.
 */
static void
set_perspective ()
{
  float k1, k2;
  phid = gtk_range_get_value (GTK_RANGE (dialog_simulator->hscale));
  thetad = gtk_range_get_value (GTK_RANGE (dialog_simulator->vscale));
  graphic->phi = phid * M_PI / 180.;
  graphic->theta = thetad * M_PI / 180.;
  if (fractal_3D)
    {
      perspective (0, 0, 0, &k1, &k2);
      graphic->xmin = k1;
      perspective (length, width, 0, &k1, &k2);
      graphic->xmax = k1;
      perspective (length, 0, 0, &k1, &k2);
      graphic->ymin = k2;
      perspective (0, width, height, &k1, &k2);
      graphic->ymax = k2;
    }
  draw ();
}

/**
 * Function to show an error message
 */
void
show_error (const char *message)        ///< Error message string.
{
  GtkMessageDialog *dlg;
  dlg = (GtkMessageDialog *)
    gtk_message_dialog_new (dialog_simulator->window,
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE, "%s", message);
#if !GTK4
  gtk_widget_show_all (GTK_WIDGET (dlg));
  g_signal_connect_swapped (dlg, "response", G_CALLBACK (gtk_widget_destroy),
                            GTK_WIDGET (dlg));
#else
  gtk_widget_show (GTK_WIDGET (dlg));
  g_signal_connect_swapped (dlg, "response", G_CALLBACK (gtk_window_destroy),
                            GTK_WINDOW (dlg));
#endif
}

/**
 * Function to update the enabled fractal characteristics.
 */
static void
dialog_options_update ()
{
  int i;
  DialogOptions *dlg = dialog_options;
  i = gtk_check_button_get_active (dlg->button_3D);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->label_length), i);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_length), i);
  for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
    if (gtk_check_button_get_active (dlg->array_seeds[i]))
      break;
  i = (i == RANDOM_SEED_TYPE_FIXED) ? 1 : 0;
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->label_seed), i);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_seed), i);
}

/**
 * Function to close a dialog to set the fractal options.
 */
static void
dialog_options_close (DialogOptions * dlg,      ///< DialogOptions struct.
                      int response_id)  ///< Response identifier.
{
  unsigned int i;
  if (response_id == GTK_RESPONSE_OK)
    {
      fractal_diagonal = gtk_check_button_get_active (dlg->button_diagonal);
      fractal_3D = gtk_check_button_get_active (dlg->button_3D);
      for (i = 0; i < N_FRACTAL_TYPES; ++i)
        if (gtk_check_button_get_active (dlg->array_fractals[i]))
          fractal_type = i;
      length = gtk_spin_button_get_value_as_int (dlg->entry_length);
      width = gtk_spin_button_get_value_as_int (dlg->entry_width);
      height = gtk_spin_button_get_value_as_int (dlg->entry_height);
#if HAVE_GTKGLAREA
      gtk_widget_set_size_request (GTK_WIDGET (dialog_simulator->gl_area),
                                   width, height);
#endif
      random_seed = gtk_spin_button_get_value_as_int (dlg->entry_seed);
      nthreads = gtk_spin_button_get_value_as_int (dlg->entry_nthreads);
      animating = gtk_check_button_get_active (dlg->button_animate);
      for (i = 0; i < N_RANDOM_TYPES; ++i)
        if (gtk_check_button_get_active (dlg->array_algorithms[i]))
          random_algorithm = i;
      for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
        if (gtk_check_button_get_active (dlg->array_seeds[i]))
          random_seed_type = i;
      medium_start ();
      set_perspective ();
      breaking = 1;
    }
  else if (response_id == GTK_RESPONSE_CANCEL);
  else
    return;
#if !GTK4
  gtk_widget_destroy (GTK_WIDGET (dlg->dialog));
#else
  gtk_window_destroy (GTK_WINDOW (dlg->dialog));
#endif
}

/**
 * Function to create a dialog to set the fractal options.
 */
static void
dialog_options_create ()
{
  unsigned int i;
  const char *array_fractals[N_FRACTAL_TYPES] =
    { _("_Tree"), _("_Forest"), _("_Neuron") };
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
    { _("_Default"), _("_Clock based"), _("_Fixed") };
  DialogOptions *dlg = dialog_options;
#if !GTK4
  GtkContainer *content;
#else
  GtkBox *content;
#endif

  dlg->button_diagonal = (GtkCheckButton *) gtk_check_button_new_with_mnemonic
    (_("_Diagonal movement"));
  dlg->button_3D =
    (GtkCheckButton *) gtk_check_button_new_with_mnemonic ("3_D");
  g_signal_connect (dlg->button_3D, "toggled", dialog_options_update, NULL);
  dlg->label_length = (GtkLabel *) gtk_label_new (_("Length"));
  dlg->label_width = (GtkLabel *) gtk_label_new (_("Width"));
  dlg->label_height = (GtkLabel *) gtk_label_new (_("Height"));
  dlg->label_seed = (GtkLabel *) gtk_label_new (_("Random seed"));
  dlg->label_nthreads = (GtkLabel *) gtk_label_new (_("Threads number"));
  dlg->entry_length =
    (GtkSpinButton *) gtk_spin_button_new_with_range (32., 2400., 1.);
  dlg->entry_width =
    (GtkSpinButton *) gtk_spin_button_new_with_range (32., 2400., 1.);
  dlg->entry_height =
    (GtkSpinButton *) gtk_spin_button_new_with_range (20., 2400., 1.);
  dlg->entry_seed =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 4294967295., 1.);
  dlg->entry_nthreads =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 64., 1.);

  dlg->grid_fractal = (GtkGrid *) gtk_grid_new ();
  dlg->array_fractals[0] = NULL;
  for (i = 0; i < N_FRACTAL_TYPES; ++i)
    {
#if !GTK4
      dlg->array_fractals[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_fractals[0], array_fractals[i]);
#else
      dlg->array_fractals[i] = (GtkCheckButton *)
        gtk_check_button_new_with_mnemonic (array_fractals[i]);
      gtk_check_button_set_group (dlg->array_fractals[i],
                                  dlg->array_fractals[0]);
#endif
      gtk_grid_attach (dlg->grid_fractal, GTK_WIDGET (dlg->array_fractals[i]),
                       0, i, 1, 1);
    }
  gtk_check_button_set_active (dlg->array_fractals[fractal_type], 1);
  dlg->frame_fractal = (GtkFrame *) gtk_frame_new (_("Fractal type"));
  gtk_frame_set_child (dlg->frame_fractal, GTK_WIDGET (dlg->grid_fractal));

  dlg->button_animate = (GtkCheckButton *) gtk_check_button_new_with_mnemonic
    (_("_Animate"));
  gtk_check_button_set_active (dlg->button_animate, animating);

  dlg->grid_algorithm = (GtkGrid *) gtk_grid_new ();
  dlg->array_algorithms[0] = NULL;
  for (i = 0; i < N_RANDOM_TYPES; ++i)
    {
#if !GTK4
      dlg->array_algorithms[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_algorithms[0], array_algorithms[i]);
#else
      dlg->array_algorithms[i] = (GtkCheckButton *)
        gtk_check_button_new_with_mnemonic (array_algorithms[i]);
      gtk_check_button_set_group (dlg->array_algorithms[i],
                                  dlg->array_algorithms[0]);
#endif
      gtk_grid_attach (dlg->grid_algorithm,
                       GTK_WIDGET (dlg->array_algorithms[i]), 0, i, 1, 1);
    }
  gtk_check_button_set_active (dlg->array_algorithms[random_algorithm], 1);
  dlg->frame_algorithm = (GtkFrame *) gtk_frame_new (_("Random algorithm"));
  gtk_frame_set_child (dlg->frame_algorithm, GTK_WIDGET (dlg->grid_algorithm));

  dlg->grid_seed = (GtkGrid *) gtk_grid_new ();
  dlg->array_seeds[0] = NULL;
  for (i = 0; i < N_RANDOM_SEED_TYPES; ++i)
    {
#if !GTK4
      dlg->array_seeds[i] =
        (GtkRadioButton *) gtk_radio_button_new_with_mnemonic_from_widget
        (dlg->array_seeds[0], array_seeds[i]);
#else
      dlg->array_seeds[i] = (GtkCheckButton *)
        gtk_check_button_new_with_mnemonic (array_seeds[i]);
      gtk_check_button_set_group (dlg->array_seeds[i], dlg->array_seeds[0]);
#endif
      gtk_grid_attach (dlg->grid_seed, GTK_WIDGET (dlg->array_seeds[i]),
                       0, i, 1, 1);
      g_signal_connect (dlg->array_seeds[i], "toggled", dialog_options_update,
                        NULL);
    }
  gtk_check_button_set_active (dlg->array_seeds[random_seed_type], 1);
  dlg->frame_seed = (GtkFrame *) gtk_frame_new (_("Random seed type"));
  gtk_frame_set_child (dlg->frame_seed, GTK_WIDGET (dlg->grid_seed));

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
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->label_nthreads), 0, 10, 1, 1);
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->entry_nthreads), 1, 10, 1, 1);

  dlg->logo = (GtkImage *) gtk_image_new_from_file ("logo2.png");

  dlg->bar = (GtkHeaderBar *) gtk_header_bar_new ();
#if !GTK4
  gtk_header_bar_set_title (dlg->bar, _("Options"));
  gtk_header_bar_set_subtitle (dlg->bar, _("Set the fractal options"));
  gtk_header_bar_set_show_close_button (dlg->bar, 1);
#endif
  gtk_header_bar_pack_start (dlg->bar, GTK_WIDGET (dlg->logo));

  dlg->dialog
    = (GtkDialog *) gtk_dialog_new_with_buttons (NULL,
                                                 dialog_simulator->window,
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 _("_OK"),
                                                 GTK_RESPONSE_OK,
                                                 _("_Cancel"),
                                                 GTK_RESPONSE_CANCEL, NULL);
  gtk_window_set_titlebar (GTK_WINDOW (dlg->dialog), GTK_WIDGET (dlg->bar));
#if !GTK4
  content = (GtkContainer *) gtk_dialog_get_content_area (dlg->dialog);
  gtk_container_add (content, GTK_WIDGET (dlg->grid));
  gtk_widget_show_all (GTK_WIDGET (dlg->dialog));
#else
  gtk_window_set_title (GTK_WINDOW (dlg->dialog), _("Options"));
  content = (GtkBox *) gtk_dialog_get_content_area (dlg->dialog);
  gtk_box_append (content, GTK_WIDGET (dlg->grid));
  gtk_widget_show (GTK_WIDGET (dlg->dialog));
#endif

  gtk_check_button_set_active (dlg->button_diagonal, fractal_diagonal);
  gtk_check_button_set_active (dlg->button_3D, fractal_3D);
  gtk_spin_button_set_value (dlg->entry_length, length);
  gtk_spin_button_set_value (dlg->entry_width, width);
  gtk_spin_button_set_value (dlg->entry_height, height);
  gtk_spin_button_set_value (dlg->entry_seed, random_seed);
  gtk_spin_button_set_value (dlg->entry_nthreads, nthreads);
  dialog_options_update ();

  g_signal_connect_swapped (dlg->dialog, "response",
                            G_CALLBACK (dialog_options_close), dlg);
}

/**
 * Function to show a help dialog.
 */
static void
dialog_simulator_help ()
{
  gchar *authors[] = {
    "Javier Burguete Tolosa (jburguete@eead.csic.es)",
    NULL
  };
  gtk_show_about_dialog (dialog_simulator->window,
                         "program_name", "Fractal",
                         "comments",
                         _("A program using growing fractals to benchmark "
                           "parallelization and drawing libraries"),
                         "authors", authors,
                         "translator-credits",
                         _("Javier Burguete Tolosa (jburguete@eead.csic.es)"),
                         "version", "3.4.16",
                         "copyright",
                         "Copyright 2009-2022 Javier Burguete Tolosa",
                         "license-type", GTK_LICENSE_BSD,
                         "logo", dialog_simulator->logo,
                         "website-label", _("Website"),
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
  set_perspective ();
}

/**
 * Function to show the fractal progress.
 */
void
dialog_simulator_progress ()
{
  GMainContext *context;
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
  context = g_main_context_default ();
  while (g_main_context_pending (context))
    g_main_context_iteration (context, 0);
}

/**
 * Function to close the dialog to save the graphical view.
 */
static void
dialog_simulator_graphic_close (GtkFileChooserDialog * dlg,
                                ///< GtkFileChooserDialog struct.
                                int response_id)       ///< Response identifier.
{
  char *filename;
  if (response_id == GTK_RESPONSE_ACCEPT)
    filename = gtk_file_chooser_get_current_name (GTK_FILE_CHOOSER (dlg));
  else
    filename = NULL;
#if !GTK4
  gtk_widget_destroy (GTK_WIDGET (dlg));
#else
  gtk_window_destroy (GTK_WINDOW (dlg));
#endif
  if (filename)
    {
      graphic_save (filename);
      g_free (filename);
    }
}

/**
 * Function to save the graphical view on a PNG file.
 */
static void
dialog_simulator_graphic_save ()
{
  GtkFileChooserDialog *dlg;
  dlg = (GtkFileChooserDialog *)
    gtk_file_chooser_dialog_new (_("Save graphical"),
                                 dialog_simulator->window,
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 _("_Cancel"), GTK_RESPONSE_CANCEL,
                                 _("_Open"), GTK_RESPONSE_ACCEPT, NULL);
#if !GTK4
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg), 1);
  gtk_widget_show_all (GTK_WIDGET (dlg));
#else
  gtk_widget_show (GTK_WIDGET (dlg));
#endif
  g_signal_connect (dlg, "response",
                    G_CALLBACK (dialog_simulator_graphic_close), NULL);
}

#if HAVE_GTKGLAREA

static void
dialog_simulator_draw_init (GtkGLArea * gl_area)
{
#if DEBUG
  printf ("dialog_simulator_draw_init: start\n");
#endif
  gtk_gl_area_make_current (gl_area);
  graphic_init (graphic, "logo.png");
#if DEBUG
  printf ("dialog_simulator_draw_init: end\n");
#endif
}

#elif HAVE_GLFW

/**
 * Function to close the GLFW window.
 */
static void
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
  static char *tip_exit, *tip_options, *tip_start, *tip_stop, *tip_save,
    *tip_help;
  DialogSimulator *dlg;

#if DEBUG
  printf ("dialog_simulator_create: start\n");
#endif

  dlg = dialog_simulator;
#if HAVE_GTKGLAREA
  dlg->loop = g_main_loop_new (NULL, 0);
#elif HAVE_SDL
  exit_event->type = SDL_QUIT;
#endif

  tip_options = _("Fractal options");
  tip_start = _("Start fractal growing");
  tip_stop = _("Stop fractal growing");
  tip_save = _("Save graphical");
  tip_help = _("Help");
  tip_exit = _("Exit");

#if !GTK4
  dlg->logo = gtk_image_get_pixbuf
    (GTK_IMAGE (gtk_image_new_from_file ("logo.png")));
  dlg->logo_min = gtk_image_get_pixbuf
    (GTK_IMAGE (gtk_image_new_from_file ("logo2.png")));
#else
  dlg->logo = gtk_image_get_paintable
    (GTK_IMAGE (gtk_image_new_from_file ("logo.png")));
  dlg->logo_min = gtk_image_get_paintable
    (GTK_IMAGE (gtk_image_new_from_file ("logo2.png")));
#endif

  dlg->box = (GtkBox *) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  dlg->button_options = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("preferences-system", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("preferences-system");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_options), tip_options);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_options));
  g_signal_connect_swapped
    (dlg->button_options, "clicked", dialog_options_create, dlg->logo_min);

  dlg->button_start = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("system-run", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("system-run");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_start), tip_start);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_start));
  g_signal_connect (dlg->button_start, "clicked", fractal, NULL);

  dlg->button_stop = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("process-stop", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("process-stop");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_stop), tip_stop);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_stop));
  g_signal_connect (dlg->button_stop, "clicked", fractal_stop, NULL);

  dlg->button_save = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("document-save", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("document-save");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_save), tip_save);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_save));
  g_signal_connect (dlg->button_save, "clicked", dialog_simulator_graphic_save,
                    NULL);

  dlg->button_help = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("help-about", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("help-about");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_help), tip_help);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_help));
  g_signal_connect (dlg->button_help, "clicked", dialog_simulator_help, NULL);

  dlg->button_exit = (GtkButton *)
#if !GTK4
    gtk_button_new_from_icon_name ("application-exit", GTK_ICON_SIZE_BUTTON);
#else
    gtk_button_new_from_icon_name ("application-exit");
#endif
  gtk_widget_set_tooltip_text (GTK_WIDGET (dlg->button_exit), tip_exit);
  gtk_box_append (dlg->box, GTK_WIDGET (dlg->button_exit));
#if HAVE_GTKGLAREA
  g_signal_connect_swapped (dlg->button_exit, "clicked",
                            (GCallback) g_main_loop_quit, dlg->loop);
#elif HAVE_FREEGLUT
  g_signal_connect (dlg->button_exit, "clicked", glutLeaveMainLoop, NULL);
#elif HAVE_SDL
  g_signal_connect_swapped (dlg->button_exit, "clicked",
                            (void (*)) SDL_PushEvent, exit_event);
#elif HAVE_GLFW
  g_signal_connect (dlg->button_exit, "clicked", (void (*)) window_close, NULL);
#endif

  dlg->label_time = (GtkLabel *) gtk_label_new (_("Calculating time"));
  dlg->entry_time =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1.e6, 0.1);
  gtk_widget_set_sensitive (GTK_WIDGET (dlg->entry_time), 0);

  dlg->progress = (GtkProgressBar *) gtk_progress_bar_new ();
  gtk_progress_bar_set_text (dlg->progress, _("Progress"));

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
    (_("Horizontal perspective angle (º)"));
  dlg->label_vertical = (GtkLabel *) gtk_label_new
    (_("Vertical perspective angle (º)"));

  dlg->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->box), 0, 0, 3, 1);
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
  g_signal_connect (dlg->gl_area, "realize",
                    (GCallback) dialog_simulator_draw_init, NULL);
  g_signal_connect (dlg->gl_area, "render", draw, NULL);
  g_signal_connect (dlg->gl_area, "resize", (GCallback) resize, NULL);
#if WINDOW_GLAREA
#if !GTK4
  dlg->window_gl = (GtkWindow *) gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_icon (dlg->window_gl, dlg->logo_min);
  gtk_container_add (GTK_CONTAINER (dlg->window_gl), GTK_WIDGET (dlg->gl_area));
  g_signal_connect_swapped (dlg->window_gl, "delete_event",
                            (GCallback) g_main_loop_quit, dlg->loop);
  gtk_widget_show_all (GTK_WIDGET (dlg->window_gl));
#else
  dlg->window_gl = (GtkWindow *) gtk_window_new ();
  gtk_window_set_child (dlg->window_gl, GTK_WIDGET (dlg->gl_area));
  g_signal_connect_swapped (dlg->window_gl, "close-request",
                            (GCallback) g_main_loop_quit, dlg->loop);
  gtk_widget_show (GTK_WIDGET (dlg->window_gl));
#endif
#else
  gtk_grid_attach (dlg->grid, GTK_WIDGET (dlg->gl_area), 0, 4, 3, 1);
#endif
#elif HAVE_SDL
  SDL_SetWindowMinimumSize (window, window_width, window_height);
#elif HAVE_GLFW
  glfwSetWindowSizeLimits (window, window_width, window_height, GLFW_DONT_CARE,
                           GLFW_DONT_CARE);
#endif

#if !GTK4
  dlg->window = (GtkWindow *) gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (dlg->window, _("Fractal growing"));
  gtk_window_set_icon (dlg->window, dlg->logo_min);
  gtk_container_add (GTK_CONTAINER (dlg->window), GTK_WIDGET (dlg->grid));
#if HAVE_GTKGLAREA
  g_signal_connect_swapped (dlg->window, "delete_event",
                            (GCallback) g_main_loop_quit, dlg->loop);
#elif HAVE_FREEGLUT
  g_signal_connect (dlg->window, "delete_event", glutLeaveMainLoop, NULL);
#elif HAVE_SDL
  g_signal_connect_swapped (dlg->window, "delete_event",
                            (void (*)) SDL_PushEvent, exit_event);
#elif HAVE_GLFW
  g_signal_connect (dlg->window, "delete_event", (void (*)) window_close, NULL);
#endif
  gtk_widget_show_all (GTK_WIDGET (dlg->window));
#else
  dlg->window = (GtkWindow *) gtk_window_new ();
  gtk_window_set_title (dlg->window, _("Fractal growing"));
  gtk_window_set_child (dlg->window, GTK_WIDGET (dlg->grid));
#if HAVE_GTKGLAREA
  g_signal_connect_swapped (dlg->window, "close-request",
                            (GCallback) g_main_loop_quit, dlg->loop);
#elif HAVE_FREEGLUT
  g_signal_connect (dlg->window, "close-request", glutLeaveMainLoop, NULL);
#elif HAVE_SDL
  g_signal_connect_swapped (dlg->window, "close-request",
                            (void (*)) SDL_PushEvent, exit_event);
#elif HAVE_GLFW
  g_signal_connect (dlg->window, "close-request", (void (*)) window_close,
                    NULL);
#endif
  gtk_widget_show (GTK_WIDGET (dlg->window));
#endif

#if DEBUG
  printf ("dialog_simulator_create: end\n");
#endif
}
