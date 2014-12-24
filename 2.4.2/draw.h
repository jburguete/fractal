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
 * \file draw.h
 * \brief Header file to define the drawing data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2014, Javier Burguete Tolosa.
 */
#ifndef DRAW__H
#define DRAW__H 1

/**
 * \struct Logo
 * \brief A struct to define the logo.
 */
typedef struct
{
/**
 * \var width
 * \brief Width.
 * \var height
 * \brief Height.
 * \var image
 * \brief Image bytes.
 */
	int width, height;
	GLubyte *image;
} Logo;

extern const float *color3f[15];
extern Logo logo;

/**
 * \fn inline void draw_logo
 * \brief Funtion to draw the logo.
 * \param
 */
static inline void draw_logo()
{
	unsigned int x2, y2;
	x2 = glutGet(GLUT_WINDOW_WIDTH);
	y2 = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, x2, y2);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRasterPos2f(-1.f, -1.f);
	glDrawPixels
		(logo.width, logo.height, GL_RGBA, GL_UNSIGNED_BYTE, logo.image);
}

void logo_new(char *name);

void draw();
void draw_save(char *file_name);

#endif
