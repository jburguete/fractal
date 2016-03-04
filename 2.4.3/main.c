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
#include <GL/glew.h>
#include <SDL.h>
#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

void
main_loop (SDL_Window *window)
{
  SDL_Event event[1];
  while (1)
	{
	  while (SDL_PollEvent (event))
		{
		  if (event->type == SDL_QUIT)
		    return;
	      if (event->type == SDL_WINDOWEVENT
			  && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		    window_resize (event->window.data1, event->window.data2);;
		}
	  while (gtk_events_pending ())
		gtk_main_iteration ();
	  draw (window);
	}
}

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
  SDL_Window *window;
  GLenum glew_status;

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

  // Initing SDL
  SDL_Init (SDL_INIT_VIDEO);
  window = SDL_CreateWindow ("fractal",
		                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		                     640, 480,
		                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
  if (!window)
    {
	  printf ("ERROR! unable to create the window: %s\n", SDL_GetError ());
	  return EXIT_FAILURE;
	}
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  if (!SDL_GL_CreateContext (window))
    {
	  printf ("ERROR! SDL_GL_CreateContext: %s\n", SDL_GetError ());
	  return EXIT_FAILURE;
	}
  glew_status = glewInit ();
  if (glew_status != GLEW_OK)
	{
	  printf ("ERROR! glewInit: %s\n", glewGetErrorString (glew_status));
      return EXIT_FAILURE;
	}

  // Initing logo
  logo_new ("logo.png");

  // Creating the main GTK+ window
#if DEBUG
  printf ("Creating simulator dialog\n");
#endif
  dialog_simulator_create (&dialog_simulator);

  // Main loop
#if DEBUG
  printf ("Main loop\n");
#endif
  main_loop (window);

  return 0;
}
