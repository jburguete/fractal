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
#if HAVE_FREEGLUT
#include <GL/freeglut.h>
#elif HAVE_SDL
#include <SDL.h>
#endif
#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

#if HAVE_SDL
SDL_Window *window;

void
main_loop ()
{
  SDL_Event event[1];
  while (1)
	{
	  while (gtk_events_pending ())
		gtk_main_iteration ();
	  while (SDL_PollEvent (event))
		{
		  if (event->type == SDL_QUIT)
		    return;
	      if (event->type == SDL_WINDOWEVENT
			  && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		    draw_resize (event->window.data1, event->window.data2);
		}
	  draw ();
	}
}

#endif

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
#if DEBUG
  printf ("Initing locales\n");
#endif
  bindtextdomain ("fractal", "./po");
  bind_textdomain_codeset ("fractal", "UTF-8");
  textdomain ("fractal");

  // Initing graphic window
#if HAVE_FREEGLUT
#if DEBUG
  printf ("Initing FreeGLUT window\n");
#endif
  glutInit (&argn, argc);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize (window_width, window_height);
  glutCreateWindow ("fractal");
#elif HAVE_SDL
#if DEBUG
  printf ("Initing SDL window\n");
#endif
  SDL_Init (SDL_INIT_VIDEO);
  window = SDL_CreateWindow ("fractal",
		                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		                     window_width, window_height,
		                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
  if (!window)
    {
	  printf ("ERROR! unable to create the window: %s\n", SDL_GetError ());
	  return 1;
	}
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  if (!SDL_GL_CreateContext (window))
    {
	  printf ("ERROR! SDL_GL_CreateContext: %s\n", SDL_GetError ());
	  return 1;
	}
#endif

  // Initing GLEW
#if DEBUG
  printf ("Initing GLEW\n");
#endif
  glew_status = glewInit ();
  if (glew_status != GLEW_OK)
    {
      printf ("ERROR! glewInit: %s\n", glewGetErrorString (glew_status));
      return 1;
    }

  // Initing GTK+
#if DEBUG
  printf ("Initing GTK+\n");
#endif
  gtk_init (&argn, &argc);

  // Initing logo
#if DEBUG
  printf ("Initing logo\n");
#endif
  logo_new ("logo.png");

  // Initing drawing data
#if DEBUG
  printf ("Initing drawing data\n");
#endif
  if (!draw_init ())
    return 2;

  // Creating the main GTK+ window
#if DEBUG
  printf ("Creating simulator dialog\n");
#endif
  dialog_simulator_create (dialog_simulator);

#if HAVE_FREEGLUT
  // FreeGLUT main loop
#if DEBUG
  printf ("GLUT main loop\n");
#endif
  // Passing the GTK+ signals to the FreeGLUT main loop
  glutIdleFunc ((void (*)) gtk_main_iteration);
  // Setting our draw resize function as the FreeGLUT reshape function
  glutReshapeFunc (draw_resize);
  // Setting our draw function as the FreeGLUT display function
  glutDisplayFunc (draw);
  glutMainLoop ();
#elif HAVE_SDL
#if DEBUG
  printf ("Main loop\n");
#endif
  main_loop ();
#endif

  return 0;
}
