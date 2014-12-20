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
 * \brief An enum to define fractal types.
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

/**
 * \def N_FRACTAL_TYPES
 * \brief Macro to define the number of fractal types.
 */
#define N_FRACTAL_TYPES (FRACTAL_TYPE_NEURON + 1)

/**
 * \enum RandomType
 * \brief An enum to define random algorithm types.
 * \var RANDOM_TYPE_MT19937
 * \brief gsl_rng_mt19937 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLXS0
 * \brief gsl_rng_ranlxs0 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLXS1
 * \brief gsl_rng_ranlxs1 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLXS2
 * \brief gsl_rng_ranlxs2 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLXD1
 * \brief gsl_rng_ranlxd1 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLXD2
 * \brief gsl_rng_ranlxd2 random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLUX
 * \brief gsl_rng_ranlux random numbers generator algorithm.
 * \var RANDOM_TYPE_RANLUX389
 * \brief gsl_rng_ranlux389 random numbers generator algorithm.
 * \var RANDOM_TYPE_CMRG
 * \brief gsl_rng_cmrg random numbers generator algorithm.
 * \var RANDOM_TYPE_MRG
 * \brief gsl_rng_mrg random numbers generator algorithm.
 * \var RANDOM_TYPE_TAUS2
 * \brief gsl_rng_taus2 random numbers generator algorithm.
 * \var RANDOM_TYPE_GFSR4
 * \brief gsl_rng_gfsr4 random numbers generator algorithm.
 */
enum RandomType
{
	RANDOM_TYPE_MT19937 = 0,
	RANDOM_TYPE_RANLXS0 = 1,
	RANDOM_TYPE_RANLXS1 = 2,
	RANDOM_TYPE_RANLXS2 = 3,
	RANDOM_TYPE_RANLXD1 = 4,
	RANDOM_TYPE_RANLXD2 = 5,
	RANDOM_TYPE_RANLUX = 6,
	RANDOM_TYPE_RANLUX389 = 7,
	RANDOM_TYPE_CMRG = 8,
	RANDOM_TYPE_MRG = 9,
	RANDOM_TYPE_TAUS2 = 10,
	RANDOM_TYPE_GFSR4 = 11
};

/**
 * \def N_RANDOM_TYPES
 * \brief Macro to define the number of random numbers generator algorithm
 *   types.
 */
#define N_RANDOM_TYPES (RANDOM_TYPE_GFSR4 + 1)

/**
 * \enum RandomSeedType
 * \brief A enum to define random seed types.
 * \var RANDOM_SEED_TYPE_DEFAULT
 * \brief default random seed.
 * \var RANDOM_SEED_TYPE_CLOCK
 * \brief clock based random seed.
 * \var RANDOM_SEED_TYPE_FIXED
 * \brief user fixed random seed.
 */
enum RandomSeedType
{
	RANDOM_SEED_TYPE_DEFAULT = 0,
	RANDOM_SEED_TYPE_CLOCK = 1,
	RANDOM_SEED_TYPE_FIXED = 2
};

/**
 * \def N_RANDOM_SEED_TYPES
 * \brief Macro to define the number of random seed types.
 */
#define N_RANDOM_SEED_TYPES (RANDOM_SEED_TYPE_FIXED + 1)

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
extern GList *list_points;

extern unsigned int random_algorithm, random_seed_type;
extern unsigned long random_seed;

extern void* (*parallel_fractal)(gsl_rng *rng);

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
