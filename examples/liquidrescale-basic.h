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

#endif /* __LIQUIDRESCALE_BASIC_H__ */
