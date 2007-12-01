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

#include <stdio.h>

#include <lqr/lqr.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif

/**** LQR_CARVER_BIAS STRUCT FUNTIONS ****/

gboolean
lqr_carver_bias_add_area(LqrCarver *r, gdouble *buffer, gint bias_factor, gint width, gint height, gint x_off, gint y_off)
{
  gint x, y;
  gint x1, y1, x2, y2;
  gdouble bias;

  if (bias_factor == 0)
    {
      return TRUE;
    }

  TRY_F_F (r->active);
  TRY_F_F (!r->transposed);

  x1 = MAX (0, x_off);
  y1 = MAX (0, y_off);
  x2 = MIN (r->w, width + x_off);
  y2 = MIN (r->h, height + y_off);

  for (y = 0; y < y2 - y1; y++)
    {
      for (x = 0; x < x2 - x1; x++)
        {
          bias = (double) bias_factor * buffer[y * width + x] / 2;

          r->bias[(y + y1) * r->w0 + (x + x1)] += bias;

        }

    }

  return TRUE;
}


gboolean
lqr_carver_bias_add(LqrCarver *r, gdouble *buffer, gint bias_factor)
//update_bias (LqrCarver *r, gint32 layer_ID, gint bias_factor, gint base_x_off, gint base_y_off)
{
  return lqr_carver_bias_add_area(r, buffer, bias_factor, r->w0, r->h0, 0, 0);
}

gboolean
lqr_carver_bias_add_rgb_area(LqrCarver *r, guchar *rgb, gint bias_factor, gint bpp, gint width, gint height, gint x_off, gint y_off)
{
  gint x, y, k, c_bpp;
  gboolean has_alpha;
  gint x1, y1, x2, y2;
  gint sum;
  gdouble bias;

  if (bias_factor == 0)
    {
      return TRUE;
    }

  TRY_F_F (r->active);
  TRY_F_F (!r->transposed);

  has_alpha = (bpp == 2 || bpp >= 4);
  c_bpp = bpp - (has_alpha ? 1 : 0);

  x1 = MAX (0, x_off);
  y1 = MAX (0, y_off);
  x2 = MIN (r->w, width + x_off);
  y2 = MIN (r->h, height + y_off);

  for (y = 0; y < y2 - y1; y++)
    {
      for (x = 0; x < x2 - x1; x++)
        {
          sum = 0;
          for (k = 0; k < c_bpp; k++)
            {
              sum += rgb[(y * width + x) * bpp + k];
            }

          bias = (double) bias_factor * sum / (2 * 255 * c_bpp);
          if (has_alpha)
            {
	      bias *= (gdouble) rgb[(y * width + x + 1) * bpp - 1] / 255;
            }

          r->bias[(y + y1) * r->w0 + (x + x1)] += bias;

        }

    }

  return TRUE;
}

gboolean
lqr_carver_bias_add_rgb(LqrCarver *r, guchar *rgb, gint bias_factor, gint bpp)
{
  return lqr_carver_bias_add_rgb_area(r, rgb, bias_factor, bpp, r->w0, r->h0, 0, 0);
}


/**** END OF LQR_CARVER_BIAS CLASS FUNCTIONS ****/
