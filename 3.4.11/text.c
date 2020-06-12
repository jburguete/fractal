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
 * \file text.c
 * \brief Source file to define the text drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <GL/glew.h>

#include "config.h"
#include "image.h"
#include "text.h"

/**
 * Function to init the variables used to draw text.
 *
 * \return 1 on success, 0 on error.
 */
int
text_init (Text * text)         ///< Text struct data.
{
  const char *vs_source =
    "attribute highp vec4 position;"
    "varying highp vec2 textcoord;"
    "void main ()"
    "{gl_Position = vec4(position.xy, 0, 1); textcoord = position.zw;}";
  const char *fs_source =
    "varying highp vec2 textcoord;"
    "uniform lowp sampler2D text;"
    "uniform lowp vec4 color;"
    "void main ()"
    "{gl_FragColor = vec4(1, 1, 1, texture2D(text, textcoord).a) * color;}";
  const char *vertex_name = "position";
  const char *color_name = "color";
  const char *text_name = "text";
  // GLSL version
  const char *version = "#version 120\n";       // OpenGL 2.1
  const char *vs_sources[3] = { version, INIT_GL_GLES, vs_source };
  const char *fs_sources[3] = { version, INIT_GL_GLES, fs_source };
  const char *error_message;
  GLint k;
  GLuint vs, fs;

  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D text vertex shader";
      goto exit_on_error;
    }

  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      glDeleteShader (vs);
      error_message = "unable to compile the 2D text fragment shader";
      goto exit_on_error;
    }

  text->program = glCreateProgram ();
  glAttachShader (text->program, vs);
  glAttachShader (text->program, fs);
  glLinkProgram (text->program);
  glDeleteShader (fs);
  glDeleteShader (vs);
  glGetProgramiv (text->program, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 2D text";
      goto exit_on_error;
    }

  text->attribute_position = glGetAttribLocation (text->program, vertex_name);
  if (text->attribute_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }
  text->uniform_text = glGetUniformLocation (text->program, text_name);
  if (text->uniform_text == -1)
    {
      error_message = "could not bind text uniform";
      goto exit_on_error;
    }
  text->uniform_color = glGetUniformLocation (text->program, color_name);
  if (text->uniform_color == -1)
    {
      error_message = "could not bind color uniform";
      goto exit_on_error;
    }

  // Initing FreeType
  if (FT_Init_FreeType (&text->ft))
    {
      error_message = "could not init freetype library";
      goto exit_on_error;
    }
  if (FT_New_Face (text->ft, FONT, 0, &text->face))
    {
      error_message = "could not open font";
      goto exit_on_error;
    }
  FT_Set_Pixel_Sizes (text->face, 0, 12);

  glGenBuffers (1, &text->vbo);
  return 1;

exit_on_error:
  printf ("ERROR! Text: %s\n", error_message);
  return 0;
}

/**
 * Function to free the memory used to draw text.
 */
void
text_destroy (Text * text)      ///< Text struct data.
{
#if DEBUG
  printf ("text_destroy: start\n");
  fflush (stdout);
#endif

  FT_Done_Face (text->face);
  FT_Done_Library (text->ft);
  glDeleteProgram (text->program);

#if DEBUG
  printf ("text_destroy: end\n");
  fflush (stdout);
#endif
}

/**
 * Function to draw a string.
 */
void
text_draw (Text * text,         ///< Text struct data.
           char *string,        ///< String.
           float x,             ///< x initial coordinate.
           float y,             ///< y initial coordinate.
           float sx,            ///< x scale factor.
           float sy,            ///< y scale factor.
           const GLfloat * color)       ///< array of RBGA colors.
{
  float box[16];
  FT_Face face;
  float x2, y2, w, h;
  GLuint id;

#if DEBUG
  printf ("text_draw: start\n");
  fflush (stdout);
#endif

  glUseProgram (text->program);
  glGenTextures (1, &id);
  glBindTexture (GL_TEXTURE_2D, id);
  glUniform1i (text->uniform_text, 0);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glUniform4fv (text->uniform_color, 1, color);
  glEnableVertexAttribArray (text->attribute_position);
  glBindBuffer (GL_ARRAY_BUFFER, text->vbo);
  glVertexAttribPointer (text->attribute_position, 4, GL_FLOAT, GL_FALSE, 0, 0);
  face = text->face;
  box[2] = 0.f;
  box[3] = 0.f;
  box[6] = 1.f;
  box[7] = 0.f;
  box[10] = 0.f;
  box[11] = 1.f;
  box[14] = 1.f;
  box[15] = 1.f;
  for (; *string; ++string)
    {
      if (FT_Load_Char (face, *string, FT_LOAD_RENDER))
        continue;
      glTexImage2D (GL_TEXTURE_2D,
                    0,
                    GL_ALPHA,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0, GL_ALPHA, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
      x2 = x + face->glyph->bitmap_left * sx;
      y2 = -y - face->glyph->bitmap_top * sy;
      w = face->glyph->bitmap.width * sx;
      h = face->glyph->bitmap.rows * sy;
      box[0] = x2;
      box[1] = -y2;
      box[4] = x2 + w;
      box[5] = -y2;
      box[8] = x2;
      box[9] = -y2 - h;
      box[12] = x2 + w;
      box[13] = -y2 - h;
      glBufferData (GL_ARRAY_BUFFER, sizeof (box), box, GL_DYNAMIC_DRAW);
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
      x += (face->glyph->advance.x >> 6) * sx;
      y += (face->glyph->advance.y >> 6) * sy;
    }
  glDisableVertexAttribArray (text->attribute_position);

#if DEBUG
  printf ("text_draw: end\n");
  fflush (stdout);
#endif
}
