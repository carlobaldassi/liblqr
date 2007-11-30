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

#include <glib.h>

#include <lqr/lqr_base.h>
#include <lqr/lqr_vmap.h>
#include <lqr/lqr_vmap_list.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif /* __LQR_DEBUG__ */


/**** VMAP LIST FUNCTIONS ****/

LqrVMapList * 
lqr_vmap_list_append (LqrVMapList * list, LqrVMap * buffer)
{
  LqrVMapList * prev = NULL;
  LqrVMapList * now = list;
  while (now != NULL)
    {
      prev = now;
      now = now->next;
    }
  TRY_N_N (now = g_try_new(LqrVMapList, 1));
  now->next = NULL;
  now->current = buffer;
  if (prev)
    {
      prev->next = now;
    }
  if (list == NULL)
    {
      return now;
    }
  else
    {
      return list;
    }
}

void
lqr_vmap_list_destroy(LqrVMapList * list)
{
  LqrVMapList * now = list;
  if (now != NULL)
    {
      lqr_vmap_list_destroy(now->next);
      lqr_vmap_destroy(now->current);
    }
}

gboolean
lqr_vmap_list_foreach (LqrVMapList * list, LqrVMapFunc func, gpointer data)
{
  LqrVMapList * now = list;
  if (now != NULL)
    {
      TRY_F_F (func(now->current, data));
      return lqr_vmap_list_foreach (now->next, func, data);
    }
  return TRUE;
}

