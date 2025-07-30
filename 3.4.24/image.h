/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2025, Javier Burguete Tolosa.

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
 * \file image.h
 * \brief Header file to define the image data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2025, Javier Burguete Tolosa.
 */
#ifndef IMAGE__H
#define IMAGE__H 1

/**
 * Macro to enable OpenGL ES.
 */
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

/**
 * \struct Image
 * \brief A struct to define the image.
 */
typedef struct
{
  GLfloat matrix[16];           ///< Projection matrix.
  GLfloat vertices[8];          ///< Vertices.
  GLfloat square_texture[8];    ///< Square texture vertices.
  GLushort elements[6];         ///< Element indices.
  GLubyte *image;               ///< Image bytes.
  GLint uniform_texture;        ///< Texture constant.
  GLint attribute_texture;      ///< Texture variable.
  GLint attribute_texture_position;     ///< Texture variable position.
  GLint uniform_matrix;         ///< Projection matrix.
  GLuint vbo;                   ///< Vertices buffer object.
  GLuint ibo;                   ///< Indices buffer object.
  GLuint vbo_texture;           ///< Texture vertex buffer object.
  GLuint program_texture;       ///< Texture program.
  GLuint id_texture;            ///< Texture identifier.
  unsigned int width;           ///< Width.
  unsigned int height;          ///< Height.
  unsigned int size;            ///< Size in bytes.
} Image;

Image *image_new (char *name);
void image_destroy (Image * image);
int image_init (Image * image);
void image_draw (Image * image, unsigned int x, unsigned int y,
                 unsigned int window_width, unsigned int window_height);

#endif
