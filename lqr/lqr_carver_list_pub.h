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

#ifndef __LQR_CARVER_LIST_PUB_H__
#define __LQR_CARVER_LIST_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver_list_pub.h"
#endif /* __LQR_BASE_H__ */


union _LqrDataTok;
typedef union _LqrDataTok LqrDataTok;

union _LqrDataTok
{
  LqrCarver *carver;
  gint integer;
  gpointer data;
};

typedef LqrRetVal (*LqrCarverFunc) (LqrCarver *carver, LqrDataTok data);


/**** LQR_CARVER_LIST CLASS DEFINITION ****/
struct _LqrCarverList;

typedef struct _LqrCarverList LqrCarverList;

struct _LqrCarverList
{
  LqrCarver * current;
  LqrCarverList * next;
};

/* LQR_CARVER_LIST PUBLIC FUNCTIONS */

LqrCarverList * lqr_carver_list_append (LqrCarverList * list, LqrCarver * buffer);
void lqr_carver_list_destroy (LqrCarverList * list);

LqrCarverList * lqr_carver_list_start(LqrCarver *r);
LqrCarver * lqr_carver_list_current(LqrCarverList *list);
LqrCarverList * lqr_carver_list_next (LqrCarverList * list);
LqrRetVal lqr_carver_list_foreach (LqrCarverList * list, LqrCarverFunc func, LqrDataTok data);

#endif /* __LQR_CARVER_LIST_PUB_H__ */


