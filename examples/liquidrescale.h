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

#endif /* __LIQUIDRESCALE_H__ */
