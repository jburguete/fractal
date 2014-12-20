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
 * \file fractal.c
 * \brief Source file to define the fractal data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2014, Javier Burguete Tolosa.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <GL/freeglut.h>
#include "config.h"
#include "fractal.h"
#include "draw.h"
#include "simulator.h"

/**
 * \var width
 * \brief Medium width.
 * \var height
 * \brief Medium height.
 * \var length
 * \brief Medium length.
 * \var area
 * \brief Medium area.
 * \var breaking
 * \brief 1 on breaking, 0 otherwise.
 * \var simulating
 * \brief 1 on simulating, 0 otherwise.
 * \var animating
 * \brief 1 on animating, 0 otherwise.
 */
unsigned int width = 320;
unsigned int height = 200;
unsigned int length = 320;
unsigned int area;
unsigned int breaking = 0;
unsigned int simulating = 0;
unsigned int animating = 1;

/**
 * \var fractal_type
 * \brief Fractal type.
 * \var fractal_points
 * \brief Fractal points number.
 * \var fractal_3D
 * \brief 1 on 3D fractals, 0 on 2D fractals.
 * \var fractal_diagonal
 * \brief 1 on diagonal point movement, 0 otherwise.
 */
unsigned int fractal_type = FRACTAL_TYPE_TREE;
unsigned int fractal_points = 0;
unsigned int fractal_3D = 0;
unsigned int fractal_diagonal = 0;

/**
 * \var t0
 * \brief Computational time.
 */
unsigned long t0;

/**
 * \var xmin
 * \brief Minimun x-coordinate of the view.
 * \var xmax
 * \brief Maximun x-coordinate of the view.
 * \var ymin
 * \brief Minimun y-coordinate of the view.
 * \var ymax
 * \brief Maximun y-coordinate of the view.
 * \var phi
 * \brief Horizontal perspective angle (in radians).
 * \var theta
 * \brief Vertical perspective angle (in radians).
 * \var phid
 * \brief Horizontal perspective angle (in degrees).
 * \var thetad
 * \brief Vertical perspective angle (in degrees).
 */
float xmin, xmax, ymin, ymax, phi, theta;
float phid = -45.;
float thetad = 80.;

/**
 * \var max_d
 * \brief Maximum fractal size.
 * \var medium
 * \brief Array of fractal points.
 * \var list_points
 * \brief GList of points.
 */
unsigned int max_d = 0;
unsigned int *medium = NULL;
GList *list_points = NULL;

/**
 * \var random_algorithm
 * \brief Type of random numbers generator algorithm.
 * \var random_seed_type
 * \brief Type of random seed.
 * \var random_seed
 * \brief Random seed.
 * \var random_type
 * \brief Array of gsl_rng_type* random numbers generator algorithms.
 */
unsigned int random_algorithm = 0;
unsigned int random_seed_type = 1;
unsigned long random_seed = 7007L;
/**
 * \var parallel_fractal
 * \brief Pointer to the function to calculate the fractal.
 */
void* (*parallel_fractal)(gsl_rng *rng);

/**
 * \var dialog_options
 * \brief DialogOptions to set the fractal options.
 * \var dialog_simulator
 * \brief DialogSimulator to show the main window.
 */
DialogOptions dialog_options;
DialogSimulator dialog_simulator;

// PARALLELIZING DATA

/**
 * \var nthreads
 * \brief Threads number.
 * \var mutex
 * \brief Mutex to lock memory saves.
 */
unsigned int nthreads;
GMutex mutex[1];

// END

/**
 * \fn inline unsigned int sqr(int x)
 * \brief Function to return the square of an unsigned int.
 * \param x
 * \brief unsigned int.
 * \return square.
 */ 
static inline unsigned int sqr(int x) {return x * x;}

/**
 * \fn inline void list_points_delete()
 * \brief Function to delete a list of points.
 * \param
 */
static inline void list_points_delete()
{
	while (list_points)
	{
		g_free(list_points->data);
		list_points = g_list_remove_link(list_points, list_points);
	}
}

/**
 * \fn inline void list_points_add(int x, int y, int z, unsigned int c)
 * \brief Function to add a point to the list.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param c
 * \brief Point color.
 */
static inline void list_points_add(int x, int y, int z, unsigned int c)
{
	Point *point;
	point = (Point*)g_malloc(sizeof(Point));
	point->x = x;
	point->y = y;
	point->z = z;
	point->c = c;
	list_points = g_list_prepend(list_points, point);
}

/**
 * \fn inline void point_2D_move(int *x, int *y, gsl_rng *rng)
 * \brief Function to make a random 2D movement on a point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void point_2D_move(int *x, int *y, gsl_rng *rng)
{
	register unsigned int k;
	static const int
		mx[4]={0, 0, 1, -1},
		my[4]={1, -1, 0, 0};
	k = gsl_rng_uniform_int(rng, 4);
	*x += mx[k];
	*y += my[k];
}

/**
 * \fn inline void point_2D_move_diagonal(int *x, int *y, gsl_rng *rng)
 * \brief Function to make a random 2D movement on a point enabling diagonals.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void point_2D_move_diagonal(int *x, int *y, gsl_rng *rng)
{
	register unsigned int k;
	static const int
		mx[8] = {1, 1, 1, 0, -1, -1, -1, 0},
		my[8] = {1, 0, -1, -1, -1, 0, 1, 1};
	k = gsl_rng_uniform_int(rng, 8);
	*x += mx[k];
	*y += my[k];
}

/**
 * \fn inline void point_3D_move(int *x, int *y, int *z, gsl_rng *rng)
 * \brief Function to make a random 3D movement on a point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void point_3D_move(int *x, int *y, int *z, gsl_rng *rng)
{
	register unsigned int k;
	static const int
		mx[6] = {0, 1, -1, 0, 0, 0},
		my[6] = {0, 0, 0, 1, -1, 0},
		mz[6] = {1, 0, 0, 0, 0, -1};
	k = gsl_rng_uniform_int(rng, 6);
	*x += mx[k];
	*y += my[k];
	*z += mz[k];
}

/**
 * \fn inline void point_3D_move_diagonal(int *x, int *y, int *z, gsl_rng *rng)
 * \brief Function to make a random 3D movement on a point enabling diagonals.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void point_3D_move_diagonal(int *x, int *y, int *z, gsl_rng *rng)
{
	register int k;
	static const int
		mx[26] =
		{
			1, 1, 1, 0, 0, 0, -1, -1, -1,
			1, 1, 1, 0, 0, -1, -1, -1,
			1, 1, 1, 0, 0, 0, -1, -1, -1
		},
		my[26] =
		{
			1, 0, -1, 1, 0, -1, 1, 0, -1,
			1, 0, -1, 1, -1, 1, 0, -1,
			1, 0, -1, 1, 0, -1, 1, 0, -1
		},
		mz[26] =
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1, -1, -1, -1
		};
	k = gsl_rng_uniform_int(rng, 26);
	*x += mx[k];
	*y += my[k];
	*z += mz[k];
}

/**
 * \fn inline void tree_2D_point_new(int *x, int *y, gsl_rng *rng)
 * \brief Function to start a new 2D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void tree_2D_point_new(int *x, int *y, gsl_rng *rng)
{
	*x = gsl_rng_uniform_int(rng, width);
	*y = max_d;
	#if DEBUG
		printf("New point x %d y %d\n", *x, *y);
	#endif
}

/**
 * \fn inline void tree_2D_point_boundary(int *x, int *y, gsl_rng *rng)
 * \brief Function to check the limits of a 2D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void tree_2D_point_boundary(int *x, int *y, gsl_rng *rng)
{
	if (*y < 0 || *y == height)
	{
		tree_2D_point_new(x, y, rng);
		return;
	}
	if (*x < 0) *x = width - 1;
	else if (*x == width) *x = 0;
	#if DEBUG
		printf("Boundary point x %d y %d\n", *x, *y);
	#endif
}

/**
 * \fn inline void tree_2D_point_fix(int *x, int *y)
 * \brief Function to fix a 2D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int tree_2D_point_fix(int x, int y)
{
	register unsigned int *point;
	#if DEBUG
		printf("x=%d y=%d max_d=%d width=%d height=%d\n", x, y, max_d, width,
			height);
	#endif
	if (y > max_d || x == 0 || y == 0 || x == width - 1 || y == height - 1)
		return 0;
	point = medium + y * width + x;
	if (point[1] || point[-1] || point[width] || point[-(int)width])
	{
		#if DEBUG
			printf("fixing point\n");
		#endif
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		point[0] = 2;
		list_points_add(x, y, 0, 2);
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void tree_2D_init()
 * \brief Function to init a 2D tree.
 * \param
 */
static inline void tree_2D_init()
{
	medium[width / 2] = 2;
}

/**
 * \fn inline unsigned int tree_2D_end(int x, int y)
 * \brief Function to check the end a 2D tree.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int tree_2D_end(int x, int y)
{
	if (y == max_d)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		++max_d;
		g_mutex_unlock(mutex);
// END
	}
	if (max_d >= height-1)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		max_d = height - 1;
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void tree_3D_point_new(int *x, int *y, int *z, gsl_rng *rng)
 * \brief Function to start a new 3D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void tree_3D_point_new(int *x, int *y, int *z, gsl_rng *rng)
{
	*x = gsl_rng_uniform_int(rng, length);
	*y = gsl_rng_uniform_int(rng, width);
	*z = max_d;
	#if DEBUG
		printf("New point x %d y %d z %d\n", *x, *y, *z);
	#endif
}

/**
 * \fn inline void tree_3D_point_boundary(int *x, int *y, int *z, gsl_rng *rng)
 * \brief Function to check the limits of a 2D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void tree_3D_point_boundary(int *x, int *y, int *z, gsl_rng *rng)
{
	if (*z < 0 || *z == height)
	{
		tree_3D_point_new(x, y, z, rng);
		return;
	}
	if (*x < 0) *x = length - 1; else if (*x == length) *x = 0;
	if (*y < 0) *y = width - 1; else if (*y == width) *y = 0;
	#if DEBUG
		printf("New point x %d y %d z %d\n", *x, *y, *z);
	#endif
}

/**
 * \fn inline unsigned int tree_3D_point_fix(int x, int y, int z)
 * \brief Function to fix a 2D tree point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief point z-coordinate.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int tree_3D_point_fix(int x, int y, int z)
{
	register unsigned int *point;
	if (z > max_d || z == 0 || y == 0 || x == 0 || z == height - 1
		|| y == width - 1 || x == length - 1)
			return 0;
	point = medium + z * area + y * length + x;
	if (point[1] || point[-1] || point[length] || point[-(int)length]
		|| point[area] || point[-(int)area])
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		point[0] = 2;
		list_points_add(x, y, z, 2);
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void tree_3D_init()
 * \brief Function to init a 3D tree.
 * \param
 */
static inline void tree_3D_init()
{
	medium[length * (width / 2) + length / 2] = 2;
}

/**
 * \fn inline unsigned int tree_3D_end(int x, int y, int z)
 * \brief Function to check the end a 3D tree.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int tree_3D_end(int x, int y, int z)
{
	if (z==max_d)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		++max_d;
		g_mutex_unlock(mutex);
// END
	}
	if (max_d>=height-1)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		max_d=height-1;
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void forest_2D_point_boundary(int *x, int *y, gsl_rng *rng)
 * \brief Function to check the limits of a 2D forest point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void forest_2D_point_boundary(int *x, int *y, gsl_rng *rng)
{
	if (*y == height || *y < 0)
	{
		tree_2D_point_new(x, y, rng);
		return;
	}
	if (*x < 0) *x = width - 1; else if (*x == width) *x = 0;
	#if DEBUG
		printf("Boundary point x %d y %d\n", *x, *y);
	#endif
}

/**
 * \fn inline unsigned int forest_2D_point_fix(int x, int y, gsl_rng *rng)
 * \brief Function to fix a 2D forest point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int forest_2D_point_fix(int x, int y, gsl_rng *rng)
{
	register unsigned int k, *point;
	if (y > max_d || x == 0 || x == width - 1 || y == height - 1) return 0;
	point = medium + y * width + x;
	if (y == 0)
	{
		k = 1 + gsl_rng_uniform_int(rng, 15);
		goto forest;
	}
	k = point[1];
	if (k) goto forest;
	k = point[-1];
	if (k) goto forest;
	k = point[width];
	if (k) goto forest;
	k = point[-(int)width];
	if (k) goto forest;
	return 0;

forest:
// PARALLELIZING MUTEX
	g_mutex_lock(mutex);
	point[0] = k;
	list_points_add(x, y, 0, k);
	g_mutex_unlock(mutex);
// END
	return k;
}

/**
 * \fn inline void forest_3D_point_boundary(int *x, int *y, int *z, \
 *   gsl_rng *rng)
 * \brief Function to check the limits of a 3D forest point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void forest_3D_point_boundary
(int *x, int *y, int *z, gsl_rng *rng)
{
	if (*z == height || *z < 0)
	{
		tree_3D_point_new(x, y, z, rng);
		return;
	}
	if (*y < 0) *y = width - 1; else if (*y == width) *y = 0;
	if (*x < 0) *x = length - 1; else if (*x == length) *x = 0;
	#if DEBUG
		printf("Boundary point x %d y %d z %d\n", *x, *y, *z);
	#endif
}

/**
 * \fn inline unsigned int forest_3D_point_fix(int x, int y, int z, \
 *   gsl_rng *rng)
 * \brief Function to fix a 3D forest point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int forest_3D_point_fix
(int x, int y, int z, gsl_rng *rng)
{
	register unsigned int k, *point;
	if (z > max_d || y == 0 || x == 0 || z == height - 1 || y == width - 1
		|| x == length - 1)
			return 0;
	point = medium + z * area + y * length + x;
	if (z == 0)
	{
		k = 1 + gsl_rng_uniform_int(rng, 15);
		goto forest;
	}
	k = point[1];
	if (k) goto forest;
	k = point[-1];
	if (k) goto forest;
	k = point[length];
	if (k) goto forest;
	k = point[-(int)length];
	if (k) goto forest;
	k = point[area];
	if (k) goto forest;
	k = point[-(int)area];
	if (k) goto forest;
	return 0;

forest:
// PARALLELIZING MUTEX
	g_mutex_lock(mutex);
	point[0] = k;
	list_points_add(x, y, z, k);
	g_mutex_unlock(mutex);
// END
	return k;
}

/**
 * \fn inline void neuron_2D_point_new(int *x, int *y, gsl_rng *rng)
 * \brief Function to start a new 2D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void neuron_2D_point_new(int *x, int *y, gsl_rng *rng)
{
	register double angle;
	angle = 2 * M_PI * gsl_rng_uniform(rng);
	*x = width / 2 + max_d * cos(angle);
	*y = height / 2 + max_d * sin(angle);
	#if DEBUG
		printf("New point x %d y %d\n", *x, *y);
	#endif
}

/**
 * \fn inline void neuron_2D_point_boundary(int *x, int *y, gsl_rng *rng)
 * \brief Function to check the limits of a 2D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void neuron_2D_point_boundary(int *x, int *y, gsl_rng *rng)
{
	if (*y < 0 || *y == height || *x < 0 || *x == width)
	{
		neuron_2D_point_new(x, y, rng); 
		#if DEBUG
			printf("Boundary point x %d y %d\n", *x, *y);
		#endif
	}
}

/**
 * \fn inline unsigned int neuron_2D_point_fix(int x, int y)
 * \brief Function to fix a 2D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int neuron_2D_point_fix(int x, int y)
{
	register unsigned int *point;
	if (x == 0 || y == 0 || x == width - 1 || y == height - 1) return 0;
	point = medium + y * width + x;
	if (point[1] || point[-1] || point[width] || point[-(int)width])
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		point[0] = 2;
		list_points_add(x, y, 0, 2);
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void neuron_2D_init()
 * \brief Function to init a 2D neuron.
 * \param
 */
static inline void neuron_2D_init()
{
	medium[(height / 2) * width + width / 2] = 2;
}

/**
 * \fn inline unsigned int neuron_2D_end(int x, int y)
 * \brief Function to check the end a 2D neuron.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int neuron_2D_end(int x, int y)
{
	register int r, k;
	r = 1 + round(sqrt(sqr(x - width / 2) + sqr(y - height / 2)));
	if (r >= max_d)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		++max_d;
		g_mutex_unlock(mutex);
// END
	}
	if (height < width) k = height; else k = width;
	k = k / 2 - 1;
	if (max_d >= k)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		max_d = k;
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void neuron_3D_point_new(int *x, int *y, int *z, gsl_rng *rng)
 * \brief Function to start a new 3D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void neuron_3D_point_new(int *x, int *y, int *z, gsl_rng *rng)
{
	register double theta, phi;
	theta = 2 * M_PI * gsl_rng_uniform(rng);
	phi = M_PI * gsl_rng_uniform(rng);
	*x = length / 2 + max_d * cos(phi) * cos(theta);
	*y = width / 2 + max_d * sin(phi) * cos(theta);
	*z = height / 2 + max_d * sin(theta);
	#if DEBUG
		printf("New point x %d y %d z %d\n", *x, *y, *z);
	#endif
}

/**
 * \fn inline void neuron_3D_point_boundary(int *x, int *y, int *z, \
 *   gsl_rng *rng)
 * \brief Function to check the limits of a 3D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param rng
 * \brief Pseudo-random number generator.
 */
static inline void neuron_3D_point_boundary
(int *x, int *y, int *z, gsl_rng *rng)
{
	if (*z < 0 || *y < 0 || *x < 0 || *z == height || *y == width
		|| *x == length)
	{
		neuron_3D_point_new(x, y, z, rng); 
		#if DEBUG
			printf("Boundary point x %d y %d z %d\n", *x, *y, *z);
		#endif
	}
}

/**
 * \fn inline unsigned int neuron_3D_point_fix(int x, int y, int z)
 * \brief Function to fix a 3D neuron point.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int neuron_3D_point_fix(int x, int y, int z)
{
	register unsigned int *point;
	if (z == 0 || y == 0 || x == 0 || z == height - 1 || y == width - 1
		|| x == length - 1)
			return 0;
	point = medium + z * area + y * length + x;
	if (point[1] || point[-1] || point[length] || point[-(int)length] ||
		point[area] || point[-(int)area])
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		point[0] = 2;
		list_points_add(x, y, z, 2);
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn inline void neuron_3D_init()
 * \brief Function to init a 3D neuron.
 * \param
 */
static inline void neuron_3D_init()
{
	medium[area * (height / 2) + length * (width / 2) + length / 2] = 2;
}

/**
 * \fn inline unsigned int neuron_3D_end(int x, int y, int z)
 * \brief Function to check the end a 3D neuron.
 * \param x
 * \brief Point x-coordinate.
 * \param y
 * \brief Point y-coordinate.
 * \param z
 * \brief Point z-coordinate.
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int neuron_3D_end(int x, int y, int z)
{
	register int r, k;
	r = 1 + sqrt(sqr(x - length / 2) + sqr(y - width / 2)
		+ sqr(z - height / 2));
	if (r >= max_d)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		++max_d;
		g_mutex_unlock(mutex);
// END
	}
	k = length;
	if (width < k) k = width;
	if (height < k) k = height;
	k = k / 2 - 1;
	if (max_d >= k)
	{
// PARALLELIZING MUTEX
		g_mutex_lock(mutex);
		max_d = k;
		g_mutex_unlock(mutex);
// END
		return 1;
	}
	return 0;
}

/**
 * \fn void fractal_stop()
 * \brief Function to stop the fractal simulation.
 */
void fractal_stop()
{
// PARALLELIZING MUTEX
	g_mutex_lock(mutex);
	breaking = 1;
	g_mutex_unlock(mutex);
// END
}

// PARALLELIZED FUNCTIONS

/**
 * \fn void* parallel_fractal_tree_2D(gsl_rng *rng)
 * \brief Function to create a 2D fractal tree.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_tree_2D(gsl_rng *rng)
{
	int x, y;
	long t0;
	#if DEBUG
		printf("parallel_fractal_tree_2D: start\n");
	#endif
	t0 = time(NULL);
	do
	{
		#if DEBUG
			printf("creating point\n");
		#endif
		tree_2D_point_new(&x, &y, rng);
		#if DEBUG
			printf("checking fix\n");
		#endif
		while (!breaking && !tree_2D_point_fix(x, y))
		{
			#if DEBUG
				printf("moving point\n");
			#endif
			point_2D_move(&x, &y, rng);
			#if DEBUG
				printf("checking boundary\n");
			#endif
			tree_2D_point_boundary(&x, &y, rng);
		}
		#if DEBUG
			printf("checking end\n");
		#endif
		if (animating && time(NULL) > t0) break;
		if (tree_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_tree_3D(gsl_rng *rng)
 * \brief Function to create a 3D fractal tree.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_tree_3D(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !tree_3D_point_fix(x, y, z))
		{
			point_3D_move(&x, &y, &z, rng);
			tree_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_forest_2D(gsl_rng *rng)
 * \brief Function to create a 2D fractal forest.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_forest_2D(gsl_rng *rng)
{
	int x, y;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_2D_point_new(&x, &y, rng);
		while (!breaking && !forest_2D_point_fix(x, y, rng))
		{
			point_2D_move(&x, &y, rng);
			forest_2D_point_boundary(&x, &y, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_forest_3D(gsl_rng *rng)
 * \brief Function to create a 3D fractal forest.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_forest_3D(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !forest_3D_point_fix(x, y, z, rng))
		{
			point_3D_move(&x, &y, &z, rng);
			forest_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_neuron_2D(gsl_rng *rng)
 * \brief Function to create a 2D fractal neuron.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_neuron_2D(gsl_rng *rng)
{
	int x, y;
	long t0;
	t0 = time(NULL);
	do
	{
		neuron_2D_point_new(&x, &y, rng);
		while (!breaking && !neuron_2D_point_fix(x, y))
		{
			point_2D_move(&x, &y, rng);
			neuron_2D_point_boundary(&x, &y, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (neuron_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_neuron_3D(gsl_rng *rng)
 * \brief Function to create a 3D fractal neuron.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_neuron_3D(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		neuron_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !neuron_3D_point_fix(x, y, z))
		{
			point_3D_move(&x, &y, &z, rng);
			neuron_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (neuron_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_tree_2D_diagonal(gsl_rng *rng)
 * \brief Function to create a 2D fractal tree with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_tree_2D_diagonal(gsl_rng *rng)
{
	int x, y;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_2D_point_new(&x, &y, rng);
		while (!breaking && !tree_2D_point_fix(x, y))
		{
			point_2D_move(&x, &y, rng);
			tree_2D_point_boundary(&x, &y, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_tree_3D_diagonal(gsl_rng *rng)
 * \brief Function to create a 3D fractal tree with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_tree_3D_diagonal(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !tree_3D_point_fix(x, y, z))
		{
			point_3D_move(&x, &y, &z, rng);
			tree_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_forest_2D_diagonal(gsl_rng *rng)
 * \brief Function to create a 2D fractal forest with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_forest_2D_diagonal(gsl_rng *rng)
{
	int x, y;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_2D_point_new(&x, &y, rng);
		while (!breaking && !forest_2D_point_fix(x, y, rng))
		{
			point_2D_move_diagonal(&x, &y, rng);
			forest_2D_point_boundary(&x, &y, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_forest_3D_diagonal(gsl_rng *rng)
 * \brief Function to create a 3D fractal forest with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_forest_3D_diagonal(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		tree_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !forest_3D_point_fix(x, y, z, rng))
		{
			point_3D_move_diagonal(&x, &y, &z, rng);
			forest_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (tree_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_neuron_2D_diagonal(gsl_rng *rng)
 * \brief Function to create a 2D fractal neuron with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_neuron_2D_diagonal(gsl_rng *rng)
{
	int x, y;
	long t0;
	t0 = time(NULL);
	do
	{
		neuron_2D_point_new(&x, &y, rng);
		while (!breaking && !neuron_2D_point_fix(x, y))
		{
			point_2D_move_diagonal(&x, &y, rng);
			neuron_2D_point_boundary(&x, &y, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (neuron_2D_end(x, y)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void* parallel_fractal_neuron_3D_diagonal(gsl_rng *rng)
 * \brief Function to create a 3D fractal neuron with diagonal movements.
 * \param rng
 * \brief Pseudo-random number generator.
 * \return NULL.
 */
void* parallel_fractal_neuron_3D_diagonal(gsl_rng *rng)
{
	int x, y, z;
	long t0;
	t0 = time(NULL);
	do
	{
		neuron_3D_point_new(&x, &y, &z, rng);
		while (!breaking && !neuron_3D_point_fix(x, y, z))
		{
			point_3D_move_diagonal(&x, &y, &z, rng);
			neuron_3D_point_boundary(&x, &y, &z, rng);
		}
		if (animating && time(NULL) > t0) break;
		if (neuron_3D_end(x, y, z)) fractal_stop();
	}
	while (!breaking);
	g_thread_exit(NULL);
	return NULL;
}

//END OF PARALLELIZED FUNCTIONS

/**
 * \fn void medium_start()
 * \brief Function to start the fractal functions and data.
 */
void medium_start()
{
	register int i, j;

	#if DEBUG
		printf("Deleting points list\n");
	#endif
	list_points_delete();

	j = area = width * length;
	if (fractal_3D) j*= length;
	medium = (unsigned int*)g_realloc(medium, j * sizeof(unsigned int));
	for (i = j; --i >= 0;) medium[i] = 0;
	#if DEBUG
		printf("Medium size=%d pointer=%ld\n", j, (size_t)medium);
	#endif

	#if DEBUG
		printf("Setting functions\n");
	#endif
	if (fractal_diagonal)
	{
		if (fractal_3D)
		{
			switch (fractal_type)
			{
			case FRACTAL_TYPE_TREE:
				tree_3D_init();
				parallel_fractal = parallel_fractal_tree_3D_diagonal;
				max_d = 1;
				break;
			case FRACTAL_TYPE_FOREST:
				parallel_fractal = parallel_fractal_forest_3D_diagonal;
				max_d = 1;
				break;
			default:
				neuron_3D_init();
				parallel_fractal = parallel_fractal_neuron_3D_diagonal;
				max_d = 2;
			}
		}
		else
		{
			switch (fractal_type)
			{
			case FRACTAL_TYPE_TREE:
				tree_2D_init();
				parallel_fractal = parallel_fractal_tree_2D_diagonal;
				max_d = 1;
				break;
			case FRACTAL_TYPE_FOREST:
				parallel_fractal = parallel_fractal_forest_2D_diagonal;
				max_d = 1;
				break;
			default:
				neuron_2D_init();
				parallel_fractal = parallel_fractal_neuron_2D_diagonal;
				max_d = 2;
			}
		}
	}
	else
	{
		if (fractal_3D)
		{
			switch (fractal_type)
			{
			case FRACTAL_TYPE_TREE:
				tree_3D_init();
				parallel_fractal = parallel_fractal_tree_3D;
				max_d = 1;
				break;
			case FRACTAL_TYPE_FOREST:
				parallel_fractal = parallel_fractal_forest_3D;
				max_d = 1;
				break;
			default:
				neuron_3D_init();
				parallel_fractal = parallel_fractal_neuron_3D;
				max_d = 2;
			}
		}
		else
		{
			switch (fractal_type)
			{
			case FRACTAL_TYPE_TREE:
				tree_2D_init();
				parallel_fractal = parallel_fractal_tree_2D;
				max_d = 1;
				break;
			case FRACTAL_TYPE_FOREST:
				parallel_fractal = parallel_fractal_forest_2D;
				max_d = 1;
				break;
			default:
				neuron_2D_init();
				parallel_fractal = parallel_fractal_neuron_2D;
				max_d = 2;
			}
		}
	}
}

/**
 * \fn void fractal()
 * \brief Function with the main bucle to draw the fractal.
 */
void fractal()
{
	int i;
	FILE *file;
	const gsl_rng_type *random_type[N_RANDOM_TYPES] = 
	{
		gsl_rng_mt19937,
		gsl_rng_ranlxs0,
		gsl_rng_ranlxs1,
		gsl_rng_ranlxs2,
		gsl_rng_ranlxd1,
		gsl_rng_ranlxd2,
		gsl_rng_ranlux,
		gsl_rng_ranlux389,
		gsl_rng_cmrg,
		gsl_rng_mrg,
		gsl_rng_taus2,
		gsl_rng_gfsr4
	};

// PARALLELIZING DATA
	gsl_rng *rng[nthreads];
	GThread *thread[nthreads];

	t0 = time(NULL);
	#if DEBUG
		printf("t0=%lu\n", t0);
	#endif

	#if DEBUG
		printf("Opening log file\n");
	#endif
	file = fopen("log", "w");

	#if DEBUG
		printf("Opening pseudo-random generators\n");
	#endif
	for (i = 0; i < nthreads; ++i)
	{
		rng[i] = gsl_rng_alloc(random_type[random_algorithm]);
		switch (random_seed_type)
		{
			case RANDOM_SEED_TYPE_DEFAULT:
				break;
			case RANDOM_SEED_TYPE_CLOCK:
				gsl_rng_set(rng[i], (unsigned long)clock());
				break;
			default:
				gsl_rng_set(rng[i], random_seed);
		}
	}

// END

	breaking = 0;
	simulating = 1;

	#if DEBUG
		printf("Updating simulator dialog\n");
	#endif
	dialog_simulator_update();

	#if DEBUG
		printf("Starting medium\n");
	#endif
	medium_start();

	#if DEBUG
		printf("Main bucle\n");
	#endif
	do
	{
		#if DEBUG
			printf("Calculating fractal\n");
		#endif
// PARALLELIZING CALLS
		for (i = 0; i < nthreads; ++i)
			thread[i] = g_thread_new(NULL, (void(*))parallel_fractal, rng[i]);
		for (i = 0; i < nthreads; ++i) g_thread_join(thread[i]);
// END

		#if DEBUG
			printf("Updating simulator dialog\n");
		#endif
		dialog_simulator_progress();

// DISPLAYING DRAW
		#if DEBUG
			printf("Redisplaying draw\n");
		#endif
		draw();
// END

		#if DEBUG
			printf("Saving log data\n");
		#endif
		fprintf(file, "%d %d\n", max_d, g_list_length(list_points));
	}
	while (!breaking);

	#if DEBUG
		printf("Closing log file\n");
	#endif
	fclose(file);

	#if DEBUG
		printf("Updating simulator dialog\n");
	#endif
	breaking = simulating = 0;
	dialog_simulator_update();

	#if DEBUG
		printf("Freeing threads\n");
	#endif
	for (i = 0; i < nthreads; ++i) g_free(rng[i]);
}
