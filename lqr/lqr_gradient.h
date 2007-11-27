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


#ifndef __LQR_GRADIENT_H__
#define __LQR_GRADIENT_H__

/**** gradient functions for energy evluation ****/
typedef double (*LqrGradFunc) (double, double);

enum _LqrGradFuncType
{
  LQR_GF_NORM,                  /* gradient norm : sqrt(x^2 + y^2)            */
  LQR_GF_NORM_BIAS,             /* gradient biased norm : sqrt(x^2 + 0.1 y^2) */
  LQR_GF_SUMABS,                /* sum of absulte values : |x| + |y|          */
  LQR_GF_XABS,                  /* x absolute value : |x|                     */
  LQR_GF_YABS,                  /* y absolute value : |y|                     */
  LQR_GF_NULL                   /* 0 */
};

typedef enum _LqrGradFuncType LqrGradFuncType;

double lqr_grad_norm (double x, double y);
double lqr_grad_norm_bias (double x, double y);
double lqr_grad_sumabs (double x, double y);
double lqr_grad_xabs (double x, double y);
double lqr_grad_yabs (double x, double y);
double lqr_grad_zero (double x, double y);

#endif /* __LQR_GRADIENT_H__ */
