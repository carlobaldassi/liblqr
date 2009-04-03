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


#ifndef __LQR_ENERGY_PRIV_H__
#define __LQR_ENERGY_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_energy_priv.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_PUB_H__
#error "lqr_gradient_pub.h must be included prior to lqr_energy_pub.h"
#endif /* __LQR_GRADIENT_PUB_H__ */

typedef double (*LqrEnergyFunc) (LqrCarver*, gint, gint);
typedef double (*LqrReadFunc) (LqrCarver*, gint, gint);
//typedef double (*LqrReadFuncAbs) (LqrCarver*, gint, gint, gint, gint);


struct _LqrEnergy
{
  LqrEnergyFunc ef;
  LqrReadFunc rf;
  LqrGradFunc gf;
  //LqrReadFuncAbs rfabs;
};

inline gdouble lqr_carver_read_brightness (LqrCarver * r, gint x, gint y);
inline gdouble lqr_carver_read_luma (LqrCarver * r, gint x, gint y);
double lqr_energy_std (LqrCarver * r, gint x, gint y);
double lqr_energy_luma_and_chroma (LqrCarver * r, gint x, gint y);
double lqr_energy_null (LqrCarver * r, gint x, gint y);

#endif /* __LQR_ENERGY_PRIV_H__ */
