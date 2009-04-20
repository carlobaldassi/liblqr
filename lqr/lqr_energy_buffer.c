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
#include <lqr/lqr_all.h>

LqrRetVal
lqr_energy_buffer_fill_std (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y)
{
  gfloat ** buffer;
  gint i, j;

  LqrReadFunc read_float;

  buffer = (float **) (ebuffer->buffer);

  switch (ebuffer->read_t)
    {
      case LQR_ER_BRIGHT:
        read_float = lqr_carver_read_brightness;
        break;
      case LQR_ER_LUMA:
        read_float = lqr_carver_read_luma;
        break;
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return LQR_ERROR;
    }

  for (i = -ebuffer->radius; i <= ebuffer->radius; i++)
    {
      for (j = -ebuffer->radius; j <= ebuffer->radius; j++)
        {
          if (x + i < 0 || x + i >= r->w || y + j < 0 || y + j >= r->h)
            {
              buffer[i][j] = 0;
            }
          else
            {
              buffer[i][j] = read_float (r, x + i, y + j);
            }
        }
    }
  
  return LQR_OK;
}

LqrRetVal
lqr_energy_buffer_fill_rgba (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y)
{
  gfloat ** buffer;
  gint i, j, k;

  buffer = (float **) (ebuffer->buffer);

  CATCH_F (lqr_energy_buffer_get_read_t (ebuffer) == LQR_ER_RGBA);

  for (i = -ebuffer->radius; i <= ebuffer->radius; i++)
    {
      for (j = -ebuffer->radius; j <= ebuffer->radius; j++)
        {
          if (x + i < 0 || x + i >= r->w || y + j < 0 || y + j >= r->h)
            {
              for (k = 0; k < 4; k++)
                {
                  buffer[i][4 * j + k] = 0;
                }
            }
          else
            {
              for (k = 0; k < 4; k++)
                {
                  buffer[i][4 * j + k] = lqr_carver_read_rgba (r, x + i, y + j, k);
                }
            }
        }
    }
  
  return LQR_OK;
}

LqrRetVal
lqr_energy_buffer_fill_custom (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y)
{
  /* TODO */
  return LQR_OK;
}


LqrRetVal
lqr_energy_buffer_fill (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y)
{
  CATCH_CANC (r);

  if (ebuffer->use_rcache)
    {
      ebuffer->carver = r;
      ebuffer->x = x;
      ebuffer->y = y;
      return LQR_OK;
    }

  switch (ebuffer->read_t)
    {
      case LQR_ER_BRIGHT:
      case LQR_ER_LUMA:
        CATCH (lqr_energy_buffer_fill_std(ebuffer, r, x, y));
        break;
      case LQR_ER_RGBA:
        CATCH (lqr_energy_buffer_fill_rgba(ebuffer, r, x, y));
        break;
      case LQR_ER_CUSTOM:
        CATCH (lqr_energy_buffer_fill_custom(ebuffer, r, x, y));
        break;
      default:
        return LQR_ERROR;
    }
  return LQR_OK;
}


LqrEnergyBuffer *
lqr_energy_buffer_new_std (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  LqrEnergyBuffer * out_ebuffer;
  gfloat ** out_buffer;
  gfloat * out_buffer_aux;

  gint buf_size1, buf_size2;
  gint i;

  TRY_N_N (out_ebuffer = g_try_new0 (LqrEnergyBuffer, 1));

  buf_size1 = (2 * radius + 1);
  buf_size2 = buf_size1 * buf_size1;

  TRY_N_N (out_buffer_aux = g_try_new0 (gfloat, buf_size2));
  TRY_N_N (out_buffer = g_try_new0 (gfloat *, buf_size1));
  for (i = 0; i < buf_size1; i++)
    {
      out_buffer[i] = out_buffer_aux + radius;
      out_buffer_aux += buf_size1;
    }
  out_buffer += radius;

  out_ebuffer->buffer = out_buffer;
  out_ebuffer->radius = radius;
  out_ebuffer->read_t = read_func_type;
  out_ebuffer->use_rcache = use_rcache;
  out_ebuffer->carver = NULL;
  out_ebuffer->x = 0;
  out_ebuffer->y = 0;
  
  return out_ebuffer;
}

LqrEnergyBuffer *
lqr_energy_buffer_new_rgba (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  LqrEnergyBuffer * out_ebuffer;
  gfloat ** out_buffer;
  gfloat * out_buffer_aux;

  gint buf_size1, buf_size2;
  gint i;

  TRY_N_N (out_ebuffer = g_try_new0 (LqrEnergyBuffer, 1));

  buf_size1 = (2 * radius + 1);
  buf_size2 = buf_size1 * buf_size1 * 4;

  TRY_N_N (out_buffer_aux = g_try_new0 (gfloat, buf_size2));
  TRY_N_N (out_buffer = g_try_new0 (gfloat *, buf_size1));
  for (i = 0; i < buf_size1; i++)
    {
      out_buffer[i] = out_buffer_aux + radius * 4;
      out_buffer_aux += buf_size1 * 4;
    }
  out_buffer += radius;

  out_ebuffer->buffer = out_buffer;
  out_ebuffer->radius = radius;
  out_ebuffer->read_t = read_func_type;
  out_ebuffer->use_rcache = use_rcache;
  out_ebuffer->carver = NULL;
  out_ebuffer->x = 0;
  out_ebuffer->y = 0;
  
  return out_ebuffer;
}

LqrEnergyBuffer *
lqr_energy_buffer_new_custom (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  LqrEnergyBuffer * out_ebuffer;

  TRY_N_N (out_ebuffer = g_try_new0 (LqrEnergyBuffer, 1));

  out_ebuffer->buffer = NULL;
  out_ebuffer->radius = radius;
  out_ebuffer->read_t = read_func_type;
  out_ebuffer->use_rcache = use_rcache;
  out_ebuffer->carver = NULL;
  out_ebuffer->x = 0;
  out_ebuffer->y = 0;
  
  /* TODO */
  return NULL;
}

LqrEnergyBuffer *
lqr_energy_buffer_new (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  switch (read_func_type)
    {
      case LQR_ER_BRIGHT:
      case LQR_ER_LUMA:
        return lqr_energy_buffer_new_std(radius, read_func_type, use_rcache);
      case LQR_ER_RGBA:
        return lqr_energy_buffer_new_rgba(radius, read_func_type, use_rcache);
      case LQR_ER_CUSTOM:
        return lqr_energy_buffer_new_custom(radius, read_func_type, use_rcache);
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return NULL;
    }
}

void
lqr_energy_buffer_destroy (LqrEnergyBuffer * ebuffer)
{
  gfloat ** buffer;

  if (ebuffer == NULL)
    {
      return;
    }

  if (ebuffer->buffer == NULL)
    {
      return;
    }

  switch (ebuffer->read_t)
    {
      case LQR_ER_BRIGHT:
      case LQR_ER_LUMA:
        buffer = ebuffer->buffer;
        buffer -= ebuffer->radius;
        buffer[0] -= ebuffer->radius;
        g_free(buffer[0]);
        g_free(buffer);
        break;
      case LQR_ER_RGBA:
        buffer = (gfloat **) (ebuffer->buffer);
        buffer -= ebuffer->radius * 4;
        buffer[0] -= ebuffer->radius * 4;
        g_free(buffer[0]);
        g_free(buffer);
        break;
      case LQR_ER_CUSTOM:
        /* TODO */
        return;
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return;
    }
}

LQR_PUBLIC
gfloat
lqr_energy_buffer_read_bright (LqrEnergyBuffer * ebuffer, gint x, gint y)
{
  if (ebuffer == NULL || ebuffer->read_t != LQR_ER_BRIGHT ||
      x < -ebuffer->radius || x > ebuffer->radius ||
      y < -ebuffer->radius || y > ebuffer->radius)
    {
      return 0;
    }

  if (ebuffer->use_rcache)
    {
      return lqr_carver_read_cached_std (ebuffer->carver, ebuffer->x + x, ebuffer->y + y);
    }

  return ebuffer->buffer[x][y];
}

LQR_PUBLIC
gfloat
lqr_energy_buffer_read_luma (LqrEnergyBuffer * ebuffer, gint x, gint y)
{
  if (ebuffer == NULL || ebuffer->read_t != LQR_ER_LUMA ||
      x < -ebuffer->radius || x > ebuffer->radius ||
      y < -ebuffer->radius || y > ebuffer->radius)
    {
      return 0;
    }

  if (ebuffer->use_rcache)
    {
      return lqr_carver_read_cached_std (ebuffer->carver, ebuffer->x + x, ebuffer->y + y);
    }

  return ebuffer->buffer[x][y];
}

LQR_PUBLIC
gfloat
lqr_energy_buffer_read_rgba (LqrEnergyBuffer * ebuffer, gint x, gint y, gint channel)
{
  if (ebuffer == NULL || ebuffer->read_t != LQR_ER_RGBA ||
      x < -ebuffer->radius || x > ebuffer->radius ||
      y < -ebuffer->radius || y > ebuffer->radius ||
      channel < 0 || channel > 3)
    {
      return 0;
    }

  if (ebuffer->use_rcache)
    {
      return lqr_carver_read_cached_rgba (ebuffer->carver, ebuffer->x + x, ebuffer->y + y, channel);
    }

  return ebuffer->buffer[x][4 * y + channel];
}

LQR_PUBLIC
gfloat
lqr_energy_buffer_read_custom (LqrEnergyBuffer * ebuffer, gint x, gint y, gint channel)
{
  /* gfloat ** buffer; */

  if (ebuffer == NULL || ebuffer->read_t != LQR_ER_CUSTOM ||
      x < -ebuffer->radius || x > ebuffer->radius ||
      y < -ebuffer->radius || y > ebuffer->radius)
    {
      return 0;
    }

  if (ebuffer->use_rcache)
    {
      return lqr_carver_read_cached_custom (ebuffer->carver, ebuffer->x + x, ebuffer->y + y, channel);
    }

  /* return ebuffer->buffer[x][y] + channel; */

  /* TODO */
  return 0;
}

LQR_PUBLIC
LqrEnergyReaderType
lqr_energy_buffer_get_read_t (LqrEnergyBuffer * ebuffer)
{
  return ebuffer->read_t;
}

LQR_PUBLIC
gint
lqr_energy_buffer_get_radius (LqrEnergyBuffer * ebuffer)
{
  return ebuffer->radius;
}


