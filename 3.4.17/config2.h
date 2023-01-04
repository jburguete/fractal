/*
FRACTAL - A program growing fractals to benchmark parallelization and drawing
libraries.

Copyright 2009-2023, Javier Burguete Tolosa.

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
 * \file config.h
 * \brief Header file to define basic configuration and compilation options.
 * \author Javier Burguete Tolosa.
 * \copyright Copyright 2009-2022, Javier Burguete Tolosa.
 */
#ifndef CONFIG__H
#define CONFIG__H 1


#define DEBUG 0                 ///< Macro to debug the code.

#define HEIGHT 200              ///< Macro to set the default medium height.
#define LENGTH 320              ///< Macro to set the default medium length.
#define SEED 7007L
///< Macro to set the default pseudo-random number generator seed.
#define WIDTH 320               ///< Macro to set the default medium width.

#define XML_3D          (const xmlChar *)"iii-d"
///< iii-d XML label.
#define XML_ANIMATE     (const xmlChar *)"animate"
///< animate XML label.
#define XML_CLOCK       (const xmlChar *)"clock"
///< clock XML label.
#define XML_CMRG        (const xmlChar *)"cmrg"
///< cmrg XML label.
#define XML_DEFAULT     (const xmlChar *)"default"
///< default XML label.
#define XML_DIAGONAL    (const xmlChar *)"diagonal"
///< diagonal XML label.
#define XML_FIXED       (const xmlChar *)"fixed"
///< fixed XML label.
#define XML_FOREST      (const xmlChar *)"forest"
///< forest XML label.
#define XML_FRACTAL     (const xmlChar *)"fractal"
///< fractal XML label.
#define XML_GFSR4       (const xmlChar *)"gfsr4"
///< gfsr4 XML label.
#define XML_HEIGHT      (const xmlChar *)"height"
///< height XML label.
#define XML_LENGTH      (const xmlChar *)"length"
///< length XML label.
#define XML_MRG         (const xmlChar *)"mrg"
///< mrg XML label.
#define XML_MT19937     (const xmlChar *)"mt19937"
///< mt19937 XML label.
#define XML_NEURON      (const xmlChar *)"neuron"
///< neuron XML label.
#define XML_NO          (const xmlChar *)"no"
///< no XML label.
#define XML_RANDOM_SEED (const xmlChar *)"random-seed"
///< random-seed XML label.
#define XML_RANDOM_TYPE (const xmlChar *)"random-type"
///< random-type XML label.
#define XML_RANLXS0     (const xmlChar *)"ranlxs0"
///< ranlxs0 XML label.
#define XML_RANLXS1     (const xmlChar *)"ranlxs1"
///< ranlxs1 XML label.
#define XML_RANLXS2     (const xmlChar *)"ranlxs2"
///< ranlxs2 XML label.
#define XML_RANLXD1      (const xmlChar *)"ranlxd1"
///< ranlxd1 XML label.
#define XML_RANLXD2      (const xmlChar *)"ranlxd2"
///< ranlxd2 XML label.
#define XML_RANLUX       (const xmlChar *)"ranlux"
///< ranlux XML label.
#define XML_RANLUX389   (const xmlChar *)"ranlux389"
///< ranlux389 XML label.
#define XML_SEED        (const xmlChar *)"seed"
///< seed XML label.
#define XML_TAUS2       (const xmlChar *)"taus2"
///< taus2 XML label.
#define XML_THREADS     (const xmlChar *)"threads"
///< threads XML label.
#define XML_TREE        (const xmlChar *)"tree"
///< tree XML label.
#define XML_TYPE        (const xmlChar *)"type"
///< type XML label.
#define XML_WIDTH       (const xmlChar *)"width"
///< width XML label.
#define XML_YES         (const xmlChar *)"yes"
///< yes XML label.

#if !HAVE_SINCOS
static inline void
sincos (double x, double *s, double *c)
{
  *s = sin (x);
  *c = cos (x);
}
#endif

#if !HAVE_SINCOSF
static inline void
sincosf (float x, float *s, float *c)
{
  *s = sinf (x);
  *c = cosf (x);
}
#endif

#if !GTK4

#define gtk_box_append(box, child) (gtk_box_pack_start(box, child, 0, 0, 0))
#define gtk_check_button_get_active(button) \
  (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
#define gtk_check_button_set_active(button, active) \
  (gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), active))
#define gtk_file_chooser_get_filename gtk_file_chooser_get_current_name
#define gtk_frame_set_child(frame, child) \
  (gtk_container_add(GTK_CONTAINER(frame), child))
#define gtk_window_destroy(window) (gtk_widget_destroy(GTK_WIDGET(window)))

#endif

#endif
