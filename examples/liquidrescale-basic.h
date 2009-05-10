/* LiquidRescaling Library EXAMPLE program
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
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

#ifndef __LIQUIDRESCALE_BASIC_H__
#define __LIQUIDRESCALE_BASIC_H__

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
      case LQR_USRCANCEL: \
	cerr << "Cancelled by user, aborting." << endl; \
	exit (1); \
	break; \
      default: \
	break; \
    } \
}G_STMT_END

/*** PARSE COMMAND LINE ***/
LqrRetVal parse_command_line(int argc, char **argv);
void help(char *command);

/*** RGB FILE I/O ***/
guchar *rgb_buffer_from_image(pngwriter *png);
LqrRetVal write_carver_to_image(LqrCarver *r, pngwriter *png);

#endif /* __LIQUIDRESCALE_BASIC_H__ */
