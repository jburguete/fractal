/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2015, Javier Burguete Tolosa.

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
 * \file main.c
 * \brief Source file to define the main function.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2015, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <libintl.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <GL/freeglut.h>
#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

/**
 * \fn int main(int argn, char **argc)
 * \brief Main function
 * \param argn
 * \brief Arguments number.
 * \param argc
 * \brief Array of arguments.
 * \return 0 on success.
 */
int
main (int argn, char **argc)
{
// PARALELLIZING INIT
#ifdef G_OS_WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo (&sysinfo);
  nthreads = sysinfo.dwNumberOfProcessors;
#else
  nthreads = (int) sysconf (_SC_NPROCESSORS_CONF);
#endif
// END

  // Initing locales
  bindtextdomain ("fractal", "./po");
  bind_textdomain_codeset ("fractal", "UTF-8");
  textdomain ("fractal");

  // Initing GTK+
  gtk_init (&argn, &argc);

  // Initing FreeGLUT
  glutInit (&argn, argc);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize (640, 480);
  glutCreateWindow ("fractal");

  // Initing logo
  logo_new ("logo.png");

  // Passing the GTK+ signals to the FreeGLUT main loop
  glutIdleFunc ((void (*)) gtk_main_iteration);

  // Setting our draw function as the FreeGLUT display function
  glutDisplayFunc (draw);

  // Creating the main GTK+ window
#if DEBUG
  printf ("Creating simulator dialog\n");
#endif
  dialog_simulator_create (&dialog_simulator);

  // FreeGLUT main loop
#if DEBUG
  printf ("GLUT main loop\n");
#endif
  glutMainLoop ();

  return 0;
}
