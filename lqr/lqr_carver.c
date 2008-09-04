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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <lqr/lqr_all.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif /* __LQR_DEBUG__ */

/**** LQR_CARVER CLASS FUNCTIONS ****/

/*** constructor & destructor ***/

/* constructors */
LqrCarver * lqr_carver_new_common (gint width, gint height, gint channels)
{
  LqrCarver *r;

  TRY_N_N (r = g_try_new (LqrCarver, 1));

  r->level = 1;
  r->max_level = 1;
  r->transposed = 0;
  r->active = FALSE;
  r->root = NULL;
  r->rigidity = 0;
  r->resize_aux_layers = FALSE;
  r->dump_vmaps = FALSE;
  r->resize_order = LQR_RES_ORDER_HOR;
  r->attached_list = NULL;
  r->flushed_vs = NULL;
  TRY_N_N (r->progress = lqr_progress_new());

  r->en = NULL;
  r->bias = NULL;
  r->m = NULL;
  r->least = NULL;
  r->_raw = NULL;
  r->raw = NULL;
  r->vpath = NULL;
  r->vpath_x = NULL;
  r->rigidity_map = NULL;
  r->rigidity_mask = NULL;
  r->delta_x = 1;

  r->h = height;
  r->w = width;
  r->channels = channels;

  r->w0 = r->w;
  r->h0 = r->h;
  r->w_start = r->w;
  r->h_start = r->h;

  lqr_carver_set_gradient_function(r, LQR_GF_XABS);

  r->leftright = 0;
  r->lr_switch_frequency = 0;

  TRY_N_N (r->vs = g_try_new0 (gint, r->w * r->h));

  /* initialize cursor */

  TRY_N_N (r->c = lqr_cursor_create (r));

  return r;
}

LQR_PUBLIC
LqrCarver *
lqr_carver_new (guchar * buffer, gint width, gint height, gint channels)
{
  LqrCarver *r;

  TRY_N_N (r = lqr_carver_new_common (width, height, channels));

  r->rgb = (void*) buffer;
  TRY_N_N (r->rgb_ro_buffer = g_try_new (guchar, r->channels * r->w));
  r->bits = 8;

  return r;
}

LQR_PUBLIC
LqrCarver *
lqr_carver_new_16 (guint16 * buffer, gint width, gint height, gint channels)
{
  LqrCarver *r;

  TRY_N_N (r = lqr_carver_new_common (width, height, channels));

  r->rgb = (void*) buffer;
  TRY_N_N (r->rgb_ro_buffer = g_try_new (guint16, r->channels * r->w));
  r->bits = 16;

  return r;
}



/* destructor */
LQR_PUBLIC
void
lqr_carver_destroy (LqrCarver * r)
{
  g_free (r->rgb);
  if (r->root == NULL)
    {
      g_free (r->vs);
    }
  g_free (r->en);
  g_free (r->bias);
  g_free (r->m);
  g_free (r->least);

  lqr_cursor_destroy (r->c);
  g_free (r->vpath);
  g_free (r->vpath_x);
  if (r->rigidity_map != NULL)
    {
      r->rigidity_map -= r->delta_x;
      g_free (r->rigidity_map);
    }
  g_free (r->rigidity_mask);
  lqr_vmap_list_destroy(r->flushed_vs);
  lqr_carver_list_destroy(r->attached_list);
  g_free (r->progress);
  g_free (r->_raw);
  g_free (r->raw);
  g_free (r);
}

/*** initialization ***/

LQR_PUBLIC
LqrRetVal
lqr_carver_init (LqrCarver *r, gint delta_x, gfloat rigidity)
{
  gint y, x;

  CATCH_MEM (r->en = g_try_new (gdouble, r->w * r->h));
  CATCH_MEM (r->bias = g_try_new0 (gdouble, r->w * r->h));
  CATCH_MEM (r->m = g_try_new (gdouble, r->w * r->h));
  CATCH_MEM (r->least = g_try_new (gint, r->w * r->h));

  CATCH_MEM (r->_raw = g_try_new (gint, r->h_start * r->w_start));
  CATCH_MEM (r->raw = g_try_new (gint *, r->h_start));

  for (y = 0; y < r->h; y++)
    {
      r->raw[y] = r->_raw + y * r->w_start;
      for (x = 0; x < r->w_start; x++)
        {
	  r->raw[y][x] = y * r->w_start + x;
	}
    }

  CATCH_MEM (r->vpath = g_try_new (gint, r->h));
  CATCH_MEM (r->vpath_x = g_try_new (gint, r->h));

  /* set rigidity map */
  r->delta_x = delta_x;
  r->rigidity = rigidity;

  r->rigidity_map = g_try_new0 (gdouble, 2 * r->delta_x + 1);
  r->rigidity_map += r->delta_x;
  for (x = -r->delta_x; x <= r->delta_x; x++)
    {
      r->rigidity_map[x] =
        (gdouble) r->rigidity * pow(fabs(x), 1.5) / r->h;
    }

  r->active = TRUE;

  return LQR_OK;
}

/*** set attributes ***/

/* gradient function for energy computation */
LQR_PUBLIC
void
lqr_carver_set_gradient_function (LqrCarver * r, LqrGradFuncType gf_ind)
{
  switch (gf_ind)
    {
    case LQR_GF_NORM:
      r->gf = &lqr_grad_norm;
      break;
    case LQR_GF_NORM_BIAS:
      r->gf = &lqr_grad_norm_bias;
      break;
    case LQR_GF_SUMABS:
      r->gf = &lqr_grad_sumabs;
      break;
    case LQR_GF_XABS:
      r->gf = &lqr_grad_xabs;
      break;
    case LQR_GF_YABS:
      r->gf = &lqr_grad_yabs;
      break;
    case LQR_GF_NULL:
      r->gf = &lqr_grad_zero;
      break;
#ifdef __LQR_DEBUG__
    default:
      assert (0);
#endif /* __LQR_DEBUG__ */
    }
}

/* attach carvers to be scaled along with the main one */
LQR_PUBLIC
LqrRetVal
lqr_carver_attach (LqrCarver * r, LqrCarver * aux)
{
  CATCH_F (r->w0 == aux->w0);
  CATCH_F (r->h0 == aux->h0);
  /* lqr_carver_copy_vsmap (r, aux); */
  CATCH_MEM (r->attached_list = lqr_carver_list_append (r->attached_list, aux));
  g_free(aux->vs);
  aux->vs = r->vs;
  aux->root = r;
  return LQR_OK;
}

/* set the seam output flag */
LQR_PUBLIC
void
lqr_carver_set_dump_vmaps (LqrCarver *r)
{
  r->dump_vmaps = TRUE;
}

/* set order if rescaling in both directions */
LQR_PUBLIC
void
lqr_carver_set_resize_order (LqrCarver *r, LqrResizeOrder resize_order)
{
  r->resize_order = resize_order;
}

/* set leftright switch interval */
LQR_PUBLIC
void
lqr_carver_set_side_switch_frequency (LqrCarver *r, gint switch_frequency)
{
  r->lr_switch_frequency = switch_frequency;
}

/* set progress reprot */
LQR_PUBLIC
void
lqr_carver_set_progress (LqrCarver *r, LqrProgress *p)
{
  g_free(r->progress);
  r->progress = p;
}


/*** compute maps (energy, minpath & visibility) ***/

/* build multisize image up to given depth
 * it is progressive (can be called multilple times) */
LqrRetVal
lqr_carver_build_maps (LqrCarver * r, gint depth)
{
#ifdef __LQR_DEBUG__
  assert (depth <= r->w_start);
  assert (depth >= 1);
#endif /* __LQR_DEBUG__ */

  /* only go deeper if needed */
  if (depth > r->max_level)
    {
      CATCH_F (r->active);
      CATCH_F (r->root == NULL);

      /* set to minimum width reached so far */
      lqr_carver_set_width (r, r->w_start - r->max_level + 1);

      /* compute energy & minpath maps */
      lqr_carver_build_emap (r);
      lqr_carver_build_mmap (r);

      /* compute visibility map */
      CATCH (lqr_carver_build_vsmap (r, depth));
    }
  return LQR_OK;
}

/* compute energy map */
void
lqr_carver_build_emap (LqrCarver * r)
{
  gint x, y;

  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          lqr_carver_compute_e (r, x, y);
        }
    }
}

/* compute auxiliary minpath map
 * defined as
 *   y = 1 : m(x,y) = e(x,y)
 *   y > 1 : m(x,y) = min_{x'=-dx,..,dx} ( m(x-x',y-1) + rig(x') ) + e(x,y)
 * where
 *   e(x,y)  is the energy at point (x,y)
 *   dx      is the max seam step delta_x 
 *   rig(x') is the rigidity for step x'
 */
void
lqr_carver_build_mmap (LqrCarver * r)
{
  gint x, y;
  gint data;
  gint data_down;
  gint x1_min, x1_max, x1;
  gdouble m, m1, r_fact;


  /* span first row */
  for (x = 0; x < r->w; x++)
    {
      data = r->raw[0][x];
#ifdef __LQR_DEBUG__
      assert (r->vs[data] == 0);
#endif /* __LQR_DEBUG__ */
      r->m[data] = r->en[data];
    }

  /* span all other rows */
  for (y = 1; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          data = r->raw[y][x];
#ifdef __LQR_DEBUG__
          assert (r->vs[data] == 0);
#endif /* __LQR_DEBUG__ */
	  /* watch for boundaries */
          x1_min = MAX (-x, -r->delta_x);
          x1_max = MIN (r->w - 1 - x, r->delta_x);
	  if (r->rigidity_mask) {
		  r_fact = r->rigidity_mask[data];
	  } else {
		  r_fact = 1;
	  }

	  /* we use the data_down pointer to be able to
	   * track the seams later (needed for rigidity) */  
          data_down = r->raw[y - 1][x + x1_min];
          r->least[data] = data_down;
          if (r->rigidity)
            {
              m = r->m[data_down] + r_fact * r->rigidity_map[x1_min];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  /* find the min among the neighbors
                   * in the last row */
                  m1 = r->m[data_down] + r_fact * r->rigidity_map[x1];
		  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                  //if (m1 <= m)
                    {
                      m = m1;
                      r->least[data] = data_down;
                    }
                  /* m = MIN(m, r->m[data_down] + r->rigidity_map[x1]); */
                }
            }
          else
            {
              m = r->m[data_down];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  /* find the min among the neighbors
                   * in the last row */
                  m1 = r->m[data_down];
		  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                  //if (m1 <= m)
                    {
                      m = m1;
                      r->least[data] = data_down;
                    }
                  m = MIN (m, r->m[data_down]);
                }
            }

          /* set current m */
          r->m[data] = r->en[data] + m;
        }
    }
}

/* compute (vertical) visibility map up to given depth
 * (it also calls inflate() to add image enlargment information) */
LqrRetVal
lqr_carver_build_vsmap (LqrCarver * r, gint depth)
{
  gint l;
  gint update_step;
  gint lr_switch_interval = 0;
  LqrDataTok data_tok;

#ifdef __LQR_VERBOSE__
  printf ("[ building visibility map ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */


#ifdef __LQR_DEBUG__
  assert (depth <= r->w_start + 1);
  assert (depth >= 1);
#endif /* __LQR_DEBUG__ */

  /* default behaviour : compute all possible levels
   * (complete map) */
  if (depth == 0)
    {
      depth = r->w_start + 1;
    }

  /* here we assume that
   * lqr_carver_set_width(w_start - max_level + 1);
   * has been given */

  /* update step for progress reprt*/
  update_step = (gint) MAX ((depth - r->max_level) * r->progress->update_step, 1);

  /* left-right switch interval */
  if (r->lr_switch_frequency)
    {
      lr_switch_interval = (depth - r->max_level - 1) / r->lr_switch_frequency + 1;
    }

  /* cycle over levels */
  for (l = r->max_level; l < depth; l++)
    {

      if ((l - r->max_level) % update_step == 0)
        {
	  lqr_progress_update (r->progress, (gdouble) (l - r->max_level) /
                                (gdouble) (depth - r->max_level));
        }

      /* compute vertical seam */
      lqr_carver_build_vpath (r);

      /* update visibility map
       * (assign level to the seam) */
      lqr_carver_update_vsmap (r, l + r->max_level - 1);

      /* increase (in)visibility level
       * (make the last seam invisible) */
      r->level++;
      r->w--;

      /* update raw data */
      lqr_carver_carve (r);

      if (r->w > 1)
        {
          /* update the energy */
          //lqr_carver_build_emap (r);
          lqr_carver_update_emap (r);

          /* recalculate the minpath map */
	  if ((r->lr_switch_frequency) && (((l - r->max_level + lr_switch_interval / 2) % lr_switch_interval) == 0))
	    {
	      r->leftright ^= 1;
	      lqr_carver_build_mmap (r);
	    }
	  else
	    {
	      lqr_carver_update_mmap (r);
	    }
        }
      else
        {
          /* complete the map (last seam) */
          lqr_carver_finish_vsmap (r);
        }
    }

  /* insert seams for image enlargement */
  CATCH (lqr_carver_inflate (r, depth - 1));

  /* reset image size */
  lqr_carver_set_width (r, r->w_start);
  /* repeat for auxiliary layers */
  data_tok.integer = r->w_start;
  CATCH (lqr_carver_list_foreach (r->attached_list, lqr_carver_set_width_attached, data_tok));

#ifdef __LQR_VERBOSE__
  printf ("[ visibility map OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  return LQR_OK;
}

/* enlarge the image by seam insertion
 * visibility map is updated and the resulting multisize image
 * is complete in both directions */
LqrRetVal
lqr_carver_inflate (LqrCarver * r, gint l)
{
  gint w1, z0, vs, k;
  gint x, y;
  gint c_left;
  void *new_rgb = NULL;
  gint *new_vs = NULL;
  guint tmp_rgb;
  gdouble *new_bias = NULL;
  gdouble *new_rigmask = NULL;
  LqrDataTok data_tok;

#ifdef __LQR_VERBOSE__
  printf ("  [ inflating (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

#ifdef __LQR_DEBUG__
  assert (l + 1 > r->max_level);        /* otherwise is useless */
#endif /* __LQR_DEBUG__ */

  /* first iterate on attached carvers */
  data_tok.integer = l;
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_inflate_attached, data_tok));

  /* scale to current maximum size
   * (this is the original size the first time) */
  lqr_carver_set_width (r, r->w0);

  /* final width */
  w1 = r->w0 + l - r->max_level + 1;

  /* allocate room for new maps */
  switch (r->bits) {
    case 8: CATCH_MEM (new_rgb = g_try_new0 (guchar, w1 * r->h0 * r->channels));
       break;
    case 16: CATCH_MEM (new_rgb = g_try_new0 (guint16, w1 * r->h0 * r->channels));
       break;
  }
  if (r->root == NULL)
    {
      CATCH_MEM (new_vs = g_try_new0 (gint, w1 * r->h0));
    }
  if (r->active)
    {
      CATCH_MEM (new_bias = g_try_new0 (gdouble, w1 * r->h0));
      if (r->rigidity_mask)
	{
	  CATCH_MEM (new_rigmask = g_try_new (gdouble, w1 * r->h0));
	}
    }

  /* span the image with a cursor
   * and build the new image */
  lqr_cursor_reset (r->c);
  x = 0;
  y = 0;
  for (z0 = 0; z0 < w1 * r->h0; z0++, lqr_cursor_next (r->c))
    {
      /* read visibility */
      vs = r->vs[r->c->now];
      if ((vs != 0) && (vs <= l + r->max_level - 1)
          && (vs >= 2 * r->max_level - 1))
        {
          /* the point belongs to a previously computed seam
           * and was not inserted during a previous
           * inflate() call : insert another seam */

          /* the new pixel value is equal to the average of its
           * left and right neighbors */

	  if (r->c->x > 0)
            {
              c_left = lqr_cursor_left (r->c);
            }
          else
            {
              c_left = r->c->now;
            }

	  for (k = 0; k < r->channels; k++)
	    {
	      tmp_rgb = (R_RGB(r->rgb, c_left * r->channels + k) +
			 R_RGB(r->rgb, r->c->now * r->channels + k)) / 2;
	      switch (r->bits)
		{
		  case 8:
		    ((guchar*)new_rgb)[z0 * r->channels + k] = (guchar)tmp_rgb;
		    break;
		  case 16:
		    ((guint16*)new_rgb)[z0 * r->channels + k] = (guint16)tmp_rgb;
		    break;
		}
	    }
          if (r->active)
            {
              new_bias[z0] = (r->bias[c_left] + r->bias[r->c->now]) / 2;
	      if (r->rigidity_mask)
	        {
		  new_rigmask[z0] = (r->rigidity_mask[c_left] + r->rigidity_mask[r->c->now]) / 2;
		}
            }
          /* the first time inflate() is called
           * the new visibility should be -vs + 1 but we shift it
           * so that the final minimum visibiliy will be 1 again
           * and so that vs=0 still means "uninitialized".
           * Subsequent inflations account for that */
          if (r->root == NULL)
            {
              new_vs[z0] = l - vs + r->max_level;
            }
          z0++;
        }
      for (k = 0; k < r->channels; k++)
        {
	  tmp_rgb = R_RGB(r->rgb, r->c->now * r->channels + k);
	  switch (r->bits)
	    {
	      case 8:
		((guchar*)new_rgb)[z0 * r->channels + k] = (guchar) tmp_rgb;
		break;
	      case 16:
		((guint16*)new_rgb)[z0 * r->channels + k] = (guint16) tmp_rgb;
		break;
	    }
        }
      if (r->active)
        {
          new_bias[z0] = r->bias[r->c->now];
	  if (r->rigidity_mask)
	    {
	      new_rigmask[z0] = r->rigidity_mask[r->c->now];
	    }
        }
      if (vs != 0)
        {
          /* visibility has to be shifted up */
          if (r->root == NULL)
            {
              new_vs[z0] = vs + l - r->max_level + 1;
            }
        }
      else if (r->raw != NULL)
        {
#ifdef __LQR_DEBUG__
          assert (y < r->h_start);
          assert (x < r->w_start - l);
#endif /* __LQR_DEBUG__ */
          r->raw[y][x] = z0;
          x++;
          if (x >= r->w_start - l)
            {
              x = 0;
              y++;
            }
        }
    }

#ifdef __LQR_DEBUG__
  if (r->raw != NULL)
    {
      assert (x == 0);
      if (w1 != 2 * r->w_start - 1)
        {
          assert ((y == r->h_start)
                  || (printf ("y=%i hst=%i w1=%i\n", y, r->h_start, w1)
                      && fflush (stdout) && 0));
        }
    }
#endif /* __LQR_DEBUG__ */

  /* substitute maps */
  g_free (r->rgb);
  //g_free (r->vs);
  g_free (r->en);
  g_free (r->m);
  g_free (r->least);
  g_free (r->bias);
  g_free (r->rigidity_mask);

  r->rgb = new_rgb;
  if (r->root == NULL)
    {
      g_free (r->vs);
      r->vs = new_vs;
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  else
    {
      //r->vs = NULL;
    }
  if (r->active)
    {
      r->bias = new_bias;
      r->rigidity_mask = new_rigmask;
      CATCH_MEM (r->en = g_try_new0 (gdouble, w1 * r->h0));
      CATCH_MEM (r->m = g_try_new0 (gdouble, w1 * r->h0));
      CATCH_MEM (r->least = g_try_new0 (gint, w1 * r->h0));
    }

  /* set new widths & levels (w_start is kept for reference) */
  r->level = l + 1;
  r->max_level = l + 1;
  r->w0 = w1;
  r->w = r->w_start;

  /* reset readout buffer */
  g_free (r->rgb_ro_buffer);
  switch (r->bits)
    {
      case 8: CATCH_MEM (r->rgb_ro_buffer = g_try_new (guchar, r->w0 * r->channels));
	 break;
      case 16: CATCH_MEM (r->rgb_ro_buffer = g_try_new (guint16, r->w0 * r->channels));
	 break;
    }
  
#ifdef __LQR_VERBOSE__
  printf ("  [ inflating OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  return LQR_OK;
}

LqrRetVal
lqr_carver_inflate_attached (LqrCarver * r, LqrDataTok data)
{
  return lqr_carver_inflate (r, data.integer);
}


/*** internal functions for maps computations ***/

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

/* do the carving
 * this actually carves the raw array,
 * which holds the indices to be used
 * in all the other maps */
void
lqr_carver_carve (LqrCarver * r)
{
  gint x, y;

#ifdef __LQR_DEBUG__
  assert (r->root == NULL);
#endif /* __LQR_DEBUG__ */

  for (y = 0; y < r->h_start; y++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->raw[y][r->vpath_x[y]]] != 0);
      for (x = 0; x < r->vpath_x[y]; x++)
        {
          assert (r->vs[r->raw[y][x]] == 0);
        }
#endif /* __LQR_DEBUG__ */
      for (x = r->vpath_x[y]; x < r->w; x++)
        {
          r->raw[y][x] = r->raw[y][x + 1];
#ifdef __LQR_DEBUG__
          assert (r->vs[r->raw[y][x]] == 0);
#endif /* __LQR_DEBUG__ */
        }
    }
}


/* update energy map after seam removal
 * (the only affected energies are to the
 * left and right of the removed seam) */
void
lqr_carver_update_emap (LqrCarver * r)
{
  gint x, y;
  gint x1, x_min, x_max;

  for (y = 0; y < r->h; y++)
    {
      x = r->vpath_x[y];
      x_min = MAX (0, x - r->delta_x);
      x_max = MIN (r->w - 1, x + r->delta_x - 1);
      for (x1 = x_min; x1 <= x_max; x1++)
        {
          lqr_carver_compute_e (r, x1, y);
        }
    }
}


/* update the auxiliary minpath map
 * this only updates the affected pixels,
 * which start form the beginning of the seam
 * and expand at most by delta_x (in both
 * directions) at each row */
void
lqr_carver_update_mmap (LqrCarver * r)
{
  gint x, y;
  gint x_min, x_max;
  gint x1;
  gint x1_min, x1_max;
  gint data, data_down, least;
  gdouble m, m1, r_fact;
  gint stop;

  /* span first row */
  x_min = MAX (r->vpath_x[0] - r->delta_x, 0);
  //x_max = MIN (r->vpath_x[0] + r->delta_x - 1, r->w - 1);
  x_max = MIN (r->vpath_x[0] + r->delta_x, r->w - 1);

  for (x = x_min; x <= x_max; x++)
    {
      r->m[r->raw[0][x]] = r->en[r->raw[0][x]];
    }

  /* other rows */
  for (y = 1; y < r->h; y++)
    {
      /* make sure to include the seam */
      x_min = MIN (x_min, r->vpath_x[y]);
      x_max = MAX (x_max, r->vpath_x[y]);
      //x_max = MAX (x_max, r->vpath_x[y] - 1);

      /* expand the affected region by delta_x */
      x_min = MAX (x_min - r->delta_x, 0);
      x_max = MIN (x_max + r->delta_x, r->w - 1);

      /* span the affected region */
      stop = 0;
      for (x = x_min; x <= x_max; x++)
        {
          data = r->raw[y][x];
	  if (r->rigidity_mask) {
		  r_fact = r->rigidity_mask[data];
	  } else {
		  r_fact = 1;
	  }

	  /* find the minimum in the previous rows
	   * as in build_mmap() */
          x1_min = MAX (-x, -r->delta_x);
          x1_max = MIN (r->w - 1 - x, r->delta_x);
          data_down = r->raw[y - 1][x + x1_min];
          least = data_down;
          if (r->rigidity)
            {
              m = r->m[data_down] + r_fact * r->rigidity_map[x1_min];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  m1 = r->m[data_down] + r_fact * r->rigidity_map[x1];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                  //if (m1 <= m)
                    {
                      m = m1;
                      least = data_down;
                    }
                }
            }
          else
            {
              m = r->m[data_down];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  m1 = r->m[data_down];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                  //if (m1 <= m)
                    {
		      //if (m1 == m) {
			//printf("m=%g least=%i data_down=%i\n", m, least, data_down); fflush(stdout);
		      //}
                      m = m1;
                      least = data_down;
                    }
                }
            }

//#if 0
	  /* reduce the range if there's no difference
	   * with the previous map */
          if (r->least[data] == least)
            {
              if ((x == x_min) && (x < r->vpath_x[y])
                  && (r->m[data] == r->en[data] + m))
                {
                  x_min++;
                }
              if ((x >= r->vpath_x[y]) && (r->m[data] == r->en[data] + m))
                {
                  stop = 1;
                  x_max = x;
                }
            }
//#endif


          /* set current m */
          r->m[data] = r->en[data] + m;
          r->least[data] = least;

          if (stop)
            {
              break;
            }
        }

    }
}


/* compute seam path from minpath map */
void
lqr_carver_build_vpath (LqrCarver * r)
{
  gint x, y, z0;
  gdouble m, m1;
  gint last = -1;
  gint last_x = 0;
  gint x_min, x_max;

  /* we start at last row */
  y = r->h - 1;

  /* span the last row for the minimum mmap value */
  m = (1 << 29);
  for (x = 0, z0 = y * r->w_start; x < r->w; x++, z0++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->raw[y][x]] == 0);
#endif /* __LQR_DEBUG__ */

      m1 = r->m[r->raw[y][x]];
      if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
      //if (m1 <= m)
        {
          last = r->raw[y][x];
          last_x = x;
          m = m1;
        }
    }

#ifdef __LQR_DEBUG__
  assert (last >= 0);
#endif /* __LQR_DEBUG__ */

  /* follow the track for the other rows */
  for (y = r->h0 - 1; y >= 0; y--)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[last] == 0);
      assert (last_x < r->w);
#endif /* __LQR_DEBUG__ */
      r->vpath[y] = last;
      r->vpath_x[y] = last_x;
      if (y > 0)
        {
          last = r->least[r->raw[y][last_x]];
	  /* we also need to retrieve the x coordinate */
          x_min = MAX (last_x - r->delta_x, 0);
          x_max = MIN (last_x + r->delta_x, r->w - 1);
          for (x = x_min; x <= x_max; x++)
            {
              if (r->raw[y - 1][x] == last)
                {
                  last_x = x;
                  break;
                }
            }
#ifdef __LQR_DEBUG__
          assert (x < x_max + 1);
#endif /* __LQR_DEBUG__ */
        }
    }


#if 0
  /* we backtrack the seam following the min mmap */
  for (y = r->h0 - 1; y >= 0; y--)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[last] == 0);
      assert (last_x < r->w);
#endif /* __LQR_DEBUG__ */

      r->vpath[y] = last;
      r->vpath_x[y] = last_x;
      if (y > 0)
        {
          m = (1 << 29);
          x_min = MAX (0, last_x - r->delta_x);
          x_max = MIN (r->w - 1, last_x + r->delta_x);
          for (x = x_min; x <= x_max; x++)
            {
              m1 = r->m[r->raw[y - 1][x]];
              if (m1 < m)
                {
                  last = r->raw[y - 1][x];
                  last_x = x;
                  m = m1;
                }
            }
        }
    }
#endif
}

/* update visibility map after seam computation */
void
lqr_carver_update_vsmap (LqrCarver * r, gint l)
{
  gint y;
#ifdef __LQR_DEBUG__
  assert(r->root == NULL);
#endif /* __LQR_DEBUG__ */
  for (y = 0; y < r->h; y++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->vpath[y]] == 0);
      assert (r->vpath[y] == r->raw[y][r->vpath_x[y]]);
#endif /* __LQR_DEBUG__ */
      r->vs[r->vpath[y]] = l;
    }
}

/* complete visibility map (last seam) */
/* set the last column of pixels to vis. level w0 */
void
lqr_carver_finish_vsmap (LqrCarver * r)
{
  gint y;

#ifdef __LQR_DEBUG__
  assert (r->w == 1);
  assert (r->root == NULL);
#endif /* __LQR_DEBUG__ */
  lqr_cursor_reset (r->c);
  for (y = 1; y <= r->h; y++, lqr_cursor_next (r->c))
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->c->now] == 0);
#endif /* __LQR_DEBUG__ */
      r->vs[r->c->now] = r->w0;
    }
  lqr_cursor_reset (r->c);
}

/* propagate the root carver's visibility map */
LqrRetVal
lqr_carver_propagate_vsmap (LqrCarver * r)
{
  LqrDataTok data_tok;
  data_tok.data = NULL;
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_propagate_vsmap_attached, data_tok));
  return LQR_OK;
}

LqrRetVal
lqr_carver_propagate_vsmap_attached (LqrCarver * r, LqrDataTok data)
{
  LqrDataTok data_tok;
  data_tok.data = NULL;
  r->vs = r->root->vs;
  lqr_carver_scan_reset(r);
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_propagate_vsmap_attached, data_tok));
  return LQR_OK;
}

/*** image manipulations ***/

/* set width of the multisize image
 * (maps have to be computed already) */
void
lqr_carver_set_width (LqrCarver * r, gint w1)
{
#ifdef __LQR_DEBUG__
  assert (w1 <= r->w0);
  assert (w1 >= r->w_start - r->max_level + 1);
#endif /* __LQR_DEBUG__ */
  r->w = w1;
  r->level = r->w0 - w1 + 1;
}

LqrRetVal
lqr_carver_set_width_attached (LqrCarver * r, LqrDataTok data)
{
  lqr_carver_set_width (r, data.integer);
  return LQR_OK;
}



/* flatten the image to its current state
 * (all maps are reset, invisible points are lost) */
LQR_PUBLIC
LqrRetVal
lqr_carver_flatten (LqrCarver * r)
{
  void *new_rgb = NULL;
  guint tmp_rgb;
  gdouble *new_bias = NULL;
  gdouble *new_rigmask = NULL;
  gint x, y, k;
  gint z0;
  LqrDataTok data_tok;

#ifdef __LQR_VERBOSE__
  printf ("    [ flattening (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  /* first iterate on attached carvers */
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_flatten_attached, data_tok));

  /* free non needed maps first */
  g_free (r->en);
  g_free (r->m);
  g_free (r->least);

  /* allocate room for new map */
  switch (r->bits)
    {
      case 8: CATCH_MEM (new_rgb = g_try_new0 (guchar, r->w * r->h * r->channels));
	 break;
      case 16: CATCH_MEM (new_rgb = g_try_new0 (guint16, r->w * r->h * r->channels));
	 break;
    }
  if (r->active)
    {
      CATCH_MEM (new_bias = g_try_new0 (gdouble, r->w * r->h));
      if (r->rigidity_mask) {
	      CATCH_MEM (new_rigmask = g_try_new (gdouble, r->w * r->h));
      }

      g_free (r->_raw);
      g_free (r->raw);
      CATCH_MEM (r->_raw = g_try_new (gint, r->w * r->h));
      CATCH_MEM (r->raw = g_try_new (gint *, r->h));
    }

  /* span the image with the cursor and copy
   * it in the new array  */
  lqr_cursor_reset (r->c);
  for (y = 0; y < r->h; y++)
    {
      if (r->active)
        {
          r->raw[y] = r->_raw + y * r->w;
        }
      for (x = 0; x < r->w; x++)
        {
          z0 = y * r->w + x;
          for (k = 0; k < r->channels; k++)
            {
              tmp_rgb = R_RGB(r->rgb, r->c->now * r->channels + k);
	      switch (r->bits)
	        {
		  case 8:
		    ((guchar*)new_rgb)[z0 * r->channels + k] = (guchar) tmp_rgb;
		    break;
		  case 16:
		    ((guint16*)new_rgb)[z0 * r->channels + k] = (guint16) tmp_rgb;
		    break;
		}
            }
          if (r->active)
            {
              new_bias[z0] = r->bias[r->c->now];
	      if (r->rigidity_mask) {
		      new_rigmask[z0] = r->rigidity_mask[r->c->now];
	      }
              r->raw[y][x] = z0;
            }
          lqr_cursor_next (r->c);
        }
    }

  /* substitute the old maps */
  g_free (r->rgb);
  r->rgb = new_rgb;
  if (r->active)
    {
      g_free (r->bias);
      r->bias = new_bias;
      if (r->rigidity_mask) {
	      g_free (r->rigidity_mask);
	      r->rigidity_mask = new_rigmask;
      }
    }

  /* init the other maps */
  if (r->root == NULL)
    { 
      g_free (r->vs);
      CATCH_MEM (r->vs = g_try_new0 (gint, r->w * r->h));
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  if (r->active)
    {
      CATCH_MEM (r->en = g_try_new0 (gdouble, r->w * r->h));
      CATCH_MEM (r->m = g_try_new0 (gdouble, r->w * r->h));
      CATCH_MEM (r->least = g_try_new (gint, r->w * r->h));
    }

  /* reset widths, heights & levels */
  r->w0 = r->w;
  r->h0 = r->h;
  r->w_start = r->w;
  r->h_start = r->h;
  r->level = 1;
  r->max_level = 1;


#ifdef __LQR_VERBOSE__
  printf ("    [ flattening OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  return LQR_OK;
}

LqrRetVal
lqr_carver_flatten_attached(LqrCarver *r, LqrDataTok data)
{
  return lqr_carver_flatten(r);
}

/* transpose the image, in its current state
 * (all maps and invisible points are lost) */
LqrRetVal
lqr_carver_transpose (LqrCarver * r)
{
  gint x, y, k;
  gint z0, z1;
  gint d;
  void *new_rgb = NULL;
  guint tmp_rgb;
  gdouble *new_bias = NULL;
  gdouble *new_rigmask = NULL;
  LqrDataTok data_tok;

#ifdef __LQR_VERBOSE__
  printf ("[ transposing (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */


  if (r->level > 1)
    {
      CATCH (lqr_carver_flatten (r));
    }

  /* first iterate on attached carvers */
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_transpose_attached, data_tok));

  /* free non needed maps first */
  if (r->root == NULL)
    {
      g_free (r->vs);
    }
  g_free (r->en);
  g_free (r->m);
  g_free (r->least);
  g_free (r->rgb_ro_buffer);

  /* allocate room for the new maps */
  switch (r->bits)
    {
      case 8: CATCH_MEM (new_rgb = g_try_new0 (guchar, r->w0 * r->h0 * r->channels));
	 break;
      case 16: CATCH_MEM (new_rgb = g_try_new0 (guint16, r->w0 * r->h0 * r->channels));
	 break;
    }

  if (r->active)
    {
      CATCH_MEM (new_bias = g_try_new0 (gdouble, r->w0 * r->h0));
      if (r->rigidity_mask)
        {
	  CATCH_MEM (new_rigmask = g_try_new (gdouble, r->w0 * r->h0));
	}
      g_free (r->_raw);
      g_free (r->raw);
      CATCH_MEM (r->_raw = g_try_new0 (gint, r->h0 * r->w0));
      CATCH_MEM (r->raw = g_try_new0 (gint *, r->w0));
    }

  /* compute trasposed maps */
  for (x = 0; x < r->w; x++)
    {
      if (r->active)
        {
          r->raw[x] = r->_raw + x * r->h0;
        }
      for (y = 0; y < r->h; y++)
        {
          z0 = y * r->w0 + x;
          z1 = x * r->h0 + y;
          for (k = 0; k < r->channels; k++)
            {
              tmp_rgb = R_RGB(r->rgb, z0 * r->channels + k);
	      switch (r->bits)
	        {
		  case 8:
		    ((guchar*)new_rgb)[z1 * r->channels + k] = (guchar) tmp_rgb;
		    break;
		  case 16:
		    ((guint16*)new_rgb)[z1 * r->channels + k] = (guint16) tmp_rgb;
		    break;
		}
            }
          if (r->active)
            {
              new_bias[z1] = r->bias[z0];
	      if (r->rigidity_mask) {
		      new_rigmask[z1] = r->rigidity_mask[z0];
	      }
              r->raw[x][y] = z1;
            }
        }
    }

  /* substitute the map */
  g_free (r->rgb);
  r->rgb = new_rgb;
  if (r->active)
    {
      g_free (r->bias);
      r->bias = new_bias;
      if (r->rigidity_mask) {
	      g_free (r->rigidity_mask);
	      r->rigidity_mask = new_rigmask;
      }
    }

  /* init the other maps */
  if (r->root == NULL)
    {
      CATCH_MEM (r->vs = g_try_new0 (gint, r->w0 * r->h0));
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  if (r->active)
    {
      CATCH_MEM (r->en = g_try_new0 (gdouble, r->w0 * r->h0));
      CATCH_MEM (r->m = g_try_new0 (gdouble, r->w0 * r->h0));
      CATCH_MEM (r->least = g_try_new (gint, r->w0 * r->h0));
    }

  /* switch widths & heights */
  d = r->w0;
  r->w0 = r->h0;
  r->h0 = d;
  r->w = r->w0;
  r->h = r->h0;

  /* reset w_start, h_start & levels */
  r->w_start = r->w0;
  r->h_start = r->h0;
  r->level = 1;
  r->max_level = 1;

  /* reset seam path, cursor and readout buffer */
  if (r->active)
    {
      g_free (r->vpath);
      CATCH_MEM (r->vpath = g_try_new (gint, r->h));
      g_free (r->vpath_x);
      CATCH_MEM (r->vpath_x = g_try_new (gint, r->h));
    }

  switch (r->bits)
    {
      case 8: CATCH_MEM (r->rgb_ro_buffer = g_try_new (guchar, r->w0 * r->channels));
	 break;
      case 16: CATCH_MEM (r->rgb_ro_buffer = g_try_new (guint16, r->w0 * r->channels));
	 break;
    }

  /* rescale rigidity */

  if (r->active)
    {
      for (x = -r->delta_x; x <= r->delta_x; x++)
        {
          r->rigidity_map[x] = (gdouble) r->rigidity_map[x] * r->w0 / r->h0;
        }
    }

  /* set transposed flag */
  r->transposed = (r->transposed ? 0 : 1);

#ifdef __LQR_VERBOSE__
  printf ("[ transpose OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  return LQR_OK;
}

LqrRetVal
lqr_carver_transpose_attached (LqrCarver * r, LqrDataTok data)
{
  return lqr_carver_transpose(r);
}

/* resize w + h: these are the liquid rescale methods.
 * They automatically determine the depth of the map
 * according to the desired size, can be called multiple
 * times, transpose the image as necessasry */
LqrRetVal
lqr_carver_resize_width (LqrCarver * r, gint w1)
{
  LqrDataTok data_tok;
  gint delta, gamma;
  /* delta is used to determine the required depth
   * gamma to decide if action is necessary */
  if (!r->transposed)
    {
      delta = w1 - r->w_start;
      gamma = w1 - r->w;
    }
  else
    {
      delta = w1 - r->h_start;
      gamma = w1 - r->h;
    }
  delta = delta > 0 ? delta : -delta;

  if (gamma)
    {
      if (r->transposed)
        {
          CATCH (lqr_carver_transpose (r));
        }
      lqr_progress_init (r->progress, r->progress->init_width_message);
      CATCH (lqr_carver_build_maps (r, delta + 1));
      lqr_carver_set_width (r, w1);

      data_tok.integer = w1;
      lqr_carver_list_foreach (r->attached_list,  lqr_carver_set_width_attached, data_tok);

      if (r->dump_vmaps)
        {
          CATCH (lqr_vmap_internal_dump (r));
        }
      lqr_progress_end (r->progress, r->progress->end_width_message);
    }
  return LQR_OK;
}

LqrRetVal
lqr_carver_resize_height (LqrCarver * r, gint h1)
{
  LqrDataTok data_tok;
  gint delta, gamma;
  if (!r->transposed)
    {
      delta = h1 - r->h_start;
      gamma = h1 - r->h;
    }
  else
    {
      delta = h1 - r->w_start;
      gamma = h1 - r->w;
    }
  delta = delta > 0 ? delta : -delta;
  if (gamma)
    {
      if (!r->transposed)
        {
          CATCH (lqr_carver_transpose (r));
        }
      lqr_progress_init (r->progress, r->progress->init_height_message);
      CATCH (lqr_carver_build_maps (r, delta + 1));
      lqr_carver_set_width (r, h1);

      data_tok.integer = h1;
      lqr_carver_list_foreach (r->attached_list,  lqr_carver_set_width_attached, data_tok);
      
      if (r->dump_vmaps)
        {
          CATCH (lqr_vmap_internal_dump (r));
        }
      lqr_progress_end (r->progress, r->progress->end_height_message);
    }

  return LQR_OK;
}

/* liquid rescale public method */
LQR_PUBLIC
LqrRetVal
lqr_carver_resize (LqrCarver * r, gint w1, gint h1)
{
#ifdef __LQR_VERBOSE__
  printf("[ Rescale from %i,%i to %i,%i ]\n", (r->transposed ? r->h : r->w), (r->transposed ? r->w : r->h), w1, h1);
  fflush(stdout);
#endif /* __LQR_VERBOSE__ */
  switch (r->resize_order)
    {
      case LQR_RES_ORDER_HOR:
	CATCH (lqr_carver_resize_width(r, w1));
	CATCH (lqr_carver_resize_height(r, h1));
	break;
      case LQR_RES_ORDER_VERT:
	CATCH (lqr_carver_resize_height(r, h1));
	CATCH (lqr_carver_resize_width(r, w1));
	break;
#ifdef __LQR_DEBUG__
      default:
	assert(0);
#endif /* __LQR_DEBUG__ */
    }
  lqr_carver_scan_reset_all(r);

#ifdef __LQR_VERBOSE__
  printf("[ Rescale OK ]\n");
  fflush(stdout);
#endif /* __LQR_VERBOSE__ */
  return TRUE;
}

/* get size */
LQR_PUBLIC
gint
lqr_carver_get_width(LqrCarver* r)
{
  return (r->transposed ? r->h : r->w);
}

LQR_PUBLIC
gint
lqr_carver_get_height(LqrCarver* r)
{
  return (r->transposed ? r->w : r->h);
}

/* get colour channels */
LQR_PUBLIC
gint
lqr_carver_get_channels (LqrCarver * r)
{
  return r->channels;
}

LQR_PUBLIC
gint
lqr_carver_get_bpp (LqrCarver * r)
{
  return lqr_carver_get_channels(r);
}


/* readout reset */
LQR_PUBLIC
void
lqr_carver_scan_reset (LqrCarver * r)
{
  lqr_cursor_reset (r->c);
}

LqrRetVal
lqr_carver_scan_reset_attached(LqrCarver * r, LqrDataTok data)
{
  lqr_carver_scan_reset(r);
  return lqr_carver_list_foreach(r->attached_list, lqr_carver_scan_reset_attached, data);
}

void
lqr_carver_scan_reset_all (LqrCarver *r)
{
  LqrDataTok data;
  data.data = NULL;
  lqr_carver_scan_reset(r);
  lqr_carver_list_foreach(r->attached_list, lqr_carver_scan_reset_attached, data);
}



/* readout all, pixel by bixel */
LQR_PUBLIC
gboolean
lqr_carver_scan (LqrCarver * r, gint * x, gint * y, guchar ** rgb)
{
  gint k;
  if (r->bits != 8)
    {
      return FALSE;
    }
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  (*x) = (r->transposed ? r->c->y : r->c->x);
  (*y) = (r->transposed ? r->c->x : r->c->y);
  for (k = 0; k < r->channels; k++)
    {
      ((guchar*)r->rgb_ro_buffer)[k] = R_RGB(r->rgb, r->c->now * r->channels + k);
    }
  (*rgb) = (guchar*)r->rgb_ro_buffer;
  lqr_cursor_next(r->c);
  return TRUE;
}

LQR_PUBLIC
gboolean
lqr_carver_scan_16 (LqrCarver * r, gint * x, gint * y, guint16 ** rgb)
{
  gint k;
  if (r->bits != 16)
    {
      return FALSE;
    }
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  (*x) = (r->transposed ? r->c->y : r->c->x);
  (*y) = (r->transposed ? r->c->x : r->c->y);
  for (k = 0; k < r->channels; k++)
    {
      ((guint16*)r->rgb_ro_buffer)[k] = R_RGB(r->rgb, r->c->now * r->channels + k);
    }
  (*rgb) = (guint16*)r->rgb_ro_buffer;
  lqr_cursor_next(r->c);
  return TRUE;
}

/* readout all, by line */
LQR_PUBLIC
gboolean
lqr_carver_scan_by_row (LqrCarver *r)
{
  return r->transposed ? FALSE : TRUE;
}

LQR_PUBLIC
gboolean
lqr_carver_scan_line (LqrCarver * r, gint * n, guchar ** rgb)
{
  gint k, x;
  if (r->bits != 8)
    {
      return FALSE;
    }
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  x = r->c->x;
  (*n) = r->c->y;
  while (x > 0)
    {
      lqr_cursor_prev(r->c);
      x = r->c->x;
    }
  for (x = 0; x < r->w; x++)
    {
      for (k = 0; k < r->channels; k++)
	{
	  ((guchar*)r->rgb_ro_buffer)[x * r->channels + k] = R_RGB(r->rgb, r->c->now * r->channels + k);
	}
      lqr_cursor_next(r->c);
    }

  (*rgb) = (guchar*)r->rgb_ro_buffer;
  return TRUE;
}

LQR_PUBLIC
gboolean
lqr_carver_scan_line_16 (LqrCarver * r, gint * n, guint16 ** rgb)
{
  gint k, x;
  if (r->bits != 16)
    {
      return FALSE;
    }
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  x = r->c->x;
  (*n) = r->c->y;
  while (x > 0)
    {
      lqr_cursor_prev(r->c);
      x = r->c->x;
    }
  for (x = 0; x < r->w; x++)
    {
      for (k = 0; k < r->channels; k++)
	{
	  ((guint16*)r->rgb_ro_buffer)[x * r->channels + k] = R_RGB(r->rgb, r->c->now * r->channels + k);
	}
      lqr_cursor_next(r->c);
    }

  (*rgb) = (guint16*)r->rgb_ro_buffer;
  return TRUE;
}

/**** END OF LQR_CARVER CLASS FUNCTIONS ****/
