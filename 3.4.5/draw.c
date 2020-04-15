/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2020, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
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
extern SDL_Window *window;
#elif HAVE_GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow *window;
#endif

#include "config.h"
#include "fractal.h"
#include "image.h"
#include "text.h"
#include "draw.h"
#include "simulator.h"

GLuint program_3D;              ///< 3D program.
GLint attribute_3D_position;    ///< 3D variable position
GLint attribute_3D_icolor;      ///< 3D variable color identifier.
GLint uniform_3D_matrix;        ///< 3D constant matrix.
unsigned int window_width = 480;        ///< Graphic window width.
unsigned int window_height = 480;       ///< Graphic window height.

GLfloat projection_matrix[16] = {
  1., 0., 0., 0.,
  0, 1., 0., 0.,
  0., 0, 0., 0.,
  0., 0., 0., 1.
};                              ///< Projection matrix.

Image *logo;                    ///< Logo data.
Text text[1];                   ///< Text data.

/**
 * Function to init the graphic data.

 * \return 1 on success, 0 on error.
 */
int
draw_init ()
{
  const char *vs_2D_source =
    "attribute highp vec2 position;"
    "attribute lowp vec3 color;"
    "varying lowp vec3 fcolor;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 0.f, 1.f); fcolor = color;}";
  const char *vs_3D_source =
    "attribute highp vec3 position;"
    "attribute lowp vec3 color;"
    "varying lowp vec3 fcolor;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 1.f); fcolor = color;}";
  const char *fs_source =
    "varying lowp vec3 fcolor;"
    "void main () {gl_FragColor = vec4 (fcolor, 1.f);}";
  const char *vertex_name = "position";
  const char *color_name = "color";
  const char *matrix_name = "matrix";
  // GLSL version
  const char *version = "#version 120\n";       // OpenGL 2.1
  const char *vs_2D_sources[3] = { version, INIT_GL_GLES, vs_2D_source };
  const char *vs_3D_sources[3] = { version, INIT_GL_GLES, vs_3D_source };
  const char *fs_sources[3] = { version, INIT_GL_GLES, fs_source };
  const char *error_message;
  GLint k;
  GLuint vs, fs;
  GLenum glew_status;

#if DEBUG
  printf ("draw_init: start\n");
  fflush (stdout);
#endif

  // Initing GLEW
#if DEBUG
  printf ("Initing GLEW\n");
  fflush (stdout);
#endif
  glew_status = glewInit ();
  if (glew_status != GLEW_OK)
    {
      printf ("ERROR! glewInit: %s\n", glewGetErrorString (glew_status));
      return 0;
    }

  // OpenGL properties
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if DEBUG
  printf ("draw_init: compiling 2D vertex shader\n");
  fflush (stdout);
#endif
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_2D_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D vertex shader";
      goto exit_on_error;
    }

#if DEBUG
  printf ("draw_init: compiling 2D vertex shader\n");
  fflush (stdout);
#endif
  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the fragment shader";
      goto exit_on_error;
    }

  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_3D_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 3D vertex shader";
      goto exit_on_error;
    }

  program_3D = glCreateProgram ();
  glAttachShader (program_3D, vs);
  glAttachShader (program_3D, fs);
  glLinkProgram (program_3D);
  glGetProgramiv (program_3D, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 3D";
      goto exit_on_error;
    }

  attribute_3D_position = glGetAttribLocation (program_3D, vertex_name);
  if (attribute_3D_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  attribute_3D_icolor = glGetAttribLocation (program_3D, color_name);
  if (attribute_3D_icolor == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  uniform_3D_matrix = glGetUniformLocation (program_3D, matrix_name);
  if (uniform_3D_matrix == -1)
    {
      error_message = "could not bind uniform";
      goto exit_on_error;
    }

#if DEBUG
  printf ("draw_init: initing logo\n");
  fflush (stdout);
#endif
  if (!image_init (logo))
    {
      error_message = "unable to init the logo";
      goto exit_on_error;
    }

#if DEBUG
  printf ("draw_init: initing text\n");
  fflush (stdout);
#endif
  if (!text_init (text))
    {
      error_message = "unable to init the text";
      goto exit_on_error;
    }

#if DEBUG
  printf ("draw_init: end\n");
  fflush (stdout);
#endif
  return 1;

exit_on_error:
  printf ("ERROR! %s\n", error_message);
#if DEBUG
  printf ("draw_init: end\n");
  fflush (stdout);
#endif
  return 0;
}

/**
 * Function to updating window data when resizing.
 */
void
draw_resize (int width,         ///< Graphic window width.
             int height)        ///< Graphic window height.
{
  window_width = width;
  window_height = height;
  glViewport (0, 0, width, height);
}

/**
 * Function to draw the fractal.
 */
void
draw ()
{
  // Rectangle matrix
  Point3D square_vertices[4] = {
    {{0., 0., 0.}, {0., 0., 0.}},
    {{length, 0., 0.}, {0., 0., 0.}},
    {{length, width, 0.}, {0., 0., 0.}},
    {{0., width, 0.}, {0., 0., 0.}}
  };
  const GLushort square_indices[4] = { 0, 1, 2, 3 };
  const GLfloat black[4] = { 0., 0., 0., 1. };
  float cp, sp, ct, st, w, h, sx, sy;
  GLuint vbo_square, ibo_square, vbo_points;
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

  // Drawing a white background
  glClearColor (1., 1., 1., 0.);
  glClear (GL_COLOR_BUFFER_BIT);

#if DEBUG
  printf ("draw: ending if no fractal\n");
  fflush (stdout);
#endif

  // Ending if no fractal
  if (!medium)
    goto end_draw;

#if DEBUG
  printf ("draw: checking the fractal type\n");
  fflush (stdout);
#endif

  // Checking if 3D or 2D fractal
  glUseProgram (program_3D);
  if (fractal_3D)
    {
      // Projection matrix
      sp = sinf (phi);
      cp = cosf (phi);
      st = sinf (theta);
      ct = cosf (theta);
      w = xmax - xmin;
      h = ymax - ymin;
      projection_matrix[0] = 2. * cp / w;
      projection_matrix[1] = 2. * ct * sp / h;
      projection_matrix[4] = -2. * sp / w;
      projection_matrix[5] = 2. * ct * cp / h;
      projection_matrix[9] = 2. * st / h;
      projection_matrix[12] = -1.;
      projection_matrix[13] = -1. - 2. * ymin / h;
      glUniformMatrix4fv (uniform_3D_matrix, 1, GL_FALSE, projection_matrix);

      // Drawing a black rectangle
      glGenBuffers (1, &vbo_square);
      glBindBuffer (GL_ARRAY_BUFFER, vbo_square);
      glBufferData (GL_ARRAY_BUFFER, sizeof (square_vertices), square_vertices,
                    GL_DYNAMIC_DRAW);
      glGenBuffers (1, &ibo_square);
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_square);
      glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (square_indices),
                    square_indices, GL_DYNAMIC_DRAW);
      glBindBuffer (GL_ARRAY_BUFFER, vbo_square);
      glEnableVertexAttribArray (attribute_3D_position);
      glVertexAttribPointer (attribute_3D_position,
                             3, GL_FLOAT, GL_FALSE, sizeof (Point3D), NULL);
      glEnableVertexAttribArray (attribute_3D_icolor);
      glVertexAttribPointer (attribute_3D_icolor,
                             3,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof (Point3D),
                             (GLvoid *) offsetof (Point3D, c));
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_square);
      glDrawElements (GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
      glDeleteBuffers (1, &ibo_square);
      glDeleteBuffers (1, &vbo_square);
      glDisableVertexAttribArray (attribute_3D_icolor);
      glDisableVertexAttribArray (attribute_3D_position);

    }
  else
    {
      // Projection matrix
      projection_matrix[0] = 2. / width;
      projection_matrix[5] = 2. / height;
      projection_matrix[12] = projection_matrix[13] = -1.;
      projection_matrix[1] = projection_matrix[4] = projection_matrix[9] = 0.;
      glUniformMatrix4fv (uniform_3D_matrix, 1, GL_FALSE, projection_matrix);
    }

#if DEBUG
  printf ("draw: drawing the fractal points\n");
  fflush (stdout);
#endif

  // Drawing the fractal points
  glGenBuffers (1, &vbo_points);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_points);
  glBufferData (GL_ARRAY_BUFFER, npoints * sizeof (Point3D), point,
                GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray (attribute_3D_position);
  glVertexAttribPointer (attribute_3D_position,
                         3, GL_FLOAT, GL_FALSE, sizeof (Point3D), NULL);
  glEnableVertexAttribArray (attribute_3D_icolor);
  glVertexAttribPointer (attribute_3D_icolor,
                         3,
                         GL_FLOAT,
                         GL_FALSE,
                         sizeof (Point3D), (GLvoid *) offsetof (Point3D, c));
  glDrawArrays (GL_POINTS, 0, npoints);
  glDeleteBuffers (1, &vbo_points);
  glDisableVertexAttribArray (attribute_3D_icolor);
  glDisableVertexAttribArray (attribute_3D_position);

end_draw:

#if DEBUG
  printf ("draw: drawing the logo\n");
  fflush (stdout);
#endif

  // Drawing the logo
  image_draw (logo, window_width, window_height);

#if DEBUG
  printf ("draw: displaying the program version\n");
  fflush (stdout);
#endif

  // Displaying the program version
  sx = 2. / window_width;
  sy = 2. / window_height;
  text_draw (text, "Fractal 3.4.5", 1. - 90. * sx, -0.99, sx, sy, black);

#if DEBUG
  printf ("draw: displaying the draw\n");
  fflush (stdout);
#endif

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
