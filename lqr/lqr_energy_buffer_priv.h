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


#ifndef __LQR_ENERGY_BUFEFR_PRIV_H__
#define __LQR_ENERGY_BUFFER_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_energy_buffer_priv.h"
#endif /* __LQR_BASE_H__ */

struct _LqrEnergyBuffer
{
  gfloat ** buffer;
  gint radius;
  LqrEnergyReaderType read_t;
  gboolean use_rcache;
  LqrCarver * carver;
  gint x;
  gint y;
};

typedef gfloat (*LqrReadFunc) (LqrCarver*, gint, gint);
typedef gfloat (*LqrReadFuncWithCh) (LqrCarver*, gint, gint, gint);
/* typedef glfoat (*LqrReadFuncAbs) (LqrCarver*, gint, gint, gint, gint); */


LqrRetVal lqr_energy_buffer_fill_std (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y);
LqrRetVal lqr_energy_buffer_fill_rgba (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y);
LqrRetVal lqr_energy_buffer_fill_custom (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y);
LqrRetVal lqr_energy_buffer_fill (LqrEnergyBuffer * ebuffer, LqrCarver * r, gint x, gint y);

LqrEnergyBuffer * lqr_energy_buffer_new_std (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
LqrEnergyBuffer * lqr_energy_buffer_new_rgba (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
LqrEnergyBuffer * lqr_energy_buffer_new_custom (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
LqrEnergyBuffer * lqr_energy_buffer_new (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
void lqr_energy_buffer_destroy (LqrEnergyBuffer * ebuffer);

#endif /* __LQR_ENERGY_BUFFER_PRIV_H__ */
