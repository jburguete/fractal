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
 * \file fractal.c
 * \brief Source file to define the fractal data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2020, Javier Burguete Tolosa.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <glib.h>
#include <glibtop.h>
#include <glibtop/close.h>
#ifdef G_OS_WIN32
#include <windows.h>
#endif
#include <libintl.h>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gtk/gtk.h>
#include "config.h"
#include "fractal.h"
#include "simulator.h"

extern void draw ();

unsigned int width = WIDTH;     ///< Medium width.
unsigned int height = HEIGHT;   ///< Medium height.
unsigned int length = LENGTH;   ///< Medium length.
unsigned int area;              ///< Medium area.
unsigned int medium_bytes;      ///< Number of bytes used by the medium.
unsigned int breaking = 0;      ///< 1 on breaking, 0 otherwise.
unsigned int simulating = 0;    ///< 1 on simulating, 0 otherwise.
unsigned int animating = 1;     ///< 1 on animating, 0 otherwise.

unsigned int fractal_type = FRACTAL_TYPE_TREE;  ///< Fractal type.
unsigned int fractal_points = 0;        ///< Fractal points number.
unsigned int fractal_3D = 0;    ///< 1 on 3D fractals, 0 on 2D fractals.
unsigned int fractal_diagonal = 0;
///< 1 on diagonal point movement, 0 otherwise.

unsigned long t0;               ///< Computational time.

float xmin;                     ///< Minimun x-coordinate of the view.
float xmax;                     ///< Maximun x-coordinate of the view.
float ymin;                     ///< Minimun y-coordinate of the view.
float ymax;                     ///< Maximun y-coordinate of the view.
float phi;                      ///< Horizontal perspective angle (in radians).
float theta;                    ///< Vertical perspective angle (in radians).
float phid = -45.;              ///< Horizontal perspective angle (in degrees).
float thetad = 80.;             ///< Vertical perspective angle (in degrees).

unsigned int max_d = 0;         ///< Maximum fractal size.
unsigned int *medium = NULL;    ///< Array of fractal points.
Point3D *point = NULL;          ///< Array of 3D points.
unsigned int npoints = 0;       ///< Number of points.

unsigned int random_algorithm = 0;
///< Type of random numbers generator algorithm.
unsigned int random_seed_type = 1;      ///< Type of random seed.
unsigned long random_seed = SEED;       ///< Random seed.
void *(*parallel_fractal) (gsl_rng * rng);
///< Pointer to the function to calculate the fractal.

const float color3f[16][3] = {
  {0., 0., 0.},
  {1., 0., 0.},
  {0., 1., 0.},
  {0., 0., 1.},
  {0.5, 0.5, 0.},
  {0.5, 0., 0.5},
  {0., 0.5, 0.5},
  {0.5, 0.25, 0.25},
  {0.25, 0.5, 0.25},
  {0.25, 0.25, 0.5},
  {0.75, 0.25, 0.},
  {0.75, 0., 0.25},
  {0.25, 0.75, 0.},
  {0., 0.75, 0.25},
  {0.25, 0., 0.75},
  {0., 0.25, 0.75}
};                              ///< Array of colors.

DialogOptions dialog_options[1];
  ///< DialogOptions to set the fractal options.
DialogSimulator dialog_simulator[1];
  ///< DialogSimulator to show the main window.

// PARALLELIZING DATA

unsigned int nthreads;          ///< Threads number.
GMutex mutex[1];                ///< Mutex to lock memory saves.

// END

/**
 * Function to return the square of an unsigned int.

 * \return square.
 */
static inline unsigned int
sqr (int x)                     ///< unsigned int.
{
  return x * x;
}

/**
 * Function to get the number of cores of the CPU.
 *
 * \return CPU number of cores.
 */
int
threads_number ()
{
  int ncores;
  glibtop *top;
  top = glibtop_init ();
  ncores = top->ncpu + 1;
  glibtop_close ();
  return ncores;
}

/**
 * Function to add a point to the array.
 */
static inline void
points_add (int x,              ///< Point x-coordinate.
            int y,              ///< Point y-coordinate.
            int z,              ///< Point z-coordinate.
            unsigned int c)     ///< Point color.
{
  Point3D *p;
  ++npoints;
  point = (Point3D *) g_realloc (point, npoints * sizeof (Point3D));
  p = point + npoints - 1;
  p->r[0] = x;
  p->r[1] = y;
  p->r[2] = z;
  p->c[0] = color3f[c][0];
  p->c[1] = color3f[c][1];
  p->c[2] = color3f[c][2];
}

/**
 * Function to make a random 2D movement on a point.
 */
static inline void
point_2D_move (int *x,          ///< Point x-coordinate.
               int *y,          ///< Point y-coordinate.
               gsl_rng * rng)   ///< Pseudo-random number generator.
{
  register unsigned int k;
  static const int mx[4] = { 0, 0, 1, -1 }, my[4] = { 1, -1, 0, 0 };
  k = gsl_rng_uniform_int (rng, 4);
  *x += mx[k];
  *y += my[k];
}

/**
 * Function to make a random 2D movement on a point enabling diagonals.
 */
static inline void
point_2D_move_diagonal (int *x, ///< Point x-coordinate.
                        int *y, ///< Point y-coordinate.
                        gsl_rng * rng)  ///< Pseudo-random number generator.
{
  register unsigned int k;
  static const int mx[8] = { 1, 1, 1, 0, -1, -1, -1, 0 },
    my[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
  k = gsl_rng_uniform_int (rng, 8);
  *x += mx[k];
  *y += my[k];
}

/**
 * Function to make a random 3D movement on a point.
 */
static inline void
point_3D_move (int *x,          ///< Point x-coordinate.
               int *y,          ///< Point y-coordinate.
               int *z,          ///< Point z-coordinate.
               gsl_rng * rng)   ///< Pseudo-random number generator.
{
  register unsigned int k;
  static const int mx[6] = { 0, 1, -1, 0, 0, 0 },
    my[6] = { 0, 0, 0, 1, -1, 0 }, mz[6] = { 1, 0, 0, 0, 0, -1 };
  k = gsl_rng_uniform_int (rng, 6);
  *x += mx[k];
  *y += my[k];
  *z += mz[k];
}

/**
 * Function to make a random 3D movement on a point enabling diagonals.
 */
static inline void
point_3D_move_diagonal (int *x, ///< Point x-coordinate.
                        int *y, ///< Point y-coordinate.
                        int *z, ///< Point z-coordinate.
                        gsl_rng * rng)  ///< Pseudo-random number generator.
{
  register int k;
  static const int mx[26] = {
    1, 1, 1, 0, 0, 0, -1, -1, -1,
    1, 1, 1, 0, 0, -1, -1, -1,
    1, 1, 1, 0, 0, 0, -1, -1, -1
  }, my[26] = {
    1, 0, -1, 1, 0, -1, 1, 0, -1,
    1, 0, -1, 1, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1, 1, 0, -1
  }, mz[26] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };
  k = gsl_rng_uniform_int (rng, 26);
  *x += mx[k];
  *y += my[k];
  *z += mz[k];
}

/**
 * Function to start a new 2D tree point.
 */
static inline void
tree_2D_point_new (int *x,      ///< Point x-coordinate.
                   int *y,      ///< Point y-coordinate.
                   gsl_rng * rng)       ///< Pseudo-random number generator.
{
  *x = gsl_rng_uniform_int (rng, width);
  *y = max_d;
#if DEBUG
  printf ("New point x %d y %d\n", *x, *y);
#endif
}

/**
 * Function to check the limits of a 2D tree point.
 */
static inline void
tree_2D_point_boundary (int *x, ///< Point x-coordinate.
                        int *y, ///< Point y-coordinate.
                        gsl_rng * rng)  ///< Pseudo-random number generator.
{
  if (*y < 0 || *y == (int) height)
    {
      tree_2D_point_new (x, y, rng);
      return;
    }
  if (*x < 0)
    *x = width - 1;
  else if (*x == (int) width)
    *x = 0;
#if DEBUG
  printf ("Boundary point x %d y %d\n", *x, *y);
#endif
}

/**
 * Function to fix a 2D tree point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
tree_2D_point_fix (int x,       ///< Point x-coordinate.
                   int y)       ///< Point y-coordinate.
{
  register unsigned int *point;
#if DEBUG
  printf ("x=%d y=%d max_d=%d width=%d height=%d\n", x, y, max_d, width,
          height);
#endif
  if (y > (int) max_d || x == 0 || y == 0 || x == (int) width - 1
      || y == (int) height - 1)
    return 0;
  point = medium + y * width + x;
  if (point[1] || point[-1] || point[width] || point[-(int) width])
    {
#if DEBUG
      printf ("fixing point\n");
#endif
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      point[0] = 2;
      points_add (x, y, 0, 2);
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to init a 2D tree.
 */
static inline void
tree_2D_init ()
{
  medium[width / 2] = 2;
}

/**
 * Function to check the end a 2D tree.
 *
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int
tree_2D_end (int y)             ///< Point y-coordinate.
{
  if (y == (int) max_d)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      ++max_d;
      g_mutex_unlock (mutex);
// END
    }
  if (max_d >= height - 1)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      max_d = height - 1;
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to start a new 3D tree point.
 */
static inline void
tree_3D_point_new (int *x,      ///< Point x-coordinate.
                   int *y,      ///< Point y-coordinate.
                   int *z,      ///< Point z-coordinate.
                   gsl_rng * rng)       ///< Pseudo-random number generator.
{
  *x = gsl_rng_uniform_int (rng, length);
  *y = gsl_rng_uniform_int (rng, width);
  *z = max_d;
#if DEBUG
  printf ("New point x %d y %d z %d\n", *x, *y, *z);
#endif
}

/**
 * Function to check the limits of a 2D tree point.
 */
static inline void
tree_3D_point_boundary (int *x, ///< Point x-coordinate.
                        int *y, ///< Point y-coordinate.
                        int *z, ///< Point z-coordinate.
                        gsl_rng * rng)  ///< Pseudo-random number generator.
{
  if (*z < 0 || *z == (int) height)
    {
      tree_3D_point_new (x, y, z, rng);
      return;
    }
  if (*x < 0)
    *x = length - 1;
  else if (*x == (int) length)
    *x = 0;
  if (*y < 0)
    *y = width - 1;
  else if (*y == (int) width)
    *y = 0;
#if DEBUG
  printf ("New point x %d y %d z %d\n", *x, *y, *z);
#endif
}

/**
 * Function to fix a 2D tree point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
tree_3D_point_fix (int x,       ///< Point x-coordinate.
                   int y,       ///< Point y-coordinate.
                   int z)       ///< point z-coordinate.
{
  register unsigned int *point;
  if (z > (int) max_d || z == 0 || y == 0 || x == 0 || z == (int) height - 1
      || y == (int) width - 1 || x == (int) length - 1)
    return 0;
  point = medium + z * area + y * length + x;
  if (point[1] || point[-1] || point[length] || point[-(int) length]
      || point[area] || point[-(int) area])
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      point[0] = 2;
      points_add (x, y, z, 2);
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to init a 3D tree.
 */
static inline void
tree_3D_init ()
{
  medium[length * (width / 2) + length / 2] = 2;
}

/**
 * Function to check the end a 3D tree.
 * 
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int
tree_3D_end (int z)             ///< Point z-coordinate.
{
  if (z == (int) max_d)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      ++max_d;
      g_mutex_unlock (mutex);
// END
    }
  if (max_d >= height - 1)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      max_d = height - 1;
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to check the limits of a 2D forest point.
 */
static inline void
forest_2D_point_boundary (int *x,       ///< Point x-coordinate.
                          int *y,       ///< Point y-coordinate.
                          gsl_rng * rng)
///< Pseudo-random number generator.
{
  if (*y == (int) height || *y < 0)
    {
      tree_2D_point_new (x, y, rng);
      return;
    }
  if (*x < 0)
    *x = width - 1;
  else if (*x == (int) width)
    *x = 0;
#if DEBUG
  printf ("Boundary point x %d y %d\n", *x, *y);
#endif
}

/**
 * Function to fix a 2D forest point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
forest_2D_point_fix (int x,     ///< Point x-coordinate.
                     int y,     ///< Point y-coordinate.
                     gsl_rng * rng)     ///< Pseudo-random number generator.
{
  register unsigned int k, *point;
  if (y > (int) max_d || x == 0 || x == (int) width - 1
      || y == (int) height - 1)
    return 0;
  point = medium + y * width + x;
  if (y == 0)
    {
      k = 1 + gsl_rng_uniform_int (rng, 15);
      goto forest;
    }
  k = point[1];
  if (k)
    goto forest;
  k = point[-1];
  if (k)
    goto forest;
  k = point[width];
  if (k)
    goto forest;
  k = point[-(int) width];
  if (k)
    goto forest;
  return 0;

forest:
// PARALLELIZING MUTEX
  g_mutex_lock (mutex);
  point[0] = k;
  points_add (x, y, 0, k);
  g_mutex_unlock (mutex);
// END
  return k;
}

/**
 * Function to check the limits of a 3D forest point.
 */
static inline void
forest_3D_point_boundary (int *x,       ///< Point x-coordinate.
                          int *y,       ///< Point y-coordinate.
                          int *z,       ///< Point z-coordinate.
                          gsl_rng * rng)
///< Pseudo-random number generator.
{
  if (*z == (int) height || *z < 0)
    {
      tree_3D_point_new (x, y, z, rng);
      return;
    }
  if (*y < 0)
    *y = width - 1;
  else if (*y == (int) width)
    *y = 0;
  if (*x < 0)
    *x = length - 1;
  else if (*x == (int) length)
    *x = 0;
#if DEBUG
  printf ("Boundary point x %d y %d z %d\n", *x, *y, *z);
#endif
}

/**
 * Function to fix a 3D forest point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
forest_3D_point_fix (int x,     ///< Point x-coordinate.
                     int y,     ///< Point y-coordinate.
                     int z,     ///< Point z-coordinate.
                     gsl_rng * rng)     ///< Pseudo-random number generator.
{
  register unsigned int k, *point;
  if (z > (int) max_d || y == 0 || x == 0 || z == (int) height - 1
      || y == (int) width - 1 || x == (int) length - 1)
    return 0;
  point = medium + z * area + y * length + x;
  if (z == 0)
    {
      k = 1 + gsl_rng_uniform_int (rng, 15);
      goto forest;
    }
  k = point[1];
  if (k)
    goto forest;
  k = point[-1];
  if (k)
    goto forest;
  k = point[length];
  if (k)
    goto forest;
  k = point[-(int) length];
  if (k)
    goto forest;
  k = point[area];
  if (k)
    goto forest;
  k = point[-(int) area];
  if (k)
    goto forest;
  return 0;

forest:
// PARALLELIZING MUTEX
  g_mutex_lock (mutex);
  point[0] = k;
  points_add (x, y, z, k);
  g_mutex_unlock (mutex);
// END
  return k;
}

/**
 * Function to start a new 2D neuron point.
 */
static inline void
neuron_2D_point_new (int *x,    ///< Point x-coordinate.
                     int *y,    ///< Point y-coordinate.
                     gsl_rng * rng)     ///< Pseudo-random number generator.
{
  register double angle;
  angle = 2 * M_PI * gsl_rng_uniform (rng);
  *x = width / 2 + max_d * cos (angle);
  *y = height / 2 + max_d * sin (angle);
#if DEBUG
  printf ("New point x %d y %d\n", *x, *y);
#endif
}

/**
 * Function to check the limits of a 2D neuron point.
 */
static inline void
neuron_2D_point_boundary (int *x,       ///< Point x-coordinate.
                          int *y,       ///< Point y-coordinate.
                          gsl_rng * rng)
///< Pseudo-random number generator.
{
  if (*y < 0 || *y == (int) height || *x < 0 || *x == (int) width)
    {
      neuron_2D_point_new (x, y, rng);
#if DEBUG
      printf ("Boundary point x %d y %d\n", *x, *y);
#endif
    }
}

/**
 * Function to fix a 2D neuron point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
neuron_2D_point_fix (int x,     ///< Point x-coordinate.
                     int y)     ///< Point y-coordinate.
{
  register unsigned int *point;
  if (x == 0 || y == 0 || x == (int) width - 1 || y == (int) height - 1)
    return 0;
  point = medium + y * width + x;
  if (point[1] || point[-1] || point[width] || point[-(int) width])
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      point[0] = 2;
      points_add (x, y, 0, 2);
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to init a 2D neuron.
 */
static inline void
neuron_2D_init ()
{
  medium[(height / 2) * width + width / 2] = 2;
}

/**
 * Function to check the end a 2D neuron.
 *
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int
neuron_2D_end (int x,           ///< Point x-coordinate.
               int y)           ///< Point y-coordinate.
{
  register int r, k;
  r = 1 + round (sqrt (sqr (x - width / 2) + sqr (y - height / 2)));
  if (r >= (int) max_d)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      ++max_d;
      g_mutex_unlock (mutex);
// END
    }
  if (height < width)
    k = height;
  else
    k = width;
  k = k / 2 - 1;
  if ((int) max_d >= k)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      max_d = k;
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to start a new 3D neuron point.
 */
static inline void
neuron_3D_point_new (int *x,    ///< Point x-coordinate.
                     int *y,    ///< Point y-coordinate.
                     int *z,    ///< Point z-coordinate.
                     gsl_rng * rng)     ///< Pseudo-random number generator.
{
  double c1, s1, c2, s2;
  sincos (2. * M_PI * gsl_rng_uniform (rng), &s1, &c1);
  sincos (asin (2. * gsl_rng_uniform (rng) - 1), &s2, &c2);
  *x = length / 2 + max_d * c1 * c2;
  *y = width / 2 + max_d * s1 * c2;
  *z = height / 2 + max_d * s2;
#if DEBUG
  printf ("New point x %d y %d z %d\n", *x, *y, *z);
#endif
}

/**
 * Function to check the limits of a 3D neuron point.
 */
static inline void
neuron_3D_point_boundary (int *x,       ///< Point x-coordinate.
                          int *y,       ///< Point y-coordinate.
                          int *z,       ///< Point z-coordinate.
                          gsl_rng * rng)
///< Pseudo-random number generator.
{
  if (*z < 0 || *y < 0 || *x < 0 || *z == (int) height || *y == (int) width
      || *x == (int) length)
    {
      neuron_3D_point_new (x, y, z, rng);
#if DEBUG
      printf ("Boundary point x %d y %d z %d\n", *x, *y, *z);
#endif
    }
}

/**
 * Function to fix a 3D neuron point.
 *
 * \return 1 on fixing point, 0 on otherwise.
 */
static inline unsigned int
neuron_3D_point_fix (int x,     ///< Point x-coordinate.
                     int y,     ///< Point y-coordinate.
                     int z)     ///< Point z-coordinate.
{
  register unsigned int *point;
  if (z == 0 || y == 0 || x == 0 || z == (int) height - 1
      || y == (int) width - 1 || x == (int) length - 1)
    return 0;
  point = medium + z * area + y * length + x;
  if (point[1] || point[-1] || point[length] || point[-(int) length] ||
      point[area] || point[-(int) area])
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      point[0] = 2;
      points_add (x, y, z, 2);
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to init a 3D neuron.
 */
static inline void
neuron_3D_init ()
{
  medium[area * (height / 2) + length * (width / 2) + length / 2] = 2;
}

/**
 * Function to check the end a 3D neuron.
 *
 * \return 1 on ending, 0 on continuing.
 */
static inline unsigned int
neuron_3D_end (int x,           ///< Point x-coordinate.
               int y,           ///< Point y-coordinate.
               int z)           ///< Point z-coordinate.
{
  register int r, k;
  r = 1 + sqrt (sqr (x - length / 2) + sqr (y - width / 2)
                + sqr (z - height / 2));
  if (r >= (int) max_d)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      ++max_d;
      g_mutex_unlock (mutex);
// END
    }
  k = length;
  if ((int) width < k)
    k = width;
  if ((int) height < k)
    k = height;
  k = k / 2 - 1;
  if ((int) max_d >= k)
    {
// PARALLELIZING MUTEX
      g_mutex_lock (mutex);
      max_d = k;
      g_mutex_unlock (mutex);
// END
      return 1;
    }
  return 0;
}

/**
 * Function to stop the fractal simulation.
 */
void
fractal_stop ()
{
// PARALLELIZING MUTEX
  g_mutex_lock (mutex);
  breaking = 1;
  g_mutex_unlock (mutex);
// END
}

// PARALLELIZED FUNCTIONS

/**
 * Function to create a 2D fractal tree.
 *
 * \return NULL.
 */
void *
parallel_fractal_tree_2D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
#if DEBUG
  printf ("parallel_fractal_tree_2D: start\n");
#endif
  t0 = time (NULL);
  do
    {
#if DEBUG
      printf ("creating point\n");
#endif
      tree_2D_point_new (&x, &y, rng);
#if DEBUG
      printf ("checking fix\n");
#endif
      while (!breaking && !tree_2D_point_fix (x, y))
        {
#if DEBUG
          printf ("moving point\n");
#endif
          point_2D_move (&x, &y, rng);
#if DEBUG
          printf ("checking boundary\n");
#endif
          tree_2D_point_boundary (&x, &y, rng);
        }
#if DEBUG
      printf ("checking end\n");
#endif
      if (animating && time (NULL) > t0)
        break;
      if (tree_2D_end (y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal tree.
 *
 * \return NULL.
 */
void *
parallel_fractal_tree_3D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !tree_3D_point_fix (x, y, z))
        {
          point_3D_move (&x, &y, &z, rng);
          tree_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_3D_end (z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 2D fractal forest.
 *
 * \return NULL.
 */
void *
parallel_fractal_forest_2D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_2D_point_new (&x, &y, rng);
      while (!breaking && !forest_2D_point_fix (x, y, rng))
        {
          point_2D_move (&x, &y, rng);
          forest_2D_point_boundary (&x, &y, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_2D_end (y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal forest.
 *
 * \return NULL.
 */
void *
parallel_fractal_forest_3D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !forest_3D_point_fix (x, y, z, rng))
        {
          point_3D_move (&x, &y, &z, rng);
          forest_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_3D_end (z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 2D fractal neuron.
 *
 * \return NULL.
 */
void *
parallel_fractal_neuron_2D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
  t0 = time (NULL);
  do
    {
      neuron_2D_point_new (&x, &y, rng);
      while (!breaking && !neuron_2D_point_fix (x, y))
        {
          point_2D_move (&x, &y, rng);
          neuron_2D_point_boundary (&x, &y, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (neuron_2D_end (x, y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal neuron.
 *
 * \return NULL.
 */
void *
parallel_fractal_neuron_3D (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      neuron_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !neuron_3D_point_fix (x, y, z))
        {
          point_3D_move (&x, &y, &z, rng);
          neuron_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (neuron_3D_end (x, y, z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 2D fractal tree with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_tree_2D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_2D_point_new (&x, &y, rng);
      while (!breaking && !tree_2D_point_fix (x, y))
        {
          point_2D_move (&x, &y, rng);
          tree_2D_point_boundary (&x, &y, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_2D_end (y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal tree with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_tree_3D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !tree_3D_point_fix (x, y, z))
        {
          point_3D_move (&x, &y, &z, rng);
          tree_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_3D_end (z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 2D fractal forest with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_forest_2D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_2D_point_new (&x, &y, rng);
      while (!breaking && !forest_2D_point_fix (x, y, rng))
        {
          point_2D_move_diagonal (&x, &y, rng);
          forest_2D_point_boundary (&x, &y, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_2D_end (y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal forest with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_forest_3D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      tree_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !forest_3D_point_fix (x, y, z, rng))
        {
          point_3D_move_diagonal (&x, &y, &z, rng);
          forest_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (tree_3D_end (z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 2D fractal neuron with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_neuron_2D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y;
  long t0;
  t0 = time (NULL);
  do
    {
      neuron_2D_point_new (&x, &y, rng);
      while (!breaking && !neuron_2D_point_fix (x, y))
        {
          point_2D_move_diagonal (&x, &y, rng);
          neuron_2D_point_boundary (&x, &y, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (neuron_2D_end (x, y))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

/**
 * Function to create a 3D fractal neuron with diagonal movements.
 *
 * \return NULL.
 */
void *
parallel_fractal_neuron_3D_diagonal (gsl_rng * rng)
///< Pseudo-random number generator.
{
  int x, y, z;
  long t0;
  t0 = time (NULL);
  do
    {
      neuron_3D_point_new (&x, &y, &z, rng);
      while (!breaking && !neuron_3D_point_fix (x, y, z))
        {
          point_3D_move_diagonal (&x, &y, &z, rng);
          neuron_3D_point_boundary (&x, &y, &z, rng);
        }
      if (animating && time (NULL) > t0)
        break;
      if (neuron_3D_end (x, y, z))
        fractal_stop ();
    }
  while (!breaking);
  g_thread_exit (NULL);
  return NULL;
}

//END OF PARALLELIZED FUNCTIONS

/**
 * Function to start the fractal functions and data.
 */
void
medium_start ()
{
  register int i, j;

#if DEBUG
  printf ("Deleting points\n");
#endif
  g_free (point);
  point = NULL;
  npoints = 0;

  j = area = width * length;
  if (fractal_3D)
    j *= length;
  medium_bytes = j * sizeof (unsigned int);
  medium = (unsigned int *) g_slice_alloc (medium_bytes);
  for (i = j; --i >= 0;)
    medium[i] = 0;
#if DEBUG
  printf ("Medium size=%d pointer=%ld\n", j, (size_t) medium);
#endif

#if DEBUG
  printf ("Setting functions\n");
#endif
  if (fractal_diagonal)
    {
      if (fractal_3D)
        {
          switch (fractal_type)
            {
            case FRACTAL_TYPE_TREE:
              tree_3D_init ();
              parallel_fractal = parallel_fractal_tree_3D_diagonal;
              max_d = 1;
              break;
            case FRACTAL_TYPE_FOREST:
              parallel_fractal = parallel_fractal_forest_3D_diagonal;
              max_d = 1;
              break;
            default:
              neuron_3D_init ();
              parallel_fractal = parallel_fractal_neuron_3D_diagonal;
              max_d = 2;
            }
        }
      else
        {
          switch (fractal_type)
            {
            case FRACTAL_TYPE_TREE:
              tree_2D_init ();
              parallel_fractal = parallel_fractal_tree_2D_diagonal;
              max_d = 1;
              break;
            case FRACTAL_TYPE_FOREST:
              parallel_fractal = parallel_fractal_forest_2D_diagonal;
              max_d = 1;
              break;
            default:
              neuron_2D_init ();
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
              tree_3D_init ();
              parallel_fractal = parallel_fractal_tree_3D;
              max_d = 1;
              break;
            case FRACTAL_TYPE_FOREST:
              parallel_fractal = parallel_fractal_forest_3D;
              max_d = 1;
              break;
            default:
              neuron_3D_init ();
              parallel_fractal = parallel_fractal_neuron_3D;
              max_d = 2;
            }
        }
      else
        {
          switch (fractal_type)
            {
            case FRACTAL_TYPE_TREE:
              tree_2D_init ();
              parallel_fractal = parallel_fractal_tree_2D;
              max_d = 1;
              break;
            case FRACTAL_TYPE_FOREST:
              parallel_fractal = parallel_fractal_forest_2D;
              max_d = 1;
              break;
            default:
              neuron_2D_init ();
              parallel_fractal = parallel_fractal_neuron_2D;
              max_d = 2;
            }
        }
    }
}

/**
 * Function to get an unsigned integer numver of a XML node property.
 *
 * \return Unsigned integer number value.
 */
static unsigned int
xml_node_get_uint_with_default (xmlNode * node, ///< XML node.
                                const xmlChar * prop,   ///< XML property.
                                unsigned int default_value,     ///< Default value.
                                int *error_code)        ///< Error code.
{
  unsigned int i = 0;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  *error_code = 0;
  if (!buffer)
    return default_value;
  else
    {
      if (sscanf ((char *) buffer, "%u", &i) != 1)
        *error_code = 1;
      xmlFree (buffer);
    }
  return i;
}

/**
 * Function to get an unsigned long integer numver of a XML node property.
 *
 * \return Unsigned long integer number value.
 */
static unsigned long
xml_node_get_ulong_with_default (xmlNode * node,        ///< XML node.
                                 const xmlChar * prop,  ///< XML property.
                                 unsigned long default_value,
                                 ///< Default value.
                                 int *error_code)       ///< Error code.
{
  unsigned long i = 0l;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  *error_code = 0;
  if (!buffer)
    return default_value;
  else
    {
      if (sscanf ((char *) buffer, "%lu", &i) != 1)
        *error_code = 1;
      xmlFree (buffer);
    }
  return i;
}

/**
 * Function to open the fractal data on a file.
 *
 * \return 1 on success, 0 on error.
 */
int
fractal_input (char *filename)  ///< File name.
{
  xmlDoc *doc;
  xmlNode *node;
  xmlChar *buffer;
  const char *error_message;
  int error_code;
  buffer = NULL;
  xmlKeepBlanksDefault (0);
  doc = xmlParseFile ((const char *) filename);
  if (!doc)
    {
      error_message = _("Unable to parse the input file");
      goto exit_on_error;
    }
  node = xmlDocGetRootElement (doc);
  if (!node || xmlStrcmp (node->name, XML_FRACTAL))
    {
      error_message = _("Bad XML root node");
      goto exit_on_error;
    }
  width = xml_node_get_uint_with_default (node, XML_WIDTH, WIDTH, &error_code);
  if (error_code)
    {
      error_message = _("Bad width");
      goto exit_on_error;
    }
  height
    = xml_node_get_uint_with_default (node, XML_HEIGHT, HEIGHT, &error_code);
  if (error_code)
    {
      error_message = _("Bad height");
      goto exit_on_error;
    }
  length
    = xml_node_get_uint_with_default (node, XML_LENGTH, LENGTH, &error_code);
  if (error_code)
    {
      error_message = _("Bad length");
      goto exit_on_error;
    }
  random_seed
    = xml_node_get_ulong_with_default (node, XML_SEED, SEED, &error_code);
  if (error_code)
    {
      error_message = _("Bad random seed");
      goto exit_on_error;
    }
  nthreads
    = xml_node_get_uint_with_default (node, XML_THREADS, threads_number (),
                                      &error_code);
  if (!nthreads || error_code)
    {
      error_message = _("Bad threads number");
      goto exit_on_error;
    }
  buffer = xmlGetProp (node, XML_DIAGONAL);
  if (!buffer || !xmlStrcmp (buffer, XML_NO))
    fractal_diagonal = 0;
  else if (!xmlStrcmp (buffer, XML_YES))
    fractal_diagonal = 1;
  else
    {
      error_message = _("Bad diagonal movement");
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = xmlGetProp (node, XML_3D);
  if (!buffer || !xmlStrcmp (buffer, XML_NO))
    fractal_3D = 0;
  else if (!xmlStrcmp (buffer, XML_YES))
    fractal_3D = 1;
  else
    {
      error_message = _("Bad 3D");
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = xmlGetProp (node, XML_ANIMATE);
  if (!buffer || !xmlStrcmp (buffer, XML_YES))
    animating = 1;
  else if (!xmlStrcmp (buffer, XML_NO))
    animating = 0;
  else
    {
      error_message = _("Bad animation");
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = xmlGetProp (node, XML_TYPE);
  if (!buffer || !xmlStrcmp (buffer, XML_TREE))
    fractal_type = FRACTAL_TYPE_TREE;
  else if (!xmlStrcmp (buffer, XML_FOREST))
    fractal_type = FRACTAL_TYPE_FOREST;
  else if (!xmlStrcmp (buffer, XML_NEURON))
    fractal_type = FRACTAL_TYPE_NEURON;
  else
    {
      error_message = _("Unknown fractal type");
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = xmlGetProp (node, XML_RANDOM_SEED);
  if (!buffer || !xmlStrcmp (buffer, XML_CLOCK))
    random_seed_type = RANDOM_SEED_TYPE_CLOCK;
  else if (!xmlStrcmp (buffer, XML_DEFAULT))
    random_seed_type = RANDOM_SEED_TYPE_DEFAULT;
  else if (!xmlStrcmp (buffer, XML_FIXED))
    random_seed_type = RANDOM_SEED_TYPE_FIXED;
  else
    {
      error_message = _("Unknown random seed type");
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = xmlGetProp (node, XML_RANDOM_TYPE);
  if (!buffer || !xmlStrcmp (buffer, XML_MT19937))
    random_algorithm = 0;
  else if (!xmlStrcmp (buffer, XML_RANLXS0))
    random_algorithm = 1;
  else if (!xmlStrcmp (buffer, XML_RANLXS1))
    random_algorithm = 2;
  else if (!xmlStrcmp (buffer, XML_RANLXS2))
    random_algorithm = 3;
  else if (!xmlStrcmp (buffer, XML_RANLXD1))
    random_algorithm = 4;
  else if (!xmlStrcmp (buffer, XML_RANLXD2))
    random_algorithm = 5;
  else if (!xmlStrcmp (buffer, XML_RANLUX))
    random_algorithm = 6;
  else if (!xmlStrcmp (buffer, XML_RANLUX389))
    random_algorithm = 7;
  else if (!xmlStrcmp (buffer, XML_CMRG))
    random_algorithm = 8;
  else if (!xmlStrcmp (buffer, XML_MRG))
    random_algorithm = 9;
  else if (!xmlStrcmp (buffer, XML_TAUS2))
    random_algorithm = 10;
  else if (!xmlStrcmp (buffer, XML_GFSR4))
    random_algorithm = 11;
  else
    {
      error_message = _("Unknown random algorithm");
      goto exit_on_error;
    }
  xmlFree (buffer);
  xmlFreeDoc (doc);
  return 1;

exit_on_error:
  show_error (error_message);
  xmlFree (buffer);
  xmlFreeDoc (doc);
  return 0;
}

/**
 * Function with the main bucle to draw the fractal.
 */
void
fractal ()
{
  const gsl_rng_type *random_type[N_RANDOM_TYPES] = {
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
  FILE *file;
  unsigned int i;

// PARALLELIZING DATA
  gsl_rng *rng[nthreads];
  GThread *thread[nthreads];

  t0 = time (NULL);
#if DEBUG
  printf ("t0=%lu\n", t0);
#endif

#if DEBUG
  printf ("Opening log file\n");
#endif
  file = fopen ("log", "w");

#if DEBUG
  printf ("Opening pseudo-random generators\n");
#endif
  for (i = 0; i < nthreads; ++i)
    {
      rng[i] = gsl_rng_alloc (random_type[random_algorithm]);
      switch (random_seed_type)
        {
        case RANDOM_SEED_TYPE_DEFAULT:
          break;
        case RANDOM_SEED_TYPE_CLOCK:
          gsl_rng_set (rng[i], (unsigned long) clock () + i);
          break;
        default:
          gsl_rng_set (rng[i], random_seed + i);
        }
    }

// END

  breaking = 0;
  simulating = 1;

#if DEBUG
  printf ("Updating simulator dialog\n");
#endif
  dialog_simulator_update ();

#if DEBUG
  printf ("Starting medium\n");
#endif
  medium_start ();

#if DEBUG
  printf ("Main bucle\n");
#endif
  do
    {
#if DEBUG
      printf ("Calculating fractal\n");
#endif
// PARALLELIZING CALLS
      for (i = 0; i < nthreads; ++i)
        thread[i] = g_thread_new (NULL, (void (*)) parallel_fractal, rng[i]);
      for (i = 0; i < nthreads; ++i)
        g_thread_join (thread[i]);
// END

#if DEBUG
      printf ("Updating simulator dialog\n");
#endif
      dialog_simulator_progress ();

// DISPLAYING DRAW
#if DEBUG
      printf ("Redisplaying draw\n");
#endif
      draw ();
// END

#if DEBUG
      printf ("Saving log data\n");
#endif
      fprintf (file, "%d %d\n", max_d, npoints);
    }
  while (!breaking);

#if DEBUG
  printf ("Closing log file\n");
#endif
  fclose (file);

#if DEBUG
  printf ("Updating simulator dialog\n");
#endif
  breaking = simulating = 0;
  dialog_simulator_update ();

#if DEBUG
  printf ("Freeing threads\n");
#endif
  for (i = 0; i < nthreads; ++i)
    gsl_rng_free (rng[i]);
  g_slice_free1 (medium_bytes, medium);
}
