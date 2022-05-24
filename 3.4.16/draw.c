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
 * \file draw.c
 * \brief Source file to define the drawing data and functions.
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
#endif

#include "config.h"
#include "fractal.h"
#include "image.h"
#include "text.h"
#include "graphic.h"
#include "draw.h"
#include "simulator.h"

Graphic graphic[1];             ///< Graphic data.

/**
 * Function to resize the draw.
 */
void
draw_resize (int w,             ///< Graphic window width.
             int h)             ///< Graphic window height.
{
#if DEBUG
  printf ("draw_resize: start\n");
  fflush (stdout);
#endif
  if ((unsigned int) w < width)
    w = width;
  if ((unsigned int) h < height)
    h = height;
#if HAVE_FREEGLUT
  glutReshapeWindow (w, h);
#endif
  window_width = w;
  window_height = h;
  glViewport (0, 0, w, h);
#if DEBUG
  printf ("draw_resize: end\n");
  fflush (stdout);
#endif
}

#if HAVE_GTKGLAREA
void
resize (GtkGLArea * gl_area, int w, int h)
{
  draw_resize (w, h);
  gtk_widget_queue_draw (GTK_WIDGET (gl_area));
}
#endif

/**
 * Function to render graphics.
 */
void
draw ()
{
#if HAVE_GLFW
  int graphic_width, graphic_height;
#endif

#if DEBUG
  printf ("draw: start\n");
  fflush (stdout);
#endif

#if HAVE_GLFW
  glfwGetFramebufferSize (window, &graphic_width, &graphic_height);
  draw_resize (graphic_width, graphic_height);
#endif

  graphic_render (graphic);

  // Displaying the draw
#if HAVE_GTKGLAREA
  gtk_widget_queue_draw (GTK_WIDGET (dialog_simulator->gl_area));
#elif HAVE_FREEGLUT
  glutSwapBuffers ();
#elif HAVE_SDL
  SDL_GL_SwapWindow (window);
#elif HAVE_GLFW
  glfwSwapBuffers (window);
#endif

#if DEBUG
  printf ("draw: end\n");
  fflush (stdout);
#endif
}
