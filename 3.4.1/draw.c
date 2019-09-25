/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2019, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2019, Javier Burguete Tolosa.
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

// Enabling OpenGL
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
#include "simulator.h"

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

GLuint program_3D;              ///< 3D program.
GLint uniform_2D_matrix;        ///< 2D constant matrix.
GLint attribute_3D_position;    ///< 3D variable position
GLint attribute_3D_icolor;      ///< 3D variable color identifier.
GLint uniform_3D_matrix;        ///< 3D constant matrix.
unsigned int window_width = 480;        ///< Graphic window width.
unsigned int window_height = 480;       ///< Graphic window height.

const GLfloat square_texture[8] = {
  0.0, 1.0,
  0.0, 0.0,
  1.0, 0.0,
  1.0, 1.0,
};                              ///< Square texture vertices.

GLuint vbo_texture;             ///< Texture vertex buffer object.
GLuint program_2D_texture;      ///< Texture program.
GLuint id_texture;              ///< Texture identifier.
GLint uniform_texture;          ///< Texture constant.
GLint attribute_texture;        ///< Texture variable.
GLint attribute_texture_position;       ///< Texture variable position.

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
};                              ///< Logo vertices.

GLushort logo_elements[6] = {
  0, 1, 2,
  2, 3, 0
};                              ///< Logo element indices.

GLuint vbo_logo;                ///< Logo vertices buffer object.
GLuint ibo_logo;                ///< Logo indices buffer object.
Logo logo;                      ///< Logo data.

GLuint vbo_text;                ///< Text vertex buffer object.
GLuint program_text;            ///< Text program
GLuint id_text;                 ///< Text identitifier.
GLint attribute_text_position;  ///< Text variable position.
GLint uniform_text;             ///< Text constant.
GLint uniform_color;            ///< Color constant.
FT_Library ft;                  ///< FreeType data.
FT_Face face;                   ///< FreeType face to draw text.

/**
 * Function to read the logo on a PNG file.
 */
void
logo_new (char *name)           ///< Logo PNG file name.
{
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;
  FILE *file;
  unsigned int i, row_bytes;

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
  logo.size = row_bytes * logo.height;
  logo.image = (GLubyte *) g_slice_alloc (logo.size);
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
 * Function to free the memory used by the logo.
 */
void
logo_destroy ()
{
  g_slice_free1 (logo.size, logo.image);
}

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
  const char *vs_2D_texture_source =
    "attribute highp vec2 position;"
    "attribute highp vec2 texture_position;"
    "varying highp vec2 t_position;"
    "uniform highp mat4 matrix;"
    "void main ()"
    "{gl_Position = matrix * vec4 (position, 0.f, 1.f);"
    "  t_position = texture_position;}";
  const char *vs_text_source =
    "attribute highp vec4 position;"
    "varying highp vec2 textcoord;"
    "void main ()"
    "{gl_Position = vec4(position.xy, 0, 1); textcoord = position.zw;}";
  const char *fs_source =
    "varying lowp vec3 fcolor;"
    "void main () {gl_FragColor = vec4 (fcolor, 1.f);}";
  const char *fs_texture_source =
    "varying highp vec2 t_position;"
    "uniform lowp sampler2D texture_logo;"
    "void main () {gl_FragColor = texture2D (texture_logo, t_position);}";
  const char *fs_text_source =
    "varying highp vec2 textcoord;"
    "uniform lowp sampler2D text;"
    "uniform lowp vec4 color;"
    "void main ()"
    "{gl_FragColor = vec4(1, 1, 1, texture2D(text, textcoord).a) * color;}";
  const char *vertex_name = "position";
  const char *color_name = "color";
  const char *texture_position_name = "texture_position";
  const char *matrix_name = "matrix";
  const char *texture_logo_name = "texture_logo";
  const char *text_name = "text";
  const char *vs_2D_sources[3] = { NULL, INIT_GL_GLES, vs_2D_source };
  const char *vs_3D_sources[3] = { NULL, INIT_GL_GLES, vs_3D_source };
  const char *vs_2D_texture_sources[3]
    = { NULL, INIT_GL_GLES, vs_2D_texture_source };
  const char *vs_text_sources[3] = { NULL, INIT_GL_GLES, vs_text_source };
  const char *fs_sources[3] = { NULL, INIT_GL_GLES, fs_source };
  const char *fs_texture_sources[3] = { NULL, INIT_GL_GLES, fs_texture_source };
  const char *fs_text_sources[3] = { NULL, INIT_GL_GLES, fs_text_source };
  const char *version;
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

  // GLSL version
  version = "#version 120\n";   // OpenGL 2.1

#if DEBUG
  printf ("draw_init: compiling 2D vertex shader\n");
  fflush (stdout);
#endif
  vs_2D_sources[0] = version;
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
  fs_sources[0] = version;
  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the fragment shader";
      goto exit_on_error;
    }

  vs_3D_sources[0] = version;
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

  vs_2D_texture_sources[0] = version;
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_2D_texture_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D texture vertex shader";
      goto exit_on_error;
    }

  fs_texture_sources[0] = version;
  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_texture_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D texture fragment shader";
      goto exit_on_error;
    }

  program_2D_texture = glCreateProgram ();
  glAttachShader (program_2D_texture, vs);
  glAttachShader (program_2D_texture, fs);
  glLinkProgram (program_2D_texture);
  glGetProgramiv (program_2D_texture, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 2D texture";
      goto exit_on_error;
    }

  glActiveTexture (GL_TEXTURE0);
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

  attribute_texture = glGetAttribLocation (program_2D_texture, vertex_name);
  if (attribute_texture == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  attribute_texture_position
    = glGetAttribLocation (program_2D_texture, texture_position_name);
  if (attribute_texture_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }

  uniform_texture
    = glGetUniformLocation (program_2D_texture, texture_logo_name);
  if (uniform_texture == -1)
    {
      error_message = "could not bind texture uniform";
      goto exit_on_error;
    }

  vs_text_sources[0] = version;
  vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 3, vs_text_sources, NULL);
  glCompileShader (vs);
  glGetShaderiv (vs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D text vertex shader";
      goto exit_on_error;
    }

  fs_text_sources[0] = version;
  fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 3, fs_text_sources, NULL);
  glCompileShader (fs);
  glGetShaderiv (fs, GL_COMPILE_STATUS, &k);
  if (!k)
    {
      error_message = "unable to compile the 2D text fragment shader";
      goto exit_on_error;
    }

  program_text = glCreateProgram ();
  glAttachShader (program_text, vs);
  glAttachShader (program_text, fs);
  glLinkProgram (program_text);
  glGetProgramiv (program_text, GL_LINK_STATUS, &k);
  if (!k)
    {
      error_message = "unable to link the program 2D text";
      goto exit_on_error;
    }

  attribute_text_position = glGetAttribLocation (program_text, vertex_name);
  if (attribute_text_position == -1)
    {
      error_message = "could not bind attribute";
      goto exit_on_error;
    }
  uniform_text = glGetUniformLocation (program_text, text_name);
  if (uniform_text == -1)
    {
      error_message = "could not bind text uniform";
      goto exit_on_error;
    }
  uniform_color = glGetUniformLocation (program_text, color_name);
  if (uniform_color == -1)
    {
      error_message = "could not bind color uniform";
      goto exit_on_error;
    }

  if (FT_New_Face (ft, "/usr/share/fonts/truetype/FreeSans.ttf", 0,
                   &face)
      && FT_New_Face (ft, "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 0,
                      &face)
      && FT_New_Face (ft, "/usr/share/fonts/TrueType/freefont/FreeSans.ttf", 0,
                      &face)
      && FT_New_Face (ft, "/usr/share/fonts/gnu-free/FreeSans.ttf", 0,
                      &face)
      && FT_New_Face (ft, "/usr/pkg/share/fonts/X11/TTF/FreeSans.ttf", 0,
                      &face)
      && FT_New_Face (ft, "/usr/share/fonts/TTF/FreeSans.ttf", 0,
                      &face)
      && FT_New_Face (ft, "/usr/local/share/texmf-dist/fonts/truetype/public"
                      "/gnu-freefont/FreeSans.ttf", 0, &face)
      && FT_New_Face (ft, "/usr/local/share/fonts/freefont/FreeSans.ttf", 0,
                      &face))
    {
      error_message = "could not open font";
      goto exit_on_error;
    }
  FT_Set_Pixel_Sizes (face, 0, 12);

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

  glGenBuffers (1, &vbo_text);

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
 * Function to draw a text.
 */
static inline void
draw_text (char *text,          ///< Text string.
           float x,             ///< x initial coordinate.
           float y,             ///< y initial coordinate.
           float sx,            ///< x scale factor.
           float sy,            ///< y scale factor.
           const GLfloat * color)       ///< array of RBGA colors.
{
  float x2, y2, w, h, box[16];
  glUseProgram (program_text);
  glGenTextures (1, &id_text);
  glBindTexture (GL_TEXTURE_2D, id_text);
  glUniform1i (uniform_text, 0);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glUniform4fv (uniform_color, 1, color);
  glEnableVertexAttribArray (attribute_text_position);
  glBindBuffer (GL_ARRAY_BUFFER, vbo_text);
  glVertexAttribPointer (attribute_text_position, 4, GL_FLOAT, GL_FALSE, 0, 0);
  for (; *text; ++text)
    {
      if (FT_Load_Char (face, *text, FT_LOAD_RENDER))
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
      box[2] = 0.;
      box[3] = 0.;
      box[4] = x2 + w;
      box[5] = -y2;
      box[6] = 1.;
      box[7] = 0.;
      box[8] = x2;
      box[9] = -y2 - h;
      box[10] = 0.;
      box[11] = 1.;
      box[12] = x2 + w;
      box[13] = -y2 - h;
      box[14] = 1.;
      box[15] = 1.;
      glBufferData (GL_ARRAY_BUFFER, sizeof (box), box, GL_DYNAMIC_DRAW);
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
      x += (face->glyph->advance.x >> 6) * sx;
      y += (face->glyph->advance.y >> 6) * sy;
    }
  glDisableVertexAttribArray (attribute_text_position);
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
  glfwGetFramebufferSize (window, &graphic_width, &graphic_height);
  draw_resize (graphic_width, graphic_height);
#endif

#if DEBUG
  printf ("draw: start\n");
  fflush (stdout);
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

  sx = 2. / window_width;
  sy = 2. / window_height;
  draw_text ("Fractal 3.4.1", 1. - 90. * sx, -0.99, sx, sy, black);

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
