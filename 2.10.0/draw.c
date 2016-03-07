/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2016, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2016, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <png.h>
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
#include "draw.h"

#define INIT_GL_GLES \
	"#ifdef GL_ES\n" \
	"#  ifdef GL_FRAGMENT_PRECISION_HIGH\n" \
	"     precision highp float;\n" \
	"#  else\n" \
	"     precision mediump float;\n" \
	"#  endif\n" \
	"#else\n" \
	"#  define lowp\n" \
	"#  define mediump\n" \
	"#  define highp\n" \
	"#endif\n"

GLuint program_3D;
GLint uniform_2D_matrix;
GLint attribute_3D_position;
GLint attribute_3D_icolor;
GLint uniform_3D_matrix;
unsigned int window_width = 480;        ///< Graphic window width.
unsigned int window_height = 480;       ///< Graphic window height.

const GLfloat square_texture[8] = {
  0.0, 1.0,
  0.0, 0.0,
  1.0, 0.0,
  1.0, 1.0,
};

GLuint vbo_texture;             ///< Texture vertex buffer object.
GLuint program_2D_texture;
GLuint id_texture;
GLint uniform_texture;
GLint attribute_texture;
GLint attribute_texture_position;

GLfloat projection_matrix[16] = {
  1., 0., 0., 0.,
  0, 1., 0., 0.,
  0., 0, 0., 0.,
  0., 0., 0., 1.
};                              ///< Projection matrix.

GLfloat logo_matrix[16] = {
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, -1.f, 1.f
};                              ///< Logo projection matrix.

const GLfloat logo_vertices[8] = {
  -1.f, 1.f,
  -1.f, -1.f,
  1.f, -1.f,
  1.f, 1.f,
};                              /// Logo vertices.

GLushort logo_elements[6] = {
  0, 1, 2,
  2, 3, 0
};                              ///< Logo element indices.

GLuint vbo_logo;                ///< Logo vertices buffer object.
GLuint ibo_logo;                ///< Logo indices buffer object.
Logo logo;                      ///< Logo data.

/**
 * \fn void logo_new(char *name)
 * \brief Function to read the logo on a PNG file.
 * \param name
 * \brief Logo PNG file name.
 */
void
logo_new (char *name)
{
  unsigned int row_bytes;
  int i;
  FILE *file;
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;

  // starting png structs
  png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct (png);

  // opening file
  file = fopen (name, "rb");
  if (!file)
    goto error1;

  // reading file and transforming to 8 bits RGBA format
  if (setjmp (png_jmpbuf (png)))
    goto error2;
  png_init_io (png, file);
  if (setjmp (png_jmpbuf (png)))
    goto error2;
  png_read_png (png,
                info,
                PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING |
                PNG_TRANSFORM_EXPAND, NULL);

  // copying pixels in the OpenGL order
  logo.width = png_get_image_width (png, info);
  logo.height = png_get_image_height (png, info);
  row_bytes = png_get_rowbytes (png, info);
  logo.image = (GLubyte *) g_malloc (row_bytes * logo.height);
  if (!logo.image)
    goto error2;
  row_pointers = png_get_rows (png, info);
  for (i = 0; i < logo.height; i++)
    memcpy (logo.image + (row_bytes * (logo.height - 1 - i)),
            row_pointers[i], row_bytes);

error2:
  // closing file
  fclose (file);

error1:
  // freeing memory
  png_destroy_read_struct (&png, &info, NULL);
}

/**
 * \fn int draw_init ()
 * \brief Function to init the graphic data.
 * \return 1 on success, 0 on error.
 */
int
draw_init ()
{
  const char *vs_2D_source =
    "attribute highp vec2 position;"
    "attribute lowp vec3 icolor;"
    "varying lowp vec3 fcolor;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 0.f, 1.f); fcolor = icolor;}";
  const char *vs_3D_source =
    "attribute highp vec3 position;"
    "attribute lowp vec3 icolor;"
    "varying lowp vec3 fcolor;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 1.f); fcolor = icolor;}";
  const char *vs_2D_texture_source =
    "attribute highp vec2 position;"
    "attribute highp vec2 texture_position;"
    "varying highp vec2 t_position;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 0.f, 1.f);"
    "  t_position = texture_position;}";
  const char *fs_source =
    "varying lowp vec3 fcolor;"
    "void main () {gl_FragColor = vec4 (fcolor, 1.f);}";
  const char *fs_texture_source =
    "varying highp vec2 t_position;"
    "uniform lowp sampler2D texture_logo;"
    "void main () {gl_FragColor = texture2D (texture_logo, t_position);}";
  const char *attribute_vertex_name = "position";
  const char *attribute_color_name = "icolor";
  const char *attribute_texture_name = "texture_position";
  const char *uniform_matrix_name = "matrix";
  const char *uniform_texture_name = "texture_logo";
  const char *vs_2D_sources[3] = { NULL, INIT_GL_GLES, vs_2D_source };
  const char *vs_3D_sources[3] = { NULL, INIT_GL_GLES, vs_3D_source };
  const char *vs_2D_texture_sources[3]
    = { NULL, INIT_GL_GLES, vs_2D_texture_source };
  const char *fs_sources[3] = { NULL, INIT_GL_GLES, fs_source };
  const char *fs_texture_sources[3] = { NULL, INIT_GL_GLES, fs_texture_source };
  const char *version;
  GLint k;
  GLuint vs, fs, fs_texture;

#if DEBUG
  printf ("draw_init: start\n");
#endif

  // GLSL version
  version = "#version 120\n";   // OpenGL 2.1

#if DEBUG
  printf ("draw_init: compiling 2D vertex shader\n");
#endif
  vs_2D_sources[0] = version;
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_2D_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to compile the 2D vertex shader\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

#if DEBUG
  printf ("draw_init: compiling 2D vertex shader\n");
#endif
  fs_sources[0] = version;
  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to compile the fragment shader\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  vs_3D_sources[0] = version;
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_3D_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to compile the 3D vertex shader\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  program_3D = glCreateProgram ();
  glAttachShader (program_3D, vs);
  glAttachShader (program_3D, fs);
  glLinkProgram (program_3D);
  glGetProgramiv (program_3D, GL_LINK_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to link the program 3D\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  attribute_3D_position
    = glGetAttribLocation (program_3D, attribute_vertex_name);
  if (attribute_3D_position == -1)
    {
      printf ("ERROR! could not bind cube attribute %s\n",
              attribute_vertex_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  attribute_3D_icolor = glGetAttribLocation (program_3D, attribute_color_name);
  if (attribute_3D_icolor == -1)
    {
      printf ("ERROR! could not bind cube attribute %s\n",
              attribute_color_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  uniform_3D_matrix = glGetUniformLocation (program_3D, uniform_matrix_name);
  if (uniform_3D_matrix == -1)
    {
      printf ("ERROR! could not bind cube uniform %s\n", uniform_matrix_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  glGenBuffers (1, &vbo_logo);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_logo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (logo_vertices), logo_vertices,
                GL_STATIC_DRAW);

  glGenBuffers (1, &ibo_logo);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_logo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (logo_elements), logo_elements,
                GL_STATIC_DRAW);

  glGenBuffers (1, &vbo_texture);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_texture);
  glBufferData (GL_ARRAY_BUFFER, sizeof (square_texture), square_texture,
                GL_STATIC_DRAW);

  vs_2D_texture_sources[0] = version;
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_2D_texture_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to compile the 2D texture vertex shader\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  fs_texture_sources[0] = version;
  fs_texture = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs_texture, 3, fs_texture_sources, NULL);
  glCompileShader (fs_texture);
  glGetShaderiv (fs_texture, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to compile the 2D texture fragment shader\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  program_2D_texture = glCreateProgram ();
  glAttachShader (program_2D_texture, vs);
  glAttachShader (program_2D_texture, fs_texture);
  glLinkProgram (program_2D_texture);
  glGetProgramiv (program_2D_texture, GL_LINK_STATUS, &k);
  if (!k)
    {
      printf ("ERROR! unable to link the program 2D texture\n");
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  glGenTextures (1, &id_texture);
  glBindTexture (GL_TEXTURE_2D, id_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D,  // target
                0,              // level, 0 = base, no minimap,
                GL_RGBA,        // internalformat
                logo.width,     // width
                logo.height,    // height
                0,              // border, always 0 in OpenGL ES
                GL_RGBA,        // format
                GL_UNSIGNED_BYTE,       // type
                logo.image);

  attribute_texture
    = glGetAttribLocation (program_2D_texture, attribute_vertex_name);
  if (attribute_texture == -1)
    {
      printf ("ERROR! could not bind attribute %s\n", attribute_vertex_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  attribute_texture_position
    = glGetAttribLocation (program_2D_texture, attribute_texture_name);
  if (attribute_texture_position == -1)
    {
      printf ("ERROR! could not bind attribute %s\n", attribute_texture_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

  uniform_texture
    = glGetUniformLocation (program_2D_texture, uniform_texture_name);
  if (uniform_texture == -1)
    {
      printf ("ERROR! could not bind texture uniform %s\n",
              uniform_texture_name);
#if DEBUG
      printf ("draw_init: end\n");
#endif
      return 0;
    }

#if DEBUG
  printf ("draw_init: end\n");
#endif
  return 1;
}

/**
 * \fn void draw_resize (int width, int height)
 * \brief Function to updating window data when resizing.
 * \param width
 * \brief Graphic window width.
 * \param height
 * \brief Graphic window height.
 */
void
draw_resize (int width, int height)
{
  window_width = width;
  window_height = height;
  glViewport (0, 0, width, height);
}

/**
 * \fn void draw()
 * \brief Function to draw the fractal.
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
  float cp, sp, ct, st, w, h;
  GLuint vbo_square, ibo_square, vbo_points;

#if HAVE_GLFW
  int graphic_width, graphic_height;
  glfwGetFramebufferSize (window, &graphic_width, &graphic_height);
  draw_resize (graphic_width, graphic_height);
#endif

  // Drawing a white background
  glClearColor (1., 1., 1., 0.);
  glClear (GL_COLOR_BUFFER_BIT);

  // Ending if no fractal
  if (!medium)
    goto end_draw;

  // Checking if 3D or 2D fractal
  glUseProgram (program_3D);
  if (fractal_3D)
    {
      // Projection matrix
      sincosf (phi, &sp, &cp);
      sincosf (theta, &st, &ct);
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

  // Drawing the logo
  cp = ((float) logo.width) / window_width;
  sp = ((float) logo.height) / window_height;
  logo_matrix[0] = cp;
  logo_matrix[5] = sp;
  logo_matrix[12] = cp - 1.f;
  logo_matrix[13] = sp - 1.f;
  glUseProgram (program_2D_texture);
  glUniformMatrix4fv (uniform_2D_matrix, 1, GL_FALSE, logo_matrix);
  glActiveTexture (GL_TEXTURE0);
  glUniform1i (uniform_texture, 0);
  glBindTexture (GL_TEXTURE_2D, id_texture);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_texture);
  glEnableVertexAttribArray (attribute_texture_position);
  glVertexAttribPointer (attribute_texture_position,
                         2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_logo);
  glEnableVertexAttribArray (attribute_texture);
  glVertexAttribPointer (attribute_texture, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo_logo);
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  glDisableVertexAttribArray (attribute_3D_position);
  glDisableVertexAttribArray (attribute_texture_position);

  // Displaying the draw
#if HAVE_FREEGLUT
  glutSwapBuffers ();
#elif HAVE_SDL
  SDL_GL_SwapWindow (window);
#elif HAVE_GLFW
  glfwSwapBuffers (window);
#endif
}

/**
 * \fn void draw_save(char *file_name)
 * \brief Function to save the draw in a PNG file.
 * \param file_name
 * \brief File name.
 */
void
draw_save (char *file_name)
{
  int i;
  unsigned int row_bytes, pointers_bytes, pixels_bytes;
  GLubyte *pixels;
  FILE *file;
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;

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
  row_pointers = (png_byte **) malloc (pointers_bytes);
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
}
