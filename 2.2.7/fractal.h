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
 * \file fractal.h
 * \brief Header file to define the fractal data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2014, Javier Burguete Tolosa.
 */
#ifndef FRACTAL__H
#define FRACTAL__H 1

/**
 * \struct Point
 * \brief A struct to define the point data.
 */
typedef struct
{
/**
 * \var x
 * \brief x-coordinate.
 * \var y
 * \brief y-coordinate.
 * \var z
 * \brief z-coordinate.
 * \var c
 * \brief color.
 */
	int x, y, z;
	unsigned int c;
} Point;

/**
 * \enum FractalType
 * \brief A enum to define fractal types.
 * \var FRACTAL_TYPE_TREE
 * \brief Tree fractal type.
 * \var FRACTAL_TYPE_FOREST
 * \brief Forest fractal type.
 * \var FRACTAL_TYPE_NEURON
 * \brief Neuron fractal type.
 */
enum FractalType
{
	FRACTAL_TYPE_TREE = 0,
	FRACTAL_TYPE_FOREST = 1,
	FRACTAL_TYPE_NEURON = 2
};

// PARALLELIZING DATA
extern unsigned int nthreads;
extern GMutex mutex[1];
// END

extern unsigned int fractal_type, fractal_points, fractal_3D, fractal_diagonal;
extern unsigned int width, height, length, area, breaking, simulating;
extern unsigned long t0;

extern float xmin, xmax, ymin, ymax, phid, thetad, phi, theta;

extern unsigned int max_d;
extern unsigned int *medium;
extern GList *list_points;

extern void* (*parallel_fractal)(GRand *rand);

/**
 * \fn inline void perspective(int x, int y, int z, float *X, float *Y)
 * \brief Function the set the perspective of a point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param X
 * \brief Perspective X-coordinate.
 * \param Y
 * \brief Perspective Y-coordinate.
 */
static inline void perspective(int x, int y, int z, float *X, float *Y)
{
	*X = x * cos(phi) - y * sin(phi);
	*Y = z * sin(theta) + (y * cos(phi) + x * sin(phi)) * cos(theta);
}

void fractal_stop();
void medium_start();
void fractal();

#endif
