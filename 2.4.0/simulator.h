/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2014, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2014, Javier Burguete Tolosa.
 */
#ifndef SIMULATOR__H
#define SIMULATOR__H 1

/**
 * \struct DialogOptions
 * \brief A struct to show a window to set the fractal options.
 */
typedef struct
{
/**
 * \var label_length
 * \brief GtkLabel to show the length label.
 * \var label_width
 * \brief GtkLabel to show the width label.
 * \var label_height
 * \brief GtkLabel to show the height label.
 * \var label_seed
 * \brief GtkLabel to show the seed label.
 * \var entry_length
 * \brief GtkSpinButton to set the medium length.
 * \var entry_width
 * \brief GtkSpinButton to set the medium width.
 * \var entry_height
 * \brief GtkSpinButton to set the medium height.
 * \var entry_seed
 * \brief GtkSpinButton to set the random numbers generator seed.
 * \var button_diagonal
 * \brief GtkButton to allow diagonal movements.
 * \var button_3D
 * \brief GtkButton to set 2D / 3D fractals.
 * \var button_animate
 * \brief GtkButton to set graphical animation.
 * \var array_fractals
 * \brief Array of GtkRadioButtons to set the fractal type.
 * \var array_algorithms
 * \brief Array of GtkRadioButtons to set the random numbers generator algoritm.
 * \var array_seeds
 * \brief Array of GtkRadioButtons to set the random seed type.
 * \var frame_fractal
 * \brief GtkFrame to show the fractal types.
 * \var frame_algorithm
 * \brief GtkFrame to show the random number generator algorithms.
 * \var frame_seed
 * \brief GtkFrame to show the random seed types.
 * \var grid_fractal
 * \brief GtkGrid to group the fractal types.
 * \var grid_algorithm
 * \brief GtkGrid to group the random numbers generator algorithms.
 * \var grid_seed
 * \brief GtkGrid to group the random seed types.
 * \var grid
 * \brief GtkGrid to group the widgets.
 * \var dialog
 * \brief GtkDialog to show the options window.
 */
	GtkLabel *label_length, *label_width, *label_height, *label_seed;
	GtkSpinButton *entry_length, *entry_width, *entry_height, *entry_seed;
	GtkCheckButton *button_diagonal, *button_3D, *button_animate;
	GtkRadioButton *array_fractals[N_FRACTAL_TYPES],
		*array_algorithms[N_RANDOM_TYPES], *array_seeds[N_RANDOM_SEED_TYPES];
	GtkFrame *frame_fractal, *frame_algorithm, *frame_seed;
	GtkGrid *grid_fractal, *grid_algorithm, *grid_seed, *grid;
	GtkDialog *dialog;
} DialogOptions;

/**
 * \struct DialogSimulator
 * \brief A struct containing the main program window.
 */
typedef struct
{
/**
 * \var button_options
 * \brief GtkToolButton to set the fractal options.
 * \var button_start
 * \brief GtkToolButton to run the simulation.
 * \var button_stop
 * \brief GtkToolButton to stop the simulation.
 * \var button_save
 * \brief GtkToolButton to save the view in a PNG file.
 * \var button_help
 * \brief GtkToolButton to show a help dialog.
 * \var button_exit
 * \brief GtkToolButton to exit.
 * \var toolbar
 * \brief GtkToolbar containing the tool items.
 * \var label_time
 * \brief GtkLabel to show the computational time label.
 * \var label_horizontal
 * \brief GtkLabel to show the horizontal perspective of the view.
 * \var label_vertical
 * \brief GtkLabel to show the vertical perspective of the view.
 * \var entry_time
 * \brief GtkSpinButton to show the computational time.
 * \var hscale
 * \brief GtkHscale to set the horizontal perspective of the view.
 * \var vscale
 * \brief GtkHscale to set the vertical perspective of the view.
 * \var progress
 * \brief GtkProgressBar to show the fractal growing progress.
 * \var grid
 * \brief GtkGrid to pack all widgets.
 * \var logo
 * \brief GdkPixbuf containing the main logo.
 * \var logo_min
 * \brief GdkPixbuf containing the reduced logo.
 * \var window
 * \brief GtkWindow to show the main windows.
 */
	GtkToolButton *button_options, *button_start, *button_stop, *button_save,
		*button_help, *button_exit;
	GtkToolbar *toolbar;
	GtkLabel *label_time, *label_horizontal, *label_vertical;
	GtkSpinButton *entry_time;
	GtkProgressBar *progress;
	GtkScale *hscale, *vscale;
	GtkGrid *grid;
	GdkPixbuf *logo, *logo_min;
	GtkWindow *window;
} DialogSimulator;

extern DialogOptions dialog_options;
extern DialogSimulator dialog_simulator;

void set_perspective();

void dialog_options_update();
void dialog_options_create();

void dialog_simulator_help();
void dialog_simulator_update();
void dialog_simulator_progress();
void dialog_simulator_create();

#endif
