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
 * \file graphic.h
 * \brief Header file to define the graphic drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2022, Javier Burguete Tolosa.
 */
#ifndef GRAPHIC__H
#define GRAPHIC__H 1

/**
 * \struct Graphic
 * Struct to define graphic drawing data.
 */
typedef struct
{
  Text text[1];                 ///< Text data.
  GLfloat projection_matrix[16];        ///< Projection matrix.
  Image *logo;                  ///< Logo data.
  float xmin;                   ///< Minimum x-coordinate.
  float xmax;                   ///< Maximum x-coordinate.
  float ymin;                   ///< Minimum y-coordinate.
  float ymax;                   ///< Maximum y-coordinate.
  float phi;                    ///< Horizontal perspective angle (in radians).
  float theta;                  ///< Vertical perspective angle (in radians).
  GLint attribute_3D_position;  ///< 3D variable position.
  GLint attribute_3D_color;     ///< 3D variable color identifier.
  GLint uniform_3D_matrix;      ///< 3D constant matrix.
  GLuint program_3D;            ///< 3D program.
} Graphic;

extern unsigned int window_width;
extern unsigned int window_height;

int graphic_init (Graphic * graphic, char *logo_name);
void graphic_destroy (Graphic * graphic);
void graphic_render (Graphic * graphic);
void graphic_save (char *file_name);

#endif
