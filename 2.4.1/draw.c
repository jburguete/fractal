/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2014, Javier Burguete Tolosa.

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
 * \copyright Copyright 2009-2014, Javier Burguete Tolosa.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <png.h>
#include <GL/freeglut.h>
#include "config.h"
#include "fractal.h"
#include "draw.h"

/**
 * \var color0
 * \brief Color 0.
 * \var color1
 * \brief Color 1.
 * \var color2
 * \brief Color 2.
 * \var color3
 * \brief Color 3.
 * \var color4
 * \brief Color 4.
 * \var color5
 * \brief Color 5.
 * \var color6
 * \brief Color 6.
 * \var color7
 * \brief Color 7.
 * \var color8
 * \brief Color 8.
 * \var color9
 * \brief Color 9.
 * \var color10
 * \brief Color 10.
 * \var color11
 * \brief Color 11.
 * \var color12
 * \brief Color 12.
 * \var color13
 * \brief Color 13.
 * \var color14
 * \brief Color 14.
 */
const float color0[3] = {1., 0., 0.};
const float color1[3] = {0., 1., 0.};
const float color2[3] = {0., 0., 1.};
const float color3[3] = {0.5, 0.5, 0.};
const float color4[3] = {0.5, 0., 0.5};
const float color5[3] = {0., 0.5, 0.5};
const float color6[3] = {0.5, 0.25, 0.25};
const float color7[3] = {0.25, 0.5, 0.25};
const float color8[3] = {0.25, 0.25, 0.5};
const float color9[3] = {0.75, 0.25, 0.};
const float color10[3] = {0.75, 0., 0.25};
const float color11[3] = {0.25, 0.75, 0.};
const float color12[3] = {0., 0.75, 0.25};
const float color13[3] = {0.25, 0., 0.75};
const float color14[3] = {0., 0.25, 0.75};

/**
 * \var color3f
 * \brief Array of color pointers.
 */
const float *color3f[15] =
{
	color0, color1, color2, color3, color4, color5, color6, color7,
	color8, color9, color10, color11, color12, color13, color14
};

/**
 * \var logo
 * \brief Logo data.
 */
Logo logo;

/**
 * \fn void logo_new(char *name)
 * \brief Function to read the logo on a PNG file.
 * \param name
 * \brief Logo PNG file name.
 */
void logo_new(char *name)
{
	unsigned int row_bytes;
    int i;
    FILE *file;
    png_struct *png;
    png_info *info;
    png_byte **row_pointers;

	// starting png structs
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info= png_create_info_struct(png);

	// opening file
	file = fopen(name, "rb");
	if (!file) goto error1;

	// reading file and transforming to 8 bits RGBA format
	if (setjmp(png_jmpbuf(png))) goto error2;
	png_init_io(png, file);
	if (setjmp(png_jmpbuf(png))) goto error2;
	png_read_png(png,
		info,
		PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND,
		NULL);

	// copying pixels in the OpenGL order
	logo.width = png_get_image_width(png, info);
	logo.height = png_get_image_height(png, info);
	row_bytes = png_get_rowbytes(png, info);
	logo.image = (GLubyte*)g_malloc(row_bytes * logo.height);
	if (!logo.image) goto error2;
	row_pointers = png_get_rows(png, info);
	for (i = 0; i < logo.height; i++)
		memcpy(logo.image + (row_bytes * (logo.height - 1 - i)),
			row_pointers[i],
			row_bytes);

error2:
	// closing file
	fclose(file);

error1:
	// freeing memory
	png_destroy_read_struct(&png, &info, NULL);
}

/**
 * \fn void draw()
 * \brief Function to draw the fractal.
 */
void draw()
{
	unsigned int x2, y2;
	// Projection matrix
	float m[16]=
	{
		cos(phi),  cos(theta) * sin(phi), 0., 0.,
		-sin(phi), cos(theta) * cos(phi), 0., 0.,
		0.,        sin(theta),            0., 0.,
		0.,        0.,                    0., 1.
	};
	GList *list;
	Point *point;

	// Drawing a white background
	glClearColor(1., 1., 1., 0.);
	glClear(GL_COLOR_BUFFER_BIT);

	// Ending if no fractal
	if (!medium) goto end_draw;

	// Window size
	x2 = glutGet(GLUT_WINDOW_WIDTH);
	y2 = glutGet(GLUT_WINDOW_HEIGHT);

	// Setting the view
	glViewport(0, 0, x2, y2);

	// Setting the model matrix
	glMatrixMode(GL_MODELVIEW);

	// Checking if 3D or 2D fractal
	if (fractal_3D)
	{
		// 3D fractal requires projection
		glLoadMatrixf(m);

		// Setting the projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(xmin, xmax, ymin, ymax, -1., 1.);

		// Drawing a black rectangle
		glColor3f(0., 0., 0.);
		glBegin(GL_LINE_LOOP);
			glVertex2i(0, 0);
			glVertex2i(0, width);
			glVertex2i(length, width);
			glVertex2i(length, 0);
		glEnd();

		// Drawing the fractal points
		glBegin(GL_POINTS);
			for (list = g_list_last(list_points); list; list = list->prev)
			{
				point = (Point*)list->data;
				glColor3fv(color3f[point->c - 1]);
				glVertex3iv((int*)point);
			}
		glEnd();
	}
	else
	{
		// No projection
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0., width,0., height, -1., 1.);

		// Drawing the fractal points
		glBegin(GL_POINTS);
			for (list = g_list_last(list_points); list; list = list->prev)
			{
				point = (Point*)list->data;
				glColor3fv(color3f[point->c - 1]);
				glVertex2iv((int*)point);
			}
		glEnd();
	}

end_draw:

	// Drawing the logo
	draw_logo();

	// Displaying the draw
	glutSwapBuffers();
}

/**
 * \fn void draw_save(char *file_name)
 * \brief Function to save the draw in a PNG file.
 * \param file_name
 * \brief File name.
 */
void draw_save(char *file_name)
{
	int i, x2, y2;
	unsigned int row_bytes, pointers_bytes, pixels_bytes;
	GLubyte *pixels;
	FILE *file;
	png_struct *png;
	png_info *info;
	png_byte **row_pointers;

	// Getting the window size
	x2 = glutGet(GLUT_WINDOW_WIDTH);
	y2 = glutGet(GLUT_WINDOW_HEIGHT);

	// Creating the PNG header
	png	= png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) return;
	info = png_create_info_struct(png);
	if (!info) return;
	file = fopen(file_name, "wb");
	if (!file) return;
	if (setjmp(png_jmpbuf(png)))
	{
		printf("Error png_init_io\n");
		exit(0);
	}
	png_init_io(png, file);
	if (setjmp(png_jmpbuf(png)))
	{
		printf("Error png_set_IHDR\n");
		exit(0);
	}
	png_set_IHDR(
		png,
		info,
		x2,
		y2,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	if (setjmp(png_jmpbuf(png)))
	{
		printf("Error png_write_info\n");
		exit(0);
	}
	png_write_info(png, info);

	// Getting the OpenGL pixels
	glViewport(0, 0, x2, y2);
	row_bytes = 4 * x2;
	pixels_bytes = row_bytes * y2;
	pixels = (GLubyte*)g_slice_alloc(pixels_bytes);
	glReadPixels
		(0, 0, x2, y2, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Saving the pixels in the PNG order
	pointers_bytes = y2 * sizeof(png_byte*);
	row_pointers = (png_byte**)malloc(pointers_bytes);
	for (i = 0; i < y2; ++i)
	{
		row_pointers[i] = (png_byte*)g_slice_alloc(row_bytes);
		memcpy(row_pointers[i], pixels + (y2 - 1 - i) * row_bytes,
			row_bytes);
	}
	if (setjmp(png_jmpbuf(png)))
	{
		printf("Error png_write_image\n");
		exit(0);
	}
	png_write_image(png, row_pointers);

	// Freeing memory
	for (i = 0; i < y2; ++i) g_slice_free1(row_bytes, row_pointers[i]);
	g_slice_free1(pointers_bytes, row_pointers);
	g_slice_free1(pixels_bytes, pixels);

	// Saving the file
	if (setjmp(png_jmpbuf(png)))
	{
		printf("Error png_write_end\n");
		exit(0);
	}
	png_write_end(png, NULL);
	fclose(file);

	// Freeing memory
	png_destroy_write_struct(&png, &info);
}
