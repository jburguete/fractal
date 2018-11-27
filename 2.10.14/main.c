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
 * \file main.c
 * \brief Source file to define the main function.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2018, Javier Burguete Tolosa.
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
#ifdef G_OS_WIN32
#include <windows.h>
#endif
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
#endif

#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

#if HAVE_SDL
SDL_Window *window;             ///< SDL window.
#elif HAVE_GLFW
GLFWwindow *window;             ///< GLFW window.
#endif

/**
 * Function to do the main loop.
 */
void
main_loop ()
{
#if HAVE_SDL
  SDL_Event event[1];
#endif

#if HAVE_FREEGLUT

  // Passing the GTK+ signals to the FreeGLUT main loop
  glutIdleFunc ((void (*)) gtk_main_iteration);
  // Setting our draw resize function as the FreeGLUT reshape function
  glutReshapeFunc (draw_resize);
  // Setting our draw function as the FreeGLUT display function
  glutDisplayFunc (draw);
  // FreeGLUT main loop
  glutMainLoop ();

#elif HAVE_SDL

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

#elif HAVE_GLFW

  while (!glfwWindowShouldClose (window))
    {
      while (gtk_events_pending ())
        gtk_main_iteration ();
      glfwPollEvents ();
      draw ();
    }

#else

  gtk_main ();

#endif
}

/**
 * Main function
 * \return 0 on success.
 */
int
main (int argn,                 ///< Arguments number.
      char **argc)              ///< Array of arguments.
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
  fflush (stdout);
#endif
  bindtextdomain ("fractal", "./po");
  bind_textdomain_codeset ("fractal", "UTF-8");
  textdomain ("fractal");

  // Initing graphic window
#if HAVE_FREEGLUT

#if DEBUG
  printf ("Initing FreeGLUT window\n");
  fflush (stdout);
#endif
  glutInit (&argn, argc);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize (window_width, window_height);
  glutCreateWindow ("fractal");

#elif HAVE_SDL

#if DEBUG
  printf ("Initing SDL window\n");
  fflush (stdout);
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
  SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  if (!SDL_GL_CreateContext (window))
    {
      printf ("ERROR! SDL_GL_CreateContext: %s\n", SDL_GetError ());
      return 1;
    }

#elif HAVE_GLFW

#if DEBUG
  printf ("Initing GLFW window\n");
  fflush (stdout);
#endif
  if (!glfwInit ())
    {
      printf ("ERROR! unable to init GLFW\n");
      return 1;
    }
  window
    = glfwCreateWindow (window_width, window_height, "fractal", NULL, NULL);
  if (!window)
    {
      printf ("ERROR! unable to open the window\n");
      glfwTerminate ();
      return 1;
    }
  glfwMakeContextCurrent (window);

#endif

  // Initing FreeType
  if (FT_Init_FreeType (&ft))
    {
      printf ("ERROR! could not init freetype library\n");
      return 1;
    }

  // Initing GTK+
#if DEBUG
  printf ("Initing GTK+\n");
  fflush (stdout);
#endif
  gtk_init (&argn, &argc);

  // Initing logo
#if DEBUG
  printf ("Initing logo\n");
  fflush (stdout);
#endif
  logo_new ("logo.png");

  // Creating the main GTK+ window
#if DEBUG
  printf ("Creating simulator dialog\n");
  fflush (stdout);
#endif
  dialog_simulator_create (dialog_simulator);

  // Initing GLEW
#if DEBUG
  printf ("Initing GLEW\n");
  fflush (stdout);
#endif
  glew_status = glewInit ();
  if (glew_status != GLEW_OK)
    {
      printf ("ERROR! glewInit: %s\n", glewGetErrorString (glew_status));
      return 1;
    }

  // Initing drawing data
#if DEBUG
  printf ("Initing drawing data\n");
  fflush (stdout);
#endif
  if (!draw_init ())
    return 1;

  set_perspective ();
  dialog_simulator_update ();

  // Main loop
#if DEBUG
  printf ("Main loop\n");
  fflush (stdout);
#endif
  main_loop ();

  // Freeing memory
#if HAVE_GLFW
  glfwDestroyWindow (window);
  glfwTerminate ();
#endif
  logo_destroy ();

  return 0;
}
