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


#ifndef __LQR_COLOUR_H__
#define __LQR_COLOUR_H__

struct _LqrColourRGBA;

typedef struct _LqrColourRGBA LqrColourRGBA;

struct _LqrColourRGBA
{
  gdouble r;
  gdouble g;
  gdouble b;
  gdouble a;
};

void lqr_colour_rgba_set(LqrColourRGBA * colour, gdouble red, gdouble green, gdouble blue, gdouble alpha);

#endif /* __LQR_COLOUR_H__ */
