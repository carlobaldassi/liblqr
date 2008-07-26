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

#include <glib.h>
#include <lqr/lqr_base.h>
#include <lqr/lqr_gradient_pub.h>
#include <lqr/lqr_progress_pub.h>
#include <lqr/lqr_cursor_pub.h>
#include <lqr/lqr_vmap_pub.h>
#include <lqr/lqr_vmap_list_pub.h>
#include <lqr/lqr_carver_list_pub.h>
#include <lqr/lqr_carver_pub.h>
#include <lqr/lqr_energy.h>

/* read average pixel value at x, y 
 * for energy computation */
inline gdouble
lqr_carver_read (LqrCarver * r, gint x, gint y)
{
  gdouble sum = 0;
  gint k;
  gint now = r->raw[y][x];
  for (k = 0; k < r->channels; k++)
    {
      sum += R_RGB(r->rgb, now * r->channels + k);
    }
  return sum / (R_RGB_MAX * r->channels);
}


/* compute energy at x, y */
void
lqr_carver_compute_e (LqrCarver * r, gint x, gint y)
{
  gdouble gx, gy;
  gint data;

  if (y == 0)
    {
      gy = lqr_carver_read (r, x, y + 1) - lqr_carver_read (r, x, y);
    }
  else if (y < r->h - 1)
    {
      gy =
        (lqr_carver_read (r, x, y + 1) - lqr_carver_read (r, x, y - 1)) / 2;
    }
  else
    {
      gy = lqr_carver_read (r, x, y) - lqr_carver_read (r, x, y - 1);
    }

  if (x == 0)
    {
      gx = lqr_carver_read (r, x + 1, y) - lqr_carver_read (r, x, y);
    }
  else if (x < r->w - 1)
    {
      gx =
        (lqr_carver_read (r, x + 1, y) - lqr_carver_read (r, x - 1, y)) / 2;
    }
  else
    {
      gx = lqr_carver_read (r, x, y) - lqr_carver_read (r, x - 1, y);
    }
  data = r->raw[y][x];
  r->en[data] = (*(r->gf)) (gx, gy) + r->bias[data] / r->w_start;
}

