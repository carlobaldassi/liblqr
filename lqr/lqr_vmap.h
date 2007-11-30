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

#ifndef __LQR_VMAP_H__
#define __LQR_VMAP_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_vmap.h"
#endif /* __LQR_BASE_H__ */


/*** LQR_VMAP CLASS DEFINITION ***/
struct _LqrVMap
{
  gint * buffer;
  gint width;
  gint height;
  gint depth;
  gint orientation;
};

typedef struct _LqrVMap LqrVMap;

typedef gboolean (*LqrVMapFunc) (LqrVMap *vmap, gpointer data);

/* LQR_VMAP FUNCTIONS */

LqrVMap* lqr_vmap_new (gint *buffer, gint width, gint heigth, gint depth, gint orientation);
void lqr_vmap_destroy (LqrVMap *vmap);

gboolean lqr_vmap_flush (LqrCarver *r);
gboolean lqr_vmap_load (LqrCarver *r, LqrVMap *vmap);


#endif /* __LQR_VMAP__ */

