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
 * \file draw.h
 * \brief Header file to define the drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#ifndef DRAW__H
#define DRAW__H 1

extern unsigned int window_width;
extern unsigned int window_height;
extern FT_Library ft;
extern FT_Face face;

int draw_init ();
void draw_resize (int width, int height);
void draw ();

/**
 * Function to save the draw in a PNG file.
 */
static inline void
draw_save (char *file_name)     ///< File name.
{
  png_struct *png;
  png_info *info;
  png_byte **row_pointers;
  GLubyte *pixels;
  FILE *file;
  unsigned int i, row_bytes, pointers_bytes, pixels_bytes;

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
}

#endif
