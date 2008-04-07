/* LiquidRescaling Library
 * Copyright (C) 2007 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/> 
 */

#ifndef __LQR_CARVER_PUB_H__
#define __LQR_CARVER_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_PUB_H__
#error "lqr_gradient_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_GRADIENT_PUB_H__ */

#ifndef __LQR_CARVER_LIST_PUB_H__
#error "lqr_carver_list_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_CARVER_LIST_PUB_H__ */

#ifndef __LQR_VMAP_LIST_PUB_H__
#error "lqr_vmap_list_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_VMAP_LIST_PUB_H__ */

#define R_RGB(rgb, z) ((r->bits == 8) ? ((guchar*)rgb)[z] : ((guint16*)rgb)[z])
#define R_RGB_MAX ((1 << r->bits) - 1)

/**** LQR_CARVER CLASS DEFINITION ****/
/* This is the representation of the multisize image
 * The image is stored internally as a one-dimentional
 * array of LqrData points, called map.
 * The points are ordered by rows. */
struct _LqrCarver
{
  gint w_start, h_start;        /* original width & height */
  gint w, h;                    /* current width & height */
  gint w0, h0;                  /* map array width & height */

  gint level;                   /* (in)visibility level (1 = full visibility) */
  gint max_level;               /* max level computed so far
                                 * it is not level <= max_level
                                 * but rather level <= 2 * max_level - 1
                                 * since levels are shifted upon inflation
                                 */

  gint channels;                     /* number of colour channels of the image */
  gint bits;			/* number of bits per channel */

  gint transposed;              /* flag to set transposed state */
  gboolean active;              /* flag to set if carver is active */

  gboolean resize_aux_layers;   /* flag to determine whether the auxiliary layers are resized */
  gboolean dump_vmaps;         /* flag to determine whether to output the seam map */
  LqrResizeOrder resize_order;  /* resize order */

  LqrCarverList *attached_list; /* list of attached carvers */

  gfloat rigidity;              /* rigidity value (can straighten seams) */
  gdouble *rigidity_map;        /* the rigidity function */
  gdouble *rigidity_mask;	/* the rigidity mask */
  gint delta_x;                 /* max displacement of seams (currently is only meaningful if 0 or 1 */

  void *rgb;                    /* array of rgb points */
  gint *vs;                     /* array of visibility levels */
  gdouble *en;                  /* array of energy levels */
  gdouble *bias;                /* bias mask */
  gdouble *m;                   /* array of auxiliary energy values */
  gint *least;                  /* array of pointers */
  gint *_raw;                   /* array of array-coordinates, for seam computation */
  gint **raw;                   /* array of array-coordinates, for seam computation */

  LqrCursor *c;                 /* cursor to be used as image reader */
  void *rgb_ro_buffer;	        /* readout buffer */

  gint *vpath;                  /* array of array-coordinates representing a vertical seam */
  gint *vpath_x;                /* array of abscisses representing a vertical seam */

  LqrGradFunc gf;                    /* pointer to a gradient function */

  gint leftright;		/* whether to favor left or right seams */
  gint lr_switch_frequency;	/* interval between leftright switches */

  LqrProgress * progress;	/* pointer to progress update functions */

  LqrVMapList * flushed_vs;  /* linked list of pointers to flushed visibility maps buffers */

};


/* LQR_CARVER CLASS PUBLIC FUNCTIONS */

/* constructor & destructor */
LqrCarver * lqr_carver_new (guchar * buffer, gint width, gint height, gint channels);
LqrCarver * lqr_carver_new_16 (guint16 * buffer, gint width, gint height, gint channels);
void lqr_carver_destroy (LqrCarver * r);

/* initialize */
LqrRetVal lqr_carver_init (LqrCarver *r, gint delta_x, gfloat rigidity);

/* set attributes */
void lqr_carver_set_gradient_function (LqrCarver * r, LqrGradFuncType gf_ind);
void lqr_carver_set_dump_vmaps (LqrCarver *r);
void lqr_carver_set_resize_order (LqrCarver *r, LqrResizeOrder resize_order);
void lqr_carver_set_side_switch_frequency (LqrCarver *r, gint switch_interval);
LqrRetVal lqr_carver_attach (LqrCarver * r, LqrCarver * aux);
void lqr_carver_set_progress (LqrCarver *r, LqrProgress *p);

/* image manipulations */
LqrRetVal lqr_carver_resize (LqrCarver * r, gint w1, gint h1);   /* liquid resize */
LqrRetVal lqr_carver_flatten (LqrCarver * r);    /* flatten the multisize image */

/* readout */
void lqr_carver_scan_reset (LqrCarver * r);
gboolean lqr_carver_scan (LqrCarver *r, gint *x, gint *y, guchar ** rgb);
gboolean lqr_carver_scan_line (LqrCarver * r, gint * n, guchar ** rgb);
gboolean lqr_carver_scan_16 (LqrCarver *r, gint *x, gint *y, guint16 ** rgb);
gboolean lqr_carver_scan_line_16 (LqrCarver * r, gint * n, guint16 ** rgb);
gboolean lqr_carver_scan_by_row (LqrCarver *r);
gint lqr_carver_get_bpp (LqrCarver *r);
gint lqr_carver_get_channels (LqrCarver *r);
gint lqr_carver_get_width (LqrCarver * r);
gint lqr_carver_get_height (LqrCarver * r);


#endif /* __LQR_CARVER_PUB_H__ */
