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
 * \file simulator.h
 * \brief Header file to define the windows data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2018, Javier Burguete Tolosa.
 */
#ifndef SIMULATOR__H
#define SIMULATOR__H 1

/**
 * /struct DialogOptions
 * /brief A struct to show a window to set the fractal options.
 */
typedef struct
{
  GtkLabel *label_length;       ///< GtkLabel to show the length label.
  GtkLabel *label_width;        ///< GtkLabel to show the width label.
  GtkLabel *label_height;       ///< GtkLabel to show the height label.
  GtkLabel *label_seed;         ///< GtkLabel to show the seed label.
  GtkSpinButton *entry_length;  ///< GtkSpinButton to set the medium length.
  GtkSpinButton *entry_width;   ///< GtkSpinButton to set the medium width.
  GtkSpinButton *entry_height;  ///< GtkSpinButton to set the medium height.
  GtkSpinButton *entry_seed;
  ///< GtkSpinButton to set the random numbers generator seed.
  GtkCheckButton *button_diagonal;
  ///< GtkButton to allow diagonal movements.
  GtkCheckButton *button_3D;
  ///< GtkButton to set 2D / 3D fractals.
  GtkCheckButton *button_animate;
  ///< GtkButton to set graphical animation.
  GtkRadioButton *array_fractals[N_FRACTAL_TYPES];
  ///< Array of GtkRadioButtons to set the fractal type.
  GtkRadioButton *array_algorithms[N_RANDOM_TYPES];
  ///< Array of GtkRadioButtons to set the random numbers generator algoritm.
  GtkRadioButton *array_seeds[N_RANDOM_SEED_TYPES];
  ///< Array of GtkRadioButtons to set the random seed type.
  GtkFrame *frame_fractal;      ///< GtkFrame to show the fractal types.
  GtkFrame *frame_algorithm;
  ///< GtkFrame to show the random number generator algorithms.
  GtkFrame *frame_seed;         ///< GtkFrame to show the random seed types.
  GtkGrid *grid_fractal;        ///< GtkGrid to group the fractal types.
  GtkGrid *grid_algorithm;
  ///< GtkGrid to group the random numbers generator algorithms.
  GtkGrid *grid_seed;           ///< GtkGrid to group the random seed types.
  GtkGrid *grid;                ///< GtkGrid to group the widgets.
  GtkDialog *dialog;            ///< GtkDialog to show the options window.
} DialogOptions;

/**
 * /struct DialogSimulator
 * /brief A struct containing the main program window.
 */
typedef struct
{
  GtkToolButton *button_options;
  ///< GtkToolButton to set the fractal options.
  GtkToolButton *button_start;  ///< GtkToolButton to run the simulation.
  GtkToolButton *button_stop;   ///< GtkToolButton to stop the simulation.
  GtkToolButton *button_save;   
	///< GtkToolButton to save the view in a PNG file.
  GtkToolButton *button_help;   ///< GtkToolButton to show a help dialog.
  GtkToolButton *button_exit;   ///< GtkToolButton to exit.
  GtkToolbar *toolbar;          ///< GtkToolbar containing the tool items.
  GtkLabel *label_time;
  ///< GtkLabel to show the computational time label.
  GtkLabel *label_horizontal;
  ///< GtkLabel to show the horizontal perspective of the view.
  GtkLabel *label_vertical;
  ///< GtkLabel to show the vertical perspective of the view.
  GtkSpinButton *entry_time;
  ///< GtkSpinButton to show the computational time.
  GtkProgressBar *progress;
  ///< GtkHscale to set the horizontal perspective of the view.
  GtkScale *hscale;
  ///< GtkHscale to set the vertical perspective of the view.
  GtkScale *vscale;
  ///< GtkProgressBar to show the fractal growing progress.
  GtkGrid *grid;                ///< GtkGrid to pack all widgets.
  GdkPixbuf *logo;              ///< GdkPixbuf containing the main logo.
  GdkPixbuf *logo_min;          ///< GdkPixbuf containing the reduced logo.
#if HAVE_GTKGLAREA
	GtkGLArea *gl_area;           ///< GtkGLArea to draw the graphics.
#endif
  GtkWindow *window;            ///< GtkWindow to show the main windows.
} DialogSimulator;

extern DialogOptions dialog_options[1];
extern DialogSimulator dialog_simulator[1];

void set_perspective ();

void dialog_options_update ();
void dialog_options_create ();

void dialog_simulator_help ();
void dialog_simulator_update ();
void dialog_simulator_progress ();
void dialog_simulator_create ();

#endif
