/* LiquidRescaling Library
 * Copyright (C) 2007 Carlo Baldassi (the "Author") <carlobaldassi@yahoo.it>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/> 
 */

#ifndef __LQR_CARVER_H__
#define __LQR_CARVER_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_H__
#error "lqr_gradient.h must be included prior to lqr_carver.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_CARVER_LIST_H__
#error "lqr_carver_list.h must be included prior to lqr_carver.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_VMAP_H__
#error "lqr_vmap.h must be included prior to lqr_carver.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_VMAP_LIST_H__
#error "lqr_vmap_list.h must be included prior to lqr_carver.h"
#endif /* __LQR_GRADIENT_H__ */

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

  gint bpp;                     /* number of bpp of the image */

  gint transposed;              /* flag to set transposed state */
  gboolean active;              /* flag to set if carver is active */

  gboolean resize_aux_layers;   /* flag to determine whether the auxiliary layers are resized */
  gboolean dump_vmaps;         /* flag to determine whether to output the seam map */
  LqrResizeOrder resize_order;  /* resize order */

  LqrCarverList *attached_list; /* list of attached carvers */

  gfloat rigidity;              /* rigidity value (can straighten seams) */
  gdouble *rigidity_map;        /* the rigidity function */
  gint delta_x;                 /* max displacement of seams (currently is only meaningful if 0 or 1 */

  guchar *rgb;                  /* array of rgb points */
  gint *vs;                     /* array of visibility levels */
  gdouble *en;                  /* array of energy levels */
  gdouble *bias;                /* array of energy levels */
  gdouble *m;                   /* array of auxiliary energy values */
  gint *least;                  /* array of pointers */
  gint *_raw;                   /* array of array-coordinates, for seam computation */
  gint **raw;                   /* array of array-coordinates, for seam computation */

  LqrCursor *c;                 /* cursor to be used as image reader */
  guchar *rgb_ro_buffer;	/* readout buffer */

  gint *vpath;                  /* array of array-coordinates representing a vertical seam */
  gint *vpath_x;                /* array of abscisses representing a vertical seam */

  LqrGradFunc gf;                    /* pointer to a gradient function */

  LqrProgress * progress;	/* pointer to progress update functions */

  LqrVMapList * flushed_vs;  /* linked list of pointers to flushed visibility maps buffers */

};


/* LQR_CARVER CLASS FUNCTIONS */

/** private functions **/

/* build maps */
LqrRetVal lqr_carver_build_maps (LqrCarver * r, gint depth);     /* build all */
void lqr_carver_build_emap (LqrCarver * r);     /* energy */
void lqr_carver_build_mmap (LqrCarver * r);     /* minpath */
LqrRetVal lqr_carver_build_vsmap (LqrCarver * r, gint depth);    /* visibility */

/* internal functions for maps computation */
inline gdouble lqr_carver_read (LqrCarver * r, gint x, gint y); /* read the average value at given point */
void lqr_carver_compute_e (LqrCarver * r, gint x, gint y);      /* compute energy of point at c (fast) */
void lqr_carver_update_emap (LqrCarver * r);    /* update energy map after seam removal */
void lqr_carver_update_mmap (LqrCarver * r);    /* minpath */
void lqr_carver_build_vpath (LqrCarver * r);    /* compute seam path */
void lqr_carver_carve (LqrCarver * r);  /* updates the "raw" buffer */
void lqr_carver_update_vsmap (LqrCarver * r, gint l);   /* update visibility map after seam removal */
void lqr_carver_finish_vsmap (LqrCarver * r);   /* complete visibility map (last seam) */
void lqr_carver_copy_vsmap (LqrCarver * r, LqrCarver * dest);   /* copy vsmap on another carver */
LqrRetVal lqr_carver_inflate (LqrCarver * r, gint l);    /* adds enlargment info to map */

/* image manipulations */
LqrRetVal lqr_carver_resize_width (LqrCarver * r, gint w1);   /* liquid resize width */
LqrRetVal lqr_carver_resize_height (LqrCarver * r, gint h1);   /* liquid resize height */
void lqr_carver_set_width (LqrCarver * r, gint w1);
LqrRetVal lqr_carver_transpose (LqrCarver * r);

/* auxiliary */
LqrRetVal lqr_carver_set_width1 (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_flatten1 (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_transpose1 (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_copy_vsmap1 (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_inflate1 (LqrCarver * r, LqrDataTok data);

/** public functions **/

/* constructor & destructor */
LqrCarver * lqr_carver_new (guchar * buffer, gint width, gint height, gint bpp);
void lqr_carver_destroy (LqrCarver * r);

/* initialize */
LqrRetVal lqr_carver_init (LqrCarver *r, gint delta_x, gfloat rigidity);

/* set attributes */
void lqr_carver_set_gradient_function (LqrCarver * r, LqrGradFuncType gf_ind);
void lqr_carver_set_dump_vmaps (LqrCarver *r);
void lqr_carver_set_resize_order (LqrCarver *r, LqrResizeOrder resize_order);
LqrRetVal lqr_carver_attach (LqrCarver * r, LqrCarver * aux);
void lqr_carver_set_progress (LqrCarver *r, LqrProgress *p);

/* image manipulations */
LqrRetVal lqr_carver_resize (LqrCarver * r, gint w1, gint h1);   /* liquid resize */
LqrRetVal lqr_carver_flatten (LqrCarver * r);    /* flatten the multisize image */
LqrRetVal lqr_carver_swoosh (LqrCarver * r);    /* flatten and transpose the multisize image */

/* readout */
void lqr_carver_scan_reset (LqrCarver * r);
gboolean lqr_carver_scan (LqrCarver *r, gint *x, gint *y, guchar ** rgb);
gint lqr_carver_get_bpp (LqrCarver *r);
gint lqr_carver_get_width (LqrCarver * r);
gint lqr_carver_get_height (LqrCarver * r);
gint lqr_carver_read_x (LqrCarver * r);
gint lqr_carver_read_y (LqrCarver * r);
gboolean lqr_carver_read_next (LqrCarver * r);
guchar lqr_carver_read_c (LqrCarver * r, gint col);


#endif /* __LQR_CARVER_H__ */
