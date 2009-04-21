/* LiquidRescaling Library
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
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
#include <math.h>
#include <lqr/lqr_base.h>
#include <lqr/lqr_gradient.h>
#include <lqr/lqr_rwindow.h>
#include <lqr/lqr_energy.h>
#include <lqr/lqr_progress_pub.h>
#include <lqr/lqr_cursor_pub.h>
#include <lqr/lqr_vmap.h>
#include <lqr/lqr_vmap_list.h>
#include <lqr/lqr_carver_list.h>
#include <lqr/lqr_carver.h>

#ifdef __LQR_DEBUG__
#include <stdio.h>
#include <assert.h>
#endif /* __LQR_DEBUG__ */


/* read normalised pixel value from
 * rgb buffer at the given index */
inline gfloat
lqr_pixel_get_norm (void * rgb, gint rgb_ind, LqrColDepth col_depth) 
{
  switch (col_depth)
    {
      case LQR_COLDEPTH_8I:
        return (gfloat) AS_8I(rgb)[rgb_ind] / 0xFF;
      case LQR_COLDEPTH_16I:
        return (gfloat) AS_16I(rgb)[rgb_ind] / 0xFFFF;
      case LQR_COLDEPTH_32F:
        return (gfloat) AS_32F(rgb)[rgb_ind];
      case LQR_COLDEPTH_64F:
        return (gfloat) AS_64F(rgb)[rgb_ind];
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return 0;
    }
}

inline gfloat
lqr_pixel_get_rgbcol (void *rgb, gint rgb_ind, LqrColDepth col_depth, LqrImageType image_type, gint channel)
{
  gfloat black_fact = 0;

  switch (image_type)
    {
      case LQR_RGB_IMAGE: 
      case LQR_RGBA_IMAGE: 
        return lqr_pixel_get_norm (rgb, rgb_ind + channel, col_depth);
      case LQR_CMY_IMAGE: 
        return 1. - lqr_pixel_get_norm (rgb, rgb_ind + channel, col_depth);
      case LQR_CMYK_IMAGE: 
      case LQR_CMYKA_IMAGE: 
        black_fact = 1 - lqr_pixel_get_norm(rgb, rgb_ind + 3, col_depth);
        return black_fact * (1. - (lqr_pixel_get_norm (rgb, rgb_ind + channel, col_depth)));
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return 0;
    }
}

inline gfloat
lqr_carver_read_brightness_grey (LqrCarver * r, gint x, gint y)
{
  gint now = r->raw[y][x];
  gint rgb_ind = now * r->channels;
  return lqr_pixel_get_norm (r->rgb, rgb_ind, r->col_depth);
}

inline gfloat
lqr_carver_read_brightness_std (LqrCarver * r, gint x, gint y)
{
  gfloat red, green, blue;
  gint now = r->raw[y][x];
  gint rgb_ind = now * r->channels;

  red = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 0);
  green = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 1);
  blue = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 2);
  return (red + green + blue) / 3;
}

gfloat
lqr_carver_read_brightness_custom (LqrCarver * r, gint x, gint y)
{
  gfloat sum = 0;
  gint k;
  gchar has_alpha = (r->alpha_channel >= 0 ? 1 : 0);
  gchar has_black = (r->black_channel >= 0 ? 1 : 0);
  guint col_channels = r->channels - has_alpha - has_black;

  gfloat black_fact = 0;

  gint now = r->raw[y][x];

  if (has_black)
    {
      black_fact = lqr_pixel_get_norm(r->rgb, now * r->channels + r->black_channel, r->col_depth);
    }

  for (k = 0; k < r->channels; k++) if ((k != r->alpha_channel) && (k != r->black_channel))
    {
      gfloat col = lqr_pixel_get_norm(r->rgb, now * r->channels + k, r->col_depth);
      sum += 1. - (1. - col) * (1. - black_fact);
    }

  sum /= col_channels;

  if (has_black)
    {
      sum = 1 - sum;
    }

  return sum;
}

/* read average pixel value at x, y 
 * for energy computation */
gfloat
lqr_carver_read_brightness (LqrCarver * r, gint x, gint y)
{
  gchar has_alpha = (r->alpha_channel >= 0 ? 1 : 0);
  gfloat alpha_fact = 1;

  gint now = r->raw[y][x];

  gfloat bright = 0;

  switch (r->image_type)
    {
      case LQR_GREY_IMAGE:
      case LQR_GREYA_IMAGE:
        bright = lqr_carver_read_brightness_grey (r, x, y);
        break;
      case LQR_RGB_IMAGE: 
      case LQR_RGBA_IMAGE: 
      case LQR_CMY_IMAGE: 
      case LQR_CMYK_IMAGE: 
      case LQR_CMYKA_IMAGE: 
        bright = lqr_carver_read_brightness_std (r, x, y);
        break;
      case LQR_CUSTOM_IMAGE:
        bright = lqr_carver_read_brightness_custom (r, x, y);
        break;
    }

  if (has_alpha)
    {
      alpha_fact = lqr_pixel_get_norm(r->rgb, now * r->channels + r->alpha_channel, r->col_depth);
    }

  return bright * alpha_fact;
}

inline gfloat
lqr_carver_read_luma_std (LqrCarver * r, gint x, gint y)
{
  gfloat red, green, blue;
  gint now = r->raw[y][x];
  gint rgb_ind = now * r->channels;

  red = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 0);
  green = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 1);
  blue = lqr_pixel_get_rgbcol (r->rgb, rgb_ind, r->col_depth, r->image_type, 2);
  return 0.2126 * red + 0.7152 * green + 0.0722 * blue;
}

gfloat
lqr_carver_read_luma (LqrCarver * r, gint x, gint y)
{
  gchar has_alpha = (r->alpha_channel >= 0 ? 1 : 0);
  gfloat alpha_fact = 1;

  gint now = r->raw[y][x];

  gfloat bright = 0;

  switch (r->image_type)
    {
      case LQR_GREY_IMAGE:
      case LQR_GREYA_IMAGE:
        bright = lqr_carver_read_brightness_grey (r, x, y);
        break;
      case LQR_RGB_IMAGE: 
      case LQR_RGBA_IMAGE: 
      case LQR_CMY_IMAGE: 
      case LQR_CMYK_IMAGE: 
      case LQR_CMYKA_IMAGE: 
        bright = lqr_carver_read_luma_std (r, x, y);
        break;
      case LQR_CUSTOM_IMAGE:
        bright = lqr_carver_read_brightness_custom (r, x, y);
        break;
    }

  if (has_alpha)
    {
      alpha_fact = lqr_pixel_get_norm(r->rgb, now * r->channels + r->alpha_channel, r->col_depth);
    }

  return bright * alpha_fact;
}

gfloat
lqr_carver_read_rgba (LqrCarver * r, gint x, gint y, gint channel)
{
  gchar has_alpha = (r->alpha_channel >= 0 ? 1 : 0);

  gint now = r->raw[y][x];

#ifdef __LQR_DEBUG__
  assert(channel >= 0 && channel < 4);
#endif /* __LQR_DEBUG__ */

  if (channel < 3)
    {
      switch (r->image_type)
        {
          case LQR_GREY_IMAGE:
          case LQR_GREYA_IMAGE:
            return lqr_carver_read_brightness_grey (r, x, y);
          case LQR_RGB_IMAGE: 
          case LQR_RGBA_IMAGE: 
          case LQR_CMY_IMAGE: 
          case LQR_CMYK_IMAGE: 
          case LQR_CMYKA_IMAGE: 
            return lqr_pixel_get_rgbcol (r->rgb, now * r->channels, r->col_depth, r->image_type, channel);
          case LQR_CUSTOM_IMAGE:
          default:
#ifdef __LQR_DEBUG__
            assert(0);
#endif /* __LQR_DEBUG__ */
            return 0;
        }
    }
  else if (has_alpha)
    {
      return lqr_pixel_get_norm(r->rgb, now * r->channels + r->alpha_channel, r->col_depth);
    }
  else
    {
      return 1;
    }
}




#if 0
/* read average pixel value at x, y 
 * for energy computation */
inline gfloat
lqr_carver_read_brightness (LqrCarver * r, gint x, gint y)
{
  gfloat sum = 0;
  gint k;
  gchar has_alpha = (r->alpha_channel >= 0 ? 1 : 0);
  gchar has_black = (r->black_channel >= 0 ? 1 : 0);
  guint col_channels = r->channels - has_alpha - has_black;

  gfloat alpha_fact = 1;
  gfloat black_fact = 0;

  gint now = r->raw[y][x];

  if (has_alpha)
    {
      alpha_fact = lqr_pixel_get_norm(r->rgb, now * r->channels + r->alpha_channel, r->col_depth);
    }

  if (has_black)
    {
      black_fact = lqr_pixel_get_norm(r->rgb, now * r->channels + r->black_channel, r->col_depth);
    }

  for (k = 0; k < r->channels; k++) if ((k != r->alpha_channel) && (k != r->black_channel))
    {
      gfloat col = lqr_pixel_get_norm(r->rgb, now * r->channels + k, r->col_depth);
      sum += 1. - (1. - col) * (1. - black_fact);
    }

  sum /= col_channels;

  switch (r->image_type)
    {
      case LQR_RGB_IMAGE:
      case LQR_RGBA_IMAGE:
      case LQR_GREY_IMAGE:
      case LQR_GREYA_IMAGE:
        break;
      case LQR_CMY_IMAGE:
      case LQR_CMYK_IMAGE:
      case LQR_CMYKA_IMAGE:
        sum = 1 - sum;
        break;
      case LQR_CUSTOM_IMAGE:
        if (has_black)
          {
            sum = 1 - sum;
          }
#ifdef __LQR_DEBUG__
      default:
        assert (0);
#endif /* __LQR_DEBUG__ */
    }

  if (has_alpha)
    {
      sum *= alpha_fact;
    }

  return sum;
}
#endif

#if 0
inline gfloat
lqr_carver_read_brightness_abs (LqrCarver * r, gint x1, gint y1, gint x2, gint y2)
{
  gchar has_alpha = (r->alpha_channel > 0 ? 1 : 0);
  gint p1, p2;
  gfloat a1, a2;
  gint k;
  gfloat sum = 0;
  p1 = r->raw[y1][x1];
  p2 = r->raw[y2][x2];
  if (has_alpha)
    {
      a1 = R_RGB(r->rgb, p1 * r->channels + r->alpha_channel) / R_RGB_MAX;
      a2 = R_RGB(r->rgb, p2 * r->channels + r->alpha_channel) / R_RGB_MAX;
    }
  else
    {
      a1 = a2 = 1;
    }
  for (k = 0; k < r->channels; k++) if (k != r->alpha_channel)
    {
      sum += fabs(R_RGB(r->rgb, p1 * r->channels + 0) * a1 - R_RGB(r->rgb, p2 * r->channels + 0) * a2);
    }
  return sum / (R_RGB_MAX * (r->channels - has_alpha));
}

inline gfloat
lqr_carver_read_luma_abs (LqrCarver * r, gint x1, gint y1, gint x2, gint y2)
{
  gint p1, p2;
  gfloat a1, a2;
  p1 = r->raw[y1][x1];
  p2 = r->raw[y2][x2];
  if (r->image_type == LQR_RGBA_IMAGE)
    {
      a1 = R_RGB(r->rgb, p1 * r->channels + 3) / R_RGB_MAX;
      a2 = R_RGB(r->rgb, p2 * r->channels + 3) / R_RGB_MAX;
    }
  else
    {
      a1 = a2 = 1;
    }
  return (0.2126 * fabs(R_RGB(r->rgb, p1 * r->channels + 0) * a1 - R_RGB(r->rgb, p2 * r->channels + 0) * a2) +
          0.7152 * fabs(R_RGB(r->rgb, p1 * r->channels + 1) * a1 - R_RGB(r->rgb, p2 * r->channels + 1) * a2) +
          0.0722 * fabs(R_RGB(r->rgb, p1 * r->channels + 2) * a1 - R_RGB(r->rgb, p2 * r->channels + 2) * a2)) /
          R_RGB_MAX;
}
#endif

gfloat
lqr_carver_read_cached_std (LqrCarver * r, gint x, gint y)
{
  gint z0 = r->raw[y][x];

  return r->rcache[z0];
}

gfloat
lqr_carver_read_cached_rgba (LqrCarver * r, gint x, gint y, gint channel)
{
  gint z0 = r->raw[y][x];

  return r->rcache[z0 * 4 + channel];
}

gfloat
lqr_carver_read_cached_custom (LqrCarver * r, gint x, gint y, gint channel)
{
  /* gint z0 = r->raw[y][x]; */

  /* TODO */
  return 0;
}

gfloat
lqr_energy_builtin_grad_all (gint x, gint y, gint img_width, gint img_height, LqrReaderWindow * rwindow, LqrGradFunc gf)
{
  gfloat gx, gy;

  gfloat (*bread_func) (LqrReaderWindow *, gint, gint);

  switch (lqr_rwindow_get_read_t(rwindow))
    {
      case LQR_ER_BRIGHT:
        bread_func = lqr_rwindow_read_bright;
        break;
      case LQR_ER_LUMA:
        bread_func = lqr_rwindow_read_luma;
        break;
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return 0;
    }

  if (y == 0)
    {
      gy = bread_func(rwindow, 0, 1) - bread_func(rwindow, 0, 0);
    }
  else if (y < img_height - 1)
    {
      gy = (bread_func(rwindow, 0, 1) - bread_func(rwindow, 0, -1)) / 2;
    }
  else
    {
      gy = bread_func(rwindow, 0, 0) - bread_func(rwindow, 0, -1);
    }

  if (x == 0)
    {
      gx = bread_func(rwindow, 1, 0) - bread_func(rwindow, 0, 0);
    }
  else if (x < img_width - 1)
    {
      gx = (bread_func(rwindow, 1, 0) - bread_func(rwindow, -1, 0)) / 2;
    }
  else
    {
      gx = bread_func(rwindow, 0, 0) - bread_func(rwindow, -1, 0);
    }

  return gf(gx, gy);
}

gfloat
lqr_energy_builtin_grad_norm (gint x, gint y, gint img_width, gint img_height, LqrReaderWindow * rwindow, gpointer extra_data)
{
  return lqr_energy_builtin_grad_all(x, y, img_width, img_height, rwindow, lqr_grad_norm);
}

gfloat
lqr_energy_builtin_grad_sumabs (gint x, gint y, gint img_width, gint img_height, LqrReaderWindow * rwindow, gpointer extra_data)
{
  return lqr_energy_builtin_grad_all(x, y, img_width, img_height, rwindow, lqr_grad_sumabs);
}

gfloat
lqr_energy_builtin_grad_xabs (gint x, gint y, gint img_width, gint img_height, LqrReaderWindow * rwindow, gpointer extra_data)
{
  return lqr_energy_builtin_grad_all(x, y, img_width, img_height, rwindow, lqr_grad_xabs);
}

gfloat
lqr_energy_builtin_null (gint x, gint y, gint img_width, gint img_height, LqrReaderWindow * rwindow, gpointer extra_data)
{
  return 0;
}


#if 0
/* compute energy at x, y */
gfloat
lqr_energy_abs (LqrCarver * r, gint x, gint y)
{
  gfloat gx, gy;

  if (y == 0)
    {
      gy = (*(r->nrg_builtin->rfabs))(r, x, y + 1, x, y);
    }
  else if (y < r->h - 1)
    {
      gy = 0.5 * (*(r->nrg_builtin->rfabs))(r, x, y + 1, x, y - 1);
    }
  else
    {
      gy = (*(r->nrg_builtin->rfabs))(r, x, y, x, y - 1);
    }

  if (x == 0)
    {
      gx = (*(r->nrg_builtin->rfabs))(r, x + 1, y, x, y);
    }
  else if (x < r->w - 1)
    {
      gx = 0.5 * (*(r->nrg_builtin->rfabs))(r, x + 1, y, x - 1, y);
    }
  else
    {
      gx = (*(r->nrg_builtin->rfabs))(r, x, y, x - 1, y);
    }
  return (*(r->nrg_builtin->gf))(gx, gy);
}
#endif

/* gradient function for energy computation */
LQR_PUBLIC
LqrRetVal
lqr_carver_set_energy_function_builtin (LqrCarver * r, LqrEnergyFuncBuiltinType ef_ind)
{
  switch (ef_ind)
    {
      case LQR_EF_GRAD_NORM:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_norm, 1, LQR_ER_BRIGHT, NULL));
        break;
      case LQR_EF_GRAD_SUMABS:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_sumabs, 1, LQR_ER_BRIGHT, NULL));
        break;
      case LQR_EF_GRAD_XABS:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_xabs, 1, LQR_ER_BRIGHT, NULL));
        break;
      case LQR_EF_LUMA_GRAD_NORM:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_norm, 1, LQR_ER_LUMA, NULL));
        break;
      case LQR_EF_LUMA_GRAD_SUMABS:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_sumabs, 1, LQR_ER_LUMA, NULL));
        break;
      case LQR_EF_LUMA_GRAD_XABS:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_grad_xabs, 1, LQR_ER_LUMA, NULL));
        break;
      case LQR_EF_NULL:
        CATCH (lqr_carver_set_energy_function (r, lqr_energy_builtin_null, 0, LQR_ER_BRIGHT, NULL));
        break;
      default:
        return LQR_ERROR;
    }

  r->nrg_builtin_flag = TRUE;

  return LQR_OK;
}

LQR_PUBLIC
LqrRetVal
lqr_carver_set_energy_function (LqrCarver * r, LqrEnergyFunc en_func, gint radius,
                LqrEnergyReaderType reader_type, gpointer extra_data)
{
  CATCH_F (r->root == NULL);

  r->nrg = en_func;
  r->nrg_radius = radius;
  r->nrg_read_t = reader_type;
  r->nrg_extra_data = extra_data;

  r->nrg_builtin_flag = FALSE;

  lqr_rwindow_destroy (r->rwindow);

  r->rwindow = lqr_rwindow_new (radius, reader_type, r->use_rcache);

  return LQR_OK;
}

gfloat *
lqr_carver_generate_rcache_bright (LqrCarver * r)
{
  gfloat * buffer;
  int x, y;
  int z0 = 0;

  TRY_N_N (buffer = g_try_new (gfloat, r->w_start * r->h_start));

  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          buffer[z0++] = lqr_carver_read_brightness (r, x, y);
        }
    }

  return buffer;
}

gfloat *
lqr_carver_generate_rcache_luma (LqrCarver * r)
{
  gfloat * buffer;
  int x, y;
  int z0 = 0;

  TRY_N_N (buffer = g_try_new (gfloat, r->w_start * r->h_start));

  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          buffer[z0++] = lqr_carver_read_luma (r, x, y);
        }
    }

  return buffer;
}

gfloat *
lqr_carver_generate_rcache_rgba (LqrCarver * r)
{
  gfloat * buffer;
  int x, y, k;
  int z0 = 0;

  TRY_N_N (buffer = g_try_new (gfloat, r->w_start * r->h_start * 4));

  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          for (k = 0; x < 4; k++)
            {
              buffer[z0++] = lqr_carver_read_rgba (r, x, y, k);
            }
        }
    }

  return buffer;
}

gfloat *
lqr_carver_generate_rcache_custom (LqrCarver * r)
{
  /* TODO */
  return NULL;
}

gfloat *
lqr_carver_generate_rcache (LqrCarver * r)
{
#ifdef __LQR_DEBUG__
  assert (r->w == r->w_start);
  assert (r->h == r->h_start);
#endif /* __LQR_DEBUG__ */

  switch (r->nrg_read_t)
    {
      case LQR_ER_BRIGHT:
        return lqr_carver_generate_rcache_bright(r);
      case LQR_ER_LUMA:
        return lqr_carver_generate_rcache_luma(r);
      case LQR_ER_RGBA:
        return lqr_carver_generate_rcache_rgba(r);
      case LQR_ER_CUSTOM:
        return lqr_carver_generate_rcache_custom(r);
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return NULL;
    }
}

