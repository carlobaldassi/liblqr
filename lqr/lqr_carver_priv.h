/* LiquidRescaling Library
 * Copyright (C) 2007-2008 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007-2008.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/> 
 */

#ifndef __LQR_CARVER_PRIV_H__
#define __LQR_CARVER_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_H__
#error "lqr_gradient.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_CARVER_LIST_H__
#error "lqr_carver_list.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_CARVER_LIST_H__ */

#ifndef __LQR_VMAP_H__
#error "lqr_vmap_priv.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_VMAP_H__ */

#ifndef __LQR_VMAP_LIST_H__
#error "lqr_vmap_list.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_VMAP_LIST_H__ */

/* Macros for internal use */

#define AS_8I(x) ((lqr_8i*)x)
#define AS_16I(x) ((lqr_16i*)x)
#define AS_32F(x) ((lqr_32f*)x)
#define AS_64F(x) ((lqr_64f*)x)

#define AS2_8I(x) ((lqr_8i**)x)
#define AS2_16I(x) ((lqr_16i**)x)
#define AS2_32F(x) ((lqr_32f**)x)
#define AS2_64F(x) ((lqr_64f**)x)

/* LQR_CARVER CLASS PRIVATE FUNCTIONS */

/* constructor base */
LqrCarver * lqr_carver_new_common (gint width, gint height, gint channels);

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
LqrRetVal lqr_carver_inflate (LqrCarver * r, gint l);    /* adds enlargment info to map */
LqrRetVal lqr_carver_propagate_vsmap (LqrCarver * r);    /* propagates vsmap on attached carvers */

/* image manipulations */
LqrRetVal lqr_carver_resize_width (LqrCarver * r, gint w1);   /* liquid resize width */
LqrRetVal lqr_carver_resize_height (LqrCarver * r, gint h1);   /* liquid resize height */
void lqr_carver_set_width (LqrCarver * r, gint w1);
LqrRetVal lqr_carver_transpose (LqrCarver * r);
void lqr_carver_scan_reset_all (LqrCarver * r);

/* auxiliary */
LqrRetVal lqr_carver_scan_reset_attached (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_set_width_attached (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_inflate_attached (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_flatten_attached (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_transpose_attached (LqrCarver * r, LqrDataTok data);
LqrRetVal lqr_carver_propagate_vsmap_attached (LqrCarver * r, LqrDataTok data);

#endif /* __LQR_CARVER_PRIV_H__ */
