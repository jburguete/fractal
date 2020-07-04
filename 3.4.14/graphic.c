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
 * \file graphic.c
 * \brief Source file to define the graphic drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gtk/gtk.h>
#include <GL/glew.h>

#include "config.h"
#include "fractal.h"
#include "image.h"
#include "text.h"
#include "graphic.h"

unsigned int window_width = 480;        ///< Graphic window width.
unsigned int window_height = 480;       ///< Graphic window height.

/**
 * Function to init the graphic data.

 * \return 1 on success, 0 on error.
 */
int
graphic_init (Graphic * graphic,        ///< Draw struct.
              char *logo_name)  ///< Logo PNG file name.
{
  const GLfloat projection_matrix[16] = {
    1., 0., 0., 0.,
    0, 1., 0., 0.,
    0., 0, 0., 0.,
    0., 0., 0., 1.
  };
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
  const char *vs_3D_sources[3] = { version, INIT_GL_GLES, vs_3D_source };
  const char *fs_sources[3] = { version, INIT_GL_GLES, fs_source };
  const char *error_message;
  GLint k;
  GLuint vs, fs;
  GLenum glew_status;

#if DEBUG
  printf ("graphic_init: start\n");
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

#if DEBUG
  printf ("graphic_init: compiling fragment shader\n");
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

#if DEBUG
  printf ("graphic_init: compiling 3D vertex shader\n");
  fflush (stdout);
#endif
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_3D_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      glDeleteShader (fs);
      error_message = "unable to compile the 3D vertex shader";
      goto exit_on_error;
    }

  graphic->program_3D = glCreateProgram ();
  glAttachShader (graphic->program_3D, vs);
  glAttachShader (graphic->program_3D, fs);
  glLinkProgram (graphic->program_3D);
  glDeleteShader (vs);
  glDeleteShader (fs);
  glGetProgramiv (graphic->program_3D, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 3D";
      goto exit_on_error;
    }

  graphic->attribute_3D_position
    = glGetAttribLocation (graphic->program_3D, vertex_name);
  if (graphic->attribute_3D_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  graphic->attribute_3D_color
    = glGetAttribLocation (graphic->program_3D, color_name);
  if (graphic->attribute_3D_color == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  graphic->uniform_3D_matrix
    = glGetUniformLocation (graphic->program_3D, matrix_name);
  if (graphic->uniform_3D_matrix == -1)
    {
      error_message = "could not bind uniform";
      goto exit_on_error;
    }

  memcpy (graphic->projection_matrix, projection_matrix, 16 * sizeof (GLfloat));

#if DEBUG
  printf ("graphic_init: initing logo\n");
  fflush (stdout);
#endif
  if (logo_name)
    {
      graphic->logo = image_new (logo_name);
      if (!graphic->logo)
        {
          error_message = "unable to open the logo";
          goto exit_on_error;
        }
      if (!image_init (graphic->logo))
        {
          error_message = "unable to init the logo";
          goto exit_on_error;
        }
    }
  else
    graphic->logo = NULL;

#if DEBUG
  printf ("graphic_init: initing text\n");
  fflush (stdout);
#endif
  if (!text_init (graphic->text))
    {
      error_message = "unable to init the text";
      goto exit_on_error;
    }

#if DEBUG
  printf ("graphic_init: end\n");
  fflush (stdout);
#endif
  return 1;

exit_on_error:
  printf ("ERROR! %s\n", error_message);
#if DEBUG
  printf ("graphic_init: end\n");
  fflush (stdout);
#endif
  return 0;
}

/**
 * Function to free the memory used by graphic drawing functions.
 */
void
graphic_destroy (Graphic * graphic)     ///< Graphic struct.
{
#if DEBUG
  printf ("graphic_destroy: start\n");
  fflush (stdout);
#endif
  text_destroy (graphic->text);
  image_destroy (graphic->logo);
#if DEBUG
  printf ("graphic_destroy: end\n");
  fflush (stdout);
#endif
}

/**
 * Function to draw the fractal.
 */
void
graphic_render (Graphic * graphic)      ///< Graphic struct.
{
  // Rectangle matrix
  const Point3D square_vertices[4] = {
    {{0., 0., 0.}, {0., 0., 0.}},
    {{length, 0., 0.}, {0., 0., 0.}},
    {{length, width, 0.}, {0., 0., 0.}},
    {{0., width, 0.}, {0., 0., 0.}}
  };
  const GLushort square_indices[4] = { 0, 1, 2, 3 };
  const GLfloat black[4] = { 0., 0., 0., 1. };
	const char *str_version = "Fractal 3.4.14";
  float cp, sp, ct, st, w, h, sx, sy;
  GLuint vbo_square, ibo_square, vbo_points;

#if DEBUG
  printf ("graphic_render: start\n");
  fflush (stdout);
#endif

  // Drawing a white background
  glClearColor (1., 1., 1., 0.);
  glClear (GL_COLOR_BUFFER_BIT);

#if DEBUG
  printf ("graphic_render: ending if no fractal\n");
  fflush (stdout);
#endif

  // Ending if no fractal
  if (!medium)
    goto end_draw;

#if DEBUG
  printf ("graphic_render: checking the fractal type\n");
  fflush (stdout);
#endif

  // Checking if 3D or 2D fractal
  glUseProgram (graphic->program_3D);
  if (fractal_3D)
    {
      // Projection matrix
      sincosf (graphic->phi, &sp, &cp);
      sincosf (graphic->theta, &st, &ct);
      w = graphic->xmax - graphic->xmin;
      h = graphic->ymax - graphic->ymin;
      graphic->projection_matrix[0] = 2. * cp / w;
      graphic->projection_matrix[1] = 2. * ct * sp / h;
      graphic->projection_matrix[4] = -2. * sp / w;
      graphic->projection_matrix[5] = 2. * ct * cp / h;
      graphic->projection_matrix[9] = 2. * st / h;
      graphic->projection_matrix[12] = -1.;
      graphic->projection_matrix[13] = -1. - 2. * graphic->ymin / h;
      glUniformMatrix4fv (graphic->uniform_3D_matrix, 1, GL_FALSE,
                          graphic->projection_matrix);

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
      glEnableVertexAttribArray (graphic->attribute_3D_position);
      glVertexAttribPointer (graphic->attribute_3D_position,
                             3, GL_FLOAT, GL_FALSE, sizeof (Point3D), NULL);
      glEnableVertexAttribArray (graphic->attribute_3D_color);
      glVertexAttribPointer (graphic->attribute_3D_color,
                             3,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof (Point3D),
                             (GLvoid *) offsetof (Point3D, c));
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_square);
      glDrawElements (GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
      glDeleteBuffers (1, &ibo_square);
      glDeleteBuffers (1, &vbo_square);
      glDisableVertexAttribArray (graphic->attribute_3D_color);
      glDisableVertexAttribArray (graphic->attribute_3D_position);
    }
  else
    {
      // Projection matrix
      graphic->projection_matrix[0] = 2. / width;
      graphic->projection_matrix[5] = 2. / height;
      graphic->projection_matrix[12] = graphic->projection_matrix[13] = -1.;
      graphic->projection_matrix[1] = graphic->projection_matrix[4]
        = graphic->projection_matrix[9] = 0.;
      glUniformMatrix4fv (graphic->uniform_3D_matrix, 1, GL_FALSE,
                          graphic->projection_matrix);
    }

#if DEBUG
  printf ("graphic_render: drawing the fractal points\n");
  fflush (stdout);
#endif

  // Drawing the fractal points
  glGenBuffers (1, &vbo_points);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_points);
  glBufferData (GL_ARRAY_BUFFER, npoints * sizeof (Point3D), point,
                GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray (graphic->attribute_3D_position);
  glVertexAttribPointer (graphic->attribute_3D_position,
                         3, GL_FLOAT, GL_FALSE, sizeof (Point3D), NULL);
  glEnableVertexAttribArray (graphic->attribute_3D_color);
  glVertexAttribPointer (graphic->attribute_3D_color,
                         3,
                         GL_FLOAT,
                         GL_FALSE,
                         sizeof (Point3D), (GLvoid *) offsetof (Point3D, c));
  glDrawArrays (GL_POINTS, 0, npoints);
  glDeleteBuffers (1, &vbo_points);
  glDisableVertexAttribArray (graphic->attribute_3D_color);
  glDisableVertexAttribArray (graphic->attribute_3D_position);

end_draw:

  // OpenGL properties
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if DEBUG
  printf ("graphic_render: drawing the logo\n");
  fflush (stdout);
#endif

  // Drawing the logo
  image_draw (graphic->logo, 0, 0, window_width, window_height);

#if DEBUG
  printf ("graphic_render: displaying the program version\n");
  fflush (stdout);
#endif

  // Displaying the program version
  sx = 0.15 * 12. / window_width;
  sy = 0.15 * 12. / window_height;
  text_draw (graphic->text, (char *) str_version,
             0.99 - 7. * strlen (str_version) * sx, -0.99, sx, sy, black);

  // Disabling OpenGL properties
  glDisable (GL_BLEND);

#if DEBUG
  printf ("graphic_render: displaying the draw\n");
  fflush (stdout);
#endif

#if DEBUG
  printf ("graphic_render: end\n");
  fflush (stdout);
#endif
}

/**
 * Function to save the draw in a PNG file.
 */
void
graphic_save (char *file_name)  ///< File name.
{
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;
  GLubyte *pixels;
  FILE *file;
  unsigned int i, row_bytes, pointers_bytes, pixels_bytes;

#if DEBUG
  printf ("graphic_save: start\n");
  fflush (stdout);
#endif

  // Creating the PNG header
  png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    return;
  info = png_create_info_struct (png);
  if (!info)
    return;
  file = fopen (file_name, "wb");
  if (!file)
    return;
  if (setjmp (png_jmpbuf (png)))
    {
      printf ("Error png_init_io\n");
      exit (0);
    }
  png_init_io (png, file);
  if (setjmp (png_jmpbuf (png)))
    {
      printf ("Error png_set_IHDR\n");
      exit (0);
    }
  png_set_IHDR (png,
                info,
                window_width,
                window_height,
                8,
                PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  if (setjmp (png_jmpbuf (png)))
    {
      printf ("Error png_write_info\n");
      exit (0);
    }
  png_write_info (png, info);

  // Getting the OpenGL pixels
  glViewport (0, 0, window_width, window_height);
  row_bytes = 4 * window_width;
  pixels_bytes = row_bytes * window_height;
  pixels = (GLubyte *) g_slice_alloc (pixels_bytes);
  glReadPixels (0, 0, window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE,
                pixels);

  // Saving the pixels in the PNG order
  pointers_bytes = window_height * sizeof (png_byte *);
  row_pointers = (png_byte **) g_slice_alloc (pointers_bytes);
  for (i = 0; i < window_height; ++i)
    {
      row_pointers[i] = (png_byte *) g_slice_alloc (row_bytes);
      memcpy (row_pointers[i], pixels + (window_height - 1 - i) * row_bytes,
              row_bytes);
    }
  if (setjmp (png_jmpbuf (png)))
    {
      printf ("Error png_write_image\n");
      exit (0);
    }
  png_write_image (png, row_pointers);

  // Freeing memory
  for (i = 0; i < window_height; ++i)
    g_slice_free1 (row_bytes, row_pointers[i]);
  g_slice_free1 (pointers_bytes, row_pointers);
  g_slice_free1 (pixels_bytes, pixels);

  // Saving the file
  if (setjmp (png_jmpbuf (png)))
    {
      printf ("Error png_write_end\n");
      exit (0);
    }
  png_write_end (png, NULL);
  fclose (file);

  // Freeing memory
  png_destroy_write_struct (&png, &info);

#if DEBUG
  printf ("graphic_save: end\n");
  fflush (stdout);
#endif
}
