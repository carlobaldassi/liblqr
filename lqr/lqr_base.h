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


#ifndef __LQR_BASE_H__
#define __LQR_BASE_H__

#define LQR_MAX_NAME_LENGTH (1024)

#define TRY_N_N(assign) if ((assign) == NULL) { return NULL; }
#define TRY_N_F(assign) if ((assign) == NULL) { return FALSE; }
#define TRY_F_N(assign) if ((assign) == FALSE) { return NULL; }
#define TRY_F_F(assign) if ((assign) == FALSE) { return FALSE; }

#if 0
#define __LQR_DEBUG__
#endif

#if 0
#define __LQR_CLOCK__
#endif

#if 0
#define __LQR_VERBOSE__
#endif

/**** OPERATIONAL_MODES ****/
enum _LqrMode
{
  LQR_MODE_NORMAL,
  LQR_MODE_LQRBACK,
  LQR_MODE_SCALEBACK
};

typedef enum _LqrMode LqrMode;

/**** RESIZE ORDER ****/
enum _LqrResizeOrder
{
  LQR_RES_ORDER_HOR,
  LQR_RES_ORDER_VERT
};

typedef enum _LqrResizeOrder LqrResizeOrder;

/**** CLASSES DECLARATIONS ****/

struct _LqrCursor;              /* a "smart" index to read the carver */
struct _LqrCarver;              /* the multisize image carver         */

typedef struct _LqrCursor LqrCursor;
typedef struct _LqrCarver LqrCarver;

#endif /* __LQR_BASE_H__ */
