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
 * \file fractal.h
 * \brief Header file to define the fractal data and functions.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2019, Javier Burguete Tolosa.
 */
#ifndef FRACTAL__H
#define FRACTAL__H 1

/**
 * \struct Point
 * \brief A struct to define the point data.
 */
typedef struct
{
  int x;                        ///< x-coordinate.
  int y;                        ///< y-coordinate.
  int z;                        ///< z-coordinate.
  unsigned int c;               ///< color.
} Point;

/**
 * \struct Point3D
 * \brief A struct to define the 3D OpenGL point data.
 */
typedef struct
{
  float r[3];                   ///< position vector.
  float c[3];                   ///< color vector.
} Point3D;

///> An enum to define fractal types.
enum FractalType
{
  FRACTAL_TYPE_TREE = 0,        ///< Tree fractal type.
  FRACTAL_TYPE_FOREST = 1,      ///< Forest fractal type.
  FRACTAL_TYPE_NEURON = 2       ///< Neuron fractal type.
};

#define N_FRACTAL_TYPES (FRACTAL_TYPE_NEURON + 1)
  ///< Macro to define the number of fractal types.

///> An enum to define random algorithm types.
enum RandomType
{
  RANDOM_TYPE_MT19937 = 0,
  ///< gsl_rng_mt19937 random numbers generator algorithm.
  RANDOM_TYPE_RANLXS0 = 1,
  ///< gsl_rng_ranlxs0 random numbers generator algorithm.
  RANDOM_TYPE_RANLXS1 = 2,
  ///< gsl_rng_ranlxs1 random numbers generator algorithm.
  RANDOM_TYPE_RANLXS2 = 3,
  ///< gsl_rng_ranlxs2 random numbers generator algorithm.
  RANDOM_TYPE_RANLXD1 = 4,
  ///< gsl_rng_ranlxd1 random numbers generator algorithm.
  RANDOM_TYPE_RANLXD2 = 5,
  ///< gsl_rng_ranlxd2 random numbers generator algorithm.
  RANDOM_TYPE_RANLUX = 6,
  ///< gsl_rng_ranlux random numbers generator algorithm.
  RANDOM_TYPE_RANLUX389 = 7,
  ///< gsl_rng_ranlux389 random numbers generator algorithm.
  RANDOM_TYPE_CMRG = 8,
  ///< gsl_rng_cmrg random numbers generator algorithm.
  RANDOM_TYPE_MRG = 9,
  ///< gsl_rng_mrg random numbers generator algorithm.
  RANDOM_TYPE_TAUS2 = 10,
  ///< gsl_rng_taus2 random numbers generator algorithm.
  RANDOM_TYPE_GFSR4 = 11
    ///< gsl_rng_gfsr4 random numbers generator algorithm.
};

#define N_RANDOM_TYPES (RANDOM_TYPE_GFSR4 + 1)
  ///< Macro to define the number of random numbers generator algorithm types.

///> An enum to define random seed types.
enum RandomSeedType
{
  RANDOM_SEED_TYPE_DEFAULT = 0, ///< default random seed.
  RANDOM_SEED_TYPE_CLOCK = 1,   ///< clock based random seed.
  RANDOM_SEED_TYPE_FIXED = 2    ///< user fixed random seed.
};

#define N_RANDOM_SEED_TYPES (RANDOM_SEED_TYPE_FIXED + 1)
  ///< Macro to define the number of random seed types.

// PARALLELIZING DATA
extern unsigned int nthreads;
extern GMutex mutex[1];
// END

extern unsigned int fractal_type, fractal_points, fractal_3D, fractal_diagonal;
extern unsigned int width, height, length, area, breaking, simulating,
  animating;
extern unsigned long t0;

extern float xmin, xmax, ymin, ymax, phid, thetad, phi, theta;

extern unsigned int max_d;
extern unsigned int *medium;
extern Point3D *point;
extern unsigned int npoints;

extern const float color3f[16][3];

extern unsigned int random_algorithm, random_seed_type;
extern unsigned long random_seed;

extern void *(*parallel_fractal) (gsl_rng * rng);

/**
 * Function the set the perspective of a point.
 */
static inline void
perspective (int x,             ///< Point x-coordinate.
             int y,             ///< Point y-coordinate.
             int z,             ///< Point z-coordinate.
             float *X,          ///< Perspective X-coordinate.
             float *Y)          ///< Perspective Y-coordinate.
{
  *X = x * cos (phi) - y * sin (phi);
  *Y = z * sin (theta) + (y * cos (phi) + x * sin (phi)) * cos (theta);
}

int threads_number ();
void fractal_stop ();
void medium_start ();
int fractal_input (char *filename);
void fractal ();

#endif
