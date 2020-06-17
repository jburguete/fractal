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
 * \file image.c
 * \brief Source file to define the image data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <glib.h>
#include <png.h>
#include <GL/glew.h>

#include "config.h"
#include "image.h"

/**
 * Function to read the image on a PNG file.
 *
 * \return pointer to the Image struct data on success, NULL on error.
 */
Image *
image_new (char *name)          ///< Image PNG file name.
{
  const GLfloat matrix[16] = {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f
  };
  const GLfloat vertices[8] = {
    -1.f, 1.f,
    -1.f, -1.f,
    1.f, -1.f,
    1.f, 1.f
  };
  const GLfloat square_texture[8] = {
    0.0, 1.0,
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0
  };
  const GLushort elements[6] = {
    0, 1, 2,
    2, 3, 0
  };
  Image *image;
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;
  FILE *file;
  unsigned int i, row_bytes;

#if DEBUG
  printf ("image_new: start\n");
  fflush (stdout);
#endif

  // initing image
  image = NULL;

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

  // allocating image
  image = (Image *) g_slice_alloc (sizeof (Image));

  // copying pixels in the OpenGL order
  image->width = png_get_image_width (png, info);
  image->height = png_get_image_height (png, info);
  row_bytes = png_get_rowbytes (png, info);
  image->size = row_bytes * image->height;
  image->image = (GLubyte *) g_slice_alloc (image->size);
  if (!image->image)
    {
      g_slice_free1 (sizeof (Image), image);
      image = NULL;
      goto error2;
    }
  row_pointers = png_get_rows (png, info);
  for (i = 0; i < image->height; i++)
    memcpy (image->image + (row_bytes * (image->height - 1 - i)),
            row_pointers[i], row_bytes);

  // setting up matrices and vertices
  memcpy (image->matrix, matrix, 16 * sizeof (GLfloat));
  memcpy (image->vertices, vertices, 8 * sizeof (GLfloat));
  memcpy (image->square_texture, square_texture, 8 * sizeof (GLfloat));
  memcpy (image->elements, elements, 6 * sizeof (GLushort));

error2:
  // closing file
  fclose (file);

error1:
  // freeing memory
  png_destroy_read_struct (&png, &info, NULL);

#if DEBUG
  printf ("image_new: end\n");
  fflush (stdout);
#endif
  return image;
}

/**
 * Function to init the variables used to draw the image.
 *
 * \return 1 on success, 0 on error.
 */
int
image_init (Image * image)      ///< Image struct.
{
  const char *vs_texture_source =
    "attribute highp vec2 position;"
    "attribute highp vec2 texture_position;"
    "varying highp vec2 t_position;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 0.f, 1.f);"
    "t_position = texture_position;}";
  const char *fs_texture_source =
    "varying highp vec2 t_position;"
    "uniform lowp sampler2D texture_image;"
    "void main () {gl_FragColor = texture2D (texture_image, t_position);}";
  const char *vertex_name = "position";
  const char *texture_name = "texture_image";
  const char *texture_position_name = "texture_position";
  const char *matrix_name = "matrix";
  // GLSL version
  const char *version = "#version 120\n";       // OpenGL 2.1
  const char *vs_texture_sources[3] =
    { version, INIT_GL_GLES, vs_texture_source };
  const char *fs_texture_sources[3] =
    { version, INIT_GL_GLES, fs_texture_source };
  const char *error_message;
  GLint k;
  GLuint vs, fs;

#if DEBUG
  printf ("image_init: start\n");
  fflush (stdout);
#endif

  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_texture_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D texture vertex shader";
      goto exit_on_error;
    }

  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_texture_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      glDeleteShader (vs);
      error_message = "unable to compile the texture fragment shader";
      goto exit_on_error;
    }

  image->program_texture = glCreateProgram ();
  glAttachShader (image->program_texture, vs);
  glAttachShader (image->program_texture, fs);
  glLinkProgram (image->program_texture);
  glDeleteShader (fs);
  glDeleteShader (vs);
  glGetProgramiv (image->program_texture, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 2D texture";
      goto exit_on_error;
    }

  glActiveTexture (GL_TEXTURE0);
  glGenTextures (1, &image->id_texture);
  glBindTexture (GL_TEXTURE_2D, image->id_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D,  // target
                0,              // level, 0 = base, no minimap,
                GL_RGBA,        // internalformat
                image->width,   // width
                image->height,  // height
                0,              // border, always 0 in OpenGL ES
                GL_RGBA,        // format
                GL_UNSIGNED_BYTE,       // type
                image->image);  // image data

  image->attribute_texture
    = glGetAttribLocation (image->program_texture, vertex_name);
  if (image->attribute_texture == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  image->attribute_texture_position
    = glGetAttribLocation (image->program_texture, texture_position_name);
  if (image->attribute_texture_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  image->uniform_texture
    = glGetUniformLocation (image->program_texture, texture_name);
  if (image->uniform_texture == -1)
    {
      error_message = "could not bind texture uniform";
      goto exit_on_error;
    }

  image->uniform_matrix
    = glGetUniformLocation (image->program_texture, matrix_name);
  if (image->uniform_matrix == -1)
    {
      error_message = "could not bind uniform";
      goto exit_on_error;
    }

  glGenBuffers (1, &image->vbo);
  glBindBuffer (GL_ARRAY_BUFFER, image->vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (image->vertices), image->vertices,
                GL_STATIC_DRAW);

  glGenBuffers (1, &image->ibo);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, image->ibo);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (image->elements),
                image->elements, GL_STATIC_DRAW);

  glGenBuffers (1, &image->vbo_texture);
  glBindBuffer (GL_ARRAY_BUFFER, image->vbo_texture);
  glBufferData (GL_ARRAY_BUFFER, sizeof (image->square_texture),
                image->square_texture, GL_STATIC_DRAW);

#if DEBUG
  printf ("image_init: end\n");
  fflush (stdout);
#endif
  return 1;

exit_on_error:
  printf ("ERROR! Image: %s\n", error_message);
#if DEBUG
  printf ("image_init: end\n");
  fflush (stdout);
#endif
  return 0;
}

/**
 * Function to free the memory used by the image.
 */
void
image_destroy (Image * image)   ///< Image struct.
{
#if DEBUG
  printf ("image_destroy: start\n");
  fflush (stdout);
#endif

  glDeleteProgram (image->program_texture);
  g_slice_free1 (image->size, image->image);

#if DEBUG
  printf ("image_destroy: end\n");
  fflush (stdout);
#endif
}

/**
 * Function to draw the image.
 */
void
image_draw (Image * image,      ///< Image struct.
            unsigned int x,     ///< Draw x-coordinate.
            unsigned int y,     ///< Draw y-coordinate.
            unsigned int window_width,  ///< Window width.
            unsigned int window_height) ///< Window height.
{
  float cp, sp;
  cp = ((float) image->width) / window_width;
  sp = ((float) image->height) / window_height;
  image->matrix[0] = cp;
  image->matrix[5] = sp;
  image->matrix[12] = cp - 1.f + (2.f * x) / window_width;
  image->matrix[13] = sp - 1.f + (2.f * y) / window_height;
  glUseProgram (image->program_texture);
  glUniformMatrix4fv (image->uniform_matrix, 1, GL_FALSE, image->matrix);
  glUniform1i (image->uniform_texture, 0);
  glBindTexture (GL_TEXTURE_2D, image->id_texture);
  glBindBuffer (GL_ARRAY_BUFFER, image->vbo_texture);
  glEnableVertexAttribArray (image->attribute_texture_position);
  glVertexAttribPointer (image->attribute_texture_position,
                         2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer (GL_ARRAY_BUFFER, image->vbo);
  glEnableVertexAttribArray (image->attribute_texture);
  glVertexAttribPointer (image->attribute_texture, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, image->ibo);
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  glDisableVertexAttribArray (image->attribute_texture);
  glDisableVertexAttribArray (image->attribute_texture_position);
}
