/* LiquidRescaling Library DEMO program
 * Copyright (C) 2007 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
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

#ifndef __LIQUIDRESCALE_H__
#define __LIQUIDRESCALE_H__

/*** SIGNAL HANDLING MACROS ***/

#define RBS (1000)
#define CHECK_OR_N(expr, mess) G_STMT_START{ \
  if (!(expr)) { \
    cerr << "Error: " << mess << endl; \
    return NULL; \
  } \
}G_STMT_END

#define TRAP_N(expr) G_STMT_START{ \
  if ((expr) == NULL) { \
    cerr << "Error: not enough memory, aborting" << endl; \
    exit(1); \
  } \
}G_STMT_END

#define TRAP(expr) G_STMT_START{ \
  switch (expr) \
    { \
      case LQR_ERROR: \
	cerr << "Fatal error, aborting." << endl; \
	exit (1); \
	break; \
      case LQR_NOMEM: \
	cerr << "Not enough memory, aborting." << endl; \
	exit (1); \
	break; \
      default: \
	break; \
    } \
}G_STMT_END 

/*** PARSE COMMAND LINE ***/
LqrRetVal parse_command_line (int argc, char **argv);
void help(char *command);

/*** RGB FILE I/O ***/
guchar *rgb_buffer_from_image (pngwriter * png);
LqrRetVal write_carver_to_image (LqrCarver * r, pngwriter * png);

/*** VMAP FILES I/O ***/
LqrRetVal save_vmap_to_file (LqrVMap *vmap, gchar * name);
LqrVMap * load_vmap_from_file (gchar *name);

/*** PROGRESS REPORT FUNCTIONS ***/
LqrRetVal my_progress_init (const gchar * message);
LqrRetVal my_progress_update (gdouble percentage);
LqrRetVal my_progress_end (const gchar * message);
void init_progress (LqrProgress * progress);

/*** EXTRA ***/
void info_msg(const gchar * msg, const gchar *name);

#endif /* __LIQUIDRESCALE_H__ */
