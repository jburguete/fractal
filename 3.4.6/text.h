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
 * \file text.h
 * \brief Header file to define the text drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#ifndef TEXT__H
#define TEXT__H 1

/**
 * \struct Text
 * Struct to define data to draw text.
 */
typedef struct
{
  FT_Library ft;                ///< FreeType data.
  FT_Face face;                 ///< FreeType face to draw text.
  GLint attribute_position;     ///< Text variable position.
  GLint uniform_text;           ///< Text constant.
  GLint uniform_color;          ///< Color constant.
  GLuint vbo;                   ///< Text vertex buffer object.
  GLuint program;               ///< Text program.
} Text;

int text_init (Text * text);
void text_destroy (Text * text);
void text_draw (Text * text, char *string, float x, float y, float sx, float sy,
                const GLfloat * color);

#endif
