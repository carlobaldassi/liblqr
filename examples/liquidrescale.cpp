/* LiquidRescaling Library DEMO program
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

#include <pngwriter.h>
#include <lqr.h>
#include <getopt.h>
#include "liquidrescale.h"

using namespace std;

gchar *infile = NULL;
gchar *outfile = NULL;
gchar *pres_infile = NULL;
gchar *pres_outfile = NULL;
gchar *disc_infile = NULL;
gchar *disc_outfile = NULL;
gchar *vmap_infile = NULL;
gchar *vmap_outfile = NULL;
gint new_width = 0;
gint new_height = 0;
gfloat rigidity = 0;
gint max_step = 1;
gint pres_strength = 1000;
gint disc_strength = 1000;
LqrResizeOrder res_order = LQR_RES_ORDER_HOR;

gfloat new_width_p = 0;
gfloat new_height_p = 0;

gboolean quiet = FALSE;

clock_t clock_start, clock_now;


/*** MAIN ***/

int
main (int argc, char **argv)
{/*{{{*/

  /*** read the command line ***/
  TRAP (parse_command_line (argc, argv));

  /*** open input and output files ***/
  pngwriter png (1, 1, 0, outfile);
  png.readfromfile (infile);

  /*** old and new size ***/
  gint old_width = png.getwidth ();
  gint old_height = png.getheight ();

  if (new_width_p)
    {
      new_width = (gint) (new_width_p * old_width / 100);
    }
  if (new_height_p)
    {
      new_height = (gint) (new_height_p * old_height / 100);
    }

  new_width = (new_width ? new_width : old_width);
  new_height = (new_height ? new_height : old_height);

  if ((new_width < 2) || (new_width >= 2 * old_width))
    {
      cerr << "The width should be between 2 and twice the original width" << endl;
      exit (1);
    }

  if ((new_height < 2) || (new_height >= 2 * old_height))
    {
      cerr << "The height should be between 2 and twice the original height" << endl;
      exit (1);
    }

  /*** print some information on screen ***/
  if (!quiet)
    {
      cout << "resizing " << infile << " [" << old_width << "x" << old_height << "] into " <<
	outfile << " [" << new_width << "x" << new_height << "]" << endl << endl << flush;
    }



  if (vmap_infile)
    {
      info_msg ("will read VMap from", vmap_infile);
    }
  if (vmap_outfile)
    {
      info_msg ("will write VMap to", vmap_outfile);
    }

  if ((new_height != old_height) && (new_width != old_width) && (vmap_outfile))
    {
      cerr << "Warning: only the last computed visibility map will be written" << endl;
    }


  /*** read and check the feature masks ***/
  pngwriter png_pmask;
  pngwriter png_dmask;

  if (pres_infile)
    {
      info_msg ("will read preservation mask from", pres_infile);
      if (pres_outfile)
        {
	  info_msg ("will write preservation mask to", pres_outfile);
	  png_pmask.pngwriter_rename(pres_outfile);
	}
      png_pmask.readfromfile (pres_infile);
      if (pres_outfile)
        {
	  if (png_pmask.getwidth () != old_width)
	    {
	      cerr << "Fatal error: preservation mask width does not match input file width" << endl;
	      cerr << "cannot honour the --pres-out-file option" << endl;
	      exit (1);
	    }
	  if (png_pmask.getheight () != old_height)
	    {
	      cerr << "Fatal error: preservation mask height does not match input file height" << endl;
	      cerr << "cannot honour the --pres-out-file option" << endl;
	      exit (1);
	    }
	}
    }

  if (disc_infile)
    {
      info_msg ("will read discard mask from", disc_infile);
      if (disc_outfile)
        {
	  info_msg ("will write discrad mask to", disc_outfile);
	  png_dmask.pngwriter_rename(disc_outfile);
	}
      png_dmask.readfromfile (disc_infile);
      if (disc_outfile)
        {
	  if (png_dmask.getwidth () != old_width)
	    {
	      cerr << "Fatal error: discard mask width does not match input file width" << endl;
	      cerr << "cannot honour the --disc-out-file option" << endl;
	      exit (1);
	    }
	  if (png_dmask.getheight () != old_height)
	    {
	      cerr << "Fatal error: discard mask height does not match input file height" << endl;
	      cerr << "cannot honour the --disc-out-file option" << endl;
	      exit (1);
	    }
	}
    }

  /* convert the images into rgb buffers to use them with the library */

  guchar *rgb_buffer;
  guchar *rgb_pres_buffer = NULL;
  guchar *rgb_disc_buffer = NULL;

  TRAP_N (rgb_buffer = rgb_buffer_from_image (&png));
  if (pres_infile)
    {
      TRAP_N (rgb_pres_buffer = rgb_buffer_from_image (&png_pmask));
    }
  if (disc_infile)
    {
      TRAP_N (rgb_disc_buffer = rgb_buffer_from_image (&png_dmask));
    }

  if (!quiet)
    {
      cout << endl;
    }

  

  /**** (I) GENERATE THE MULTISIZE REPRESENTATION ****/

  /* (I.1) swallow the buffer in a (minimal) LqrCarver object 
   *       (arguments are width, height and number of colour channels) */
  LqrCarver *carver;
  TRAP_N (carver = lqr_carver_new (rgb_buffer, old_width, old_height, 3));

  /* (I.2) if we have to attach other images, we have to do so
   *       immediatley after the carver construction (we might
   *       initialize the carver first, but not load a visibility map)*/
  LqrCarver *pres_carver;
  if (pres_outfile)
    {
      TRAP_N (pres_carver = lqr_carver_new(rgb_pres_buffer, old_width, old_height, 3));
      TRAP (lqr_carver_attach(carver, pres_carver));
      /* note : this way the rgb_pres_buffer is lost, and we will have
       *        to read it again in order to use it as a bias (this could
       *        be avoided but the code would be harder to read) */
    }

  LqrCarver *disc_carver;
  if (disc_outfile)
    {
      TRAP_N (disc_carver = lqr_carver_new(rgb_disc_buffer, old_width, old_height, 3));
      TRAP (lqr_carver_attach(carver, disc_carver));
    }

  /* (I.3) next step depends on whether we have a pre-computed
   *       map to use or not*/
  if (!vmap_infile)
    {
      /* (I.3a.1) initialize the carver (with default values),
       *          so that we can do the resizing */
      TRAP (lqr_carver_init (carver, max_step, rigidity));
      /* (I.3a.2) add the bias (positive to preserve, negative
       *          to discard) */
      if (pres_infile)
	{
	  TRAP_N (rgb_pres_buffer = rgb_buffer_from_image (&png_pmask));
	  TRAP (lqr_carver_bias_add_rgb (carver, rgb_pres_buffer, pres_strength, 3)); 
	}
      if (disc_infile)
	{
	  TRAP_N (rgb_disc_buffer = rgb_buffer_from_image (&png_dmask));
	  TRAP (lqr_carver_bias_add_rgb (carver, rgb_disc_buffer, -disc_strength, 3)); 
	}
    }
  else
    {
      /* (I.3b.1) read a visibility map from a file */
      LqrVMap * vmap;
      vmap = load_vmap_from_file (vmap_infile);
      if (vmap == NULL)
        {
	  cerr << "Unable to load vmap, aborting" << endl;
	  exit (1);
        }  
      /* (I.3b.2) load it in the carver */
      TRAP (lqr_vmap_load (carver, vmap));
    }

  /* (I.4) if we want to access the visibility maps, we have
   *       to set it in the carver before rescaling occurs */
  if (vmap_outfile)
    {
      lqr_carver_set_dump_vmaps (carver);
    }



  /**** (II) SET UP THE PROGRESS INDICATOR ****/

  if (!quiet)
    {
      /* (II.1) generate a progress with default values */
      LqrProgress *progress;
      TRAP_N (progress = lqr_progress_new ());
      /* (II.2) set up with custom commands */
      init_progress (progress);
      /* (II.3) attach the progress to out multisize image */
      lqr_carver_set_progress (carver, progress);
    }


  /**** (III) LIQUID RESCALE ****/

  /* (III.1) set the rescaling order */
  lqr_carver_set_resize_order (carver, res_order);
  /* (III.2) invoke the rescaling function
   *         this step could be reiterated at wish */
  TRAP (lqr_carver_resize (carver, new_width, new_height));


  /**** (IV) SAVE THE VISIBILITY MAP ****/

  if (vmap_outfile)
    {
      LqrVMap *vmap;
      TRAP_N (vmap = lqr_vmap_dump(carver));
      TRAP (save_vmap_to_file (vmap, vmap_outfile));
    }

#if 0
  /* alternative way */
  LqrVMapList * vmap_list = lqr_vmap_list_start(carver);
  if (vmap_outfile)
    {
      TRAP (save_vmap_to_file (lqr_vmap_list_current(vmap_list), vmap_outfile));
      lqr_vmap_list_next(vmap_list);
    }
#endif


  /**** (V) READOUT THE MULTISIZE IMAGE ****/

  /* (V.1) readout the main image */
  TRAP (write_carver_to_image (carver, &png));
  /* (V.2) readout the atteched images */
  LqrCarverList *carver_list = lqr_carver_list_start(carver);
  if (pres_outfile)
    {
      TRAP (write_carver_to_image (lqr_carver_list_current(carver_list), &png_pmask));
      lqr_carver_list_next(carver_list);
    }
  if (disc_outfile)
    {
      TRAP (write_carver_to_image (lqr_carver_list_current(carver_list), &png_dmask));
    }



  /*** close files (write the images on disk) ***/

  png.close ();
  if (pres_outfile)
    {
      png_pmask.close();
    }
  if (disc_outfile)
    {
      png_dmask.close();
    }


  return 0;
}/*}}}*/

/*** PARSE COMMAND LINE ***/

LqrRetVal parse_command_line (int argc, char **argv)
{/*{{{*/
  int i;
  int c;
  struct option lopts[]= {
    {"file", required_argument, NULL, 'f'},
    {"out-file", required_argument, NULL, 'o'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"rigidity", required_argument, NULL, 'r'},
    {"max-step", required_argument, NULL, 's'},
    {"pres-file", required_argument, NULL, 'p'},
    {"pres-out-file", required_argument, NULL, 'P'},
    {"pres-strength", required_argument, NULL, 'z'},
    {"disc-file", required_argument, NULL, 'd'},
    {"disc-out-file", required_argument, NULL, 'D'},
    {"disc-strength", required_argument, NULL, 'x'},
    {"vmap-out-file", required_argument, NULL, 'v'},
    {"vmap-in-file", required_argument, NULL, 'V'},
    {"vertical-first", no_argument, NULL, 't'},
    {"quiet", no_argument, NULL, 'q'},
    {"help", no_argument, NULL, '#'},
    {NULL,0,NULL,0}
  };



  while ((c = getopt_long(argc, argv, "f:,o:,w:,h:,r:,s:,p:,P:,z:,d:,D:,x:,v:,V:,tq", lopts, &i)) != EOF) {
    switch (c)
    {
      case 'f':
	infile = optarg;
	break;
      case 'o':
	outfile = optarg;
	break;
      case 'w':
	if (optarg[strlen(optarg) - 1] == '%')
	  {
	    new_width_p = atof(optarg); 
	  }
	else
	  {  
	    new_width = atoi(optarg);
	    new_width_p = 0;
	  }
	break;
      case 'h':
	if (optarg[strlen(optarg) - 1] == '%')
	  {
	    new_height_p = atof(optarg);
	  }
	else
	  {  
	    new_height = atoi(optarg);
	    new_height_p = 0;
	  }
	break;
      case 'r':
	rigidity = atof(optarg);
	break;
      case 's':
	max_step = atoi(optarg);
	break;
      case 'p':
	pres_infile = optarg;
	break;
      case 'P':
	pres_outfile = optarg;
	break;
      case 'z':
	pres_strength = atoi(optarg);
	break;
      case 'd':
	disc_infile = optarg;
	break;
      case 'D':
	disc_outfile = optarg;
	break;
      case 'x':
	disc_strength = atoi (optarg);
	break;
      case 'v':
	vmap_infile = optarg;
	break;
      case 'V':
	vmap_outfile = optarg;
	break;
      case 't':
	res_order = LQR_RES_ORDER_VERT;
	break;
      case 'q':
	quiet = 1;
	break;
      case '#':
	help(argv[0]);
	exit (0);
      default:
	cerr << "Error parsing command line. Use " << argv[0] << " --help for usage instructions." << endl;
	return LQR_ERROR;
    }
  }

  if (!infile)
    {
     cerr << "Input file missing." << endl;
     return LQR_ERROR;
    }
  
  if (!outfile)
    {
     cerr << "Output file missing." << endl;
     return LQR_ERROR;
    } 

  if (!new_width && !new_height && !new_width_p && !new_height_p)
    {
      cerr << "At least one of --width or --height has to be specified and be different from 0." << endl;
      return LQR_ERROR;
    }

  if (pres_outfile && !pres_infile)
    {
      cerr << "Option --pres-out-file can't be used without --pres-in-file." << endl;
      return LQR_ERROR;
    }

  if (disc_outfile && !disc_infile)
    {
      cerr << "Option --disc-out-file can't be used without --disc-in-file." << endl;
      return LQR_ERROR;
    }

  if (pres_strength < 0)
    {
      cerr << "Preservation strength cannot be negative." << endl;
      return LQR_ERROR;
    }

  if (disc_strength < 0)
    {
      cerr << "Discard strength cannot be negative." << endl;
      return LQR_ERROR;
    }

  if (rigidity < 0)
    {
      cerr << "Rigidity cannot be negative." << endl;
      return LQR_ERROR;
    }

  if (max_step < 0)
    {
      cerr << "Max step cannot be negative." << endl;
      return LQR_ERROR;
    }


  return LQR_OK;
}/*}}}*/

void help(char *command)
{/*{{{*/
  cout << "Usage: " << command << " -f <file> -o <out-file> [ -w <width> | -h <height> ] [ ... ]" << endl;
  cout << "  Options:" << endl;
  cout << "    -f <file> or --file <file>" << endl;
  cout << "        Specifies input file. Must be in PNG format, in RGB without alpha channel" << endl;
  cout << "    -o <out-file> or --out-file <out-file>" << endl;
  cout << "        Specifies the output file." << endl;
  cout << "    -w <width> or --width <width>" << endl;
  cout << "        The new width. It must be between 2 and twice the origianl width." << endl;
  cout << "        If it is 0, or it is not given, the width is unchanged." << endl;
  cout << "        If it is followed by a %, it is interpreted as a percentage with" << endl;
  cout << "        respect to the original width (and needs not to be an integer)." << endl;
  cout << "    -h <height> or --height <height>" << endl;
  cout << "        Same as -w for the height." << endl;
  cout << "    -r <rigidity> or --rigidity < rigidity>" << endl;
  cout << "        Seams rigidity. Any non-negative value is allowed. Defaults to 0." << endl;
  cout << "    -s <max-step> or --max-step <max-step>" << endl;
  cout << "        Maximum seam transversal step. Default value is 1." << endl;
  cout << "    -p <pres-file> or --pres-file <pres-file>" << endl;
  cout << "        File to be used as a mask for features preservation. It must be in the" << endl;
  cout << "        same format as the input file." << endl;
  cout << "    -P <pres-out-file> or --pres-out-file <pres-out-file>" << endl;
  cout << "        If specified, the preservation mask will be rescaled along with the" << endl;
  cout << "        input file, and the output will be written in the specified file." << endl;
  cout << "        The size of the preservation mask has to match that of the original" << endl;
  cout << "        image." << endl;
  cout << "    -z <pres-strength> or --pres-strength <pres-strength>" << endl;
  cout << "        Preservation mask strength. Any integer non-negative value is allowed." << endl;
  cout << "        Defaults to 1000." << endl;
  cout << "    -d <disc-file> or --disc-file <disc-file>" << endl;
  cout << "        File to be used as a mask for features discard. It must be in the" << endl;
  cout << "        same format as the input file." << endl;
  cout << "    -D <disc-out-file> or --disc-out-file <disc-out-file>" << endl;
  cout << "        Same as -P for the discard mask." << endl;
  cout << "    -x <disc-strength> or --disc-strength <disc-strength>" << endl;
  cout << "        Same as -z for the discard mask." << endl;
  cout << "    -v <vmap-file> or --vmap-file <vmap-file>" << endl;
  cout << "        Reads the visibility map from the specified file. The rescaling will fail" << endl;
  cout << "        if it is asked to go beyond the depth of the map." << endl;
  cout << "    -V <vmap-out-file> or --vmap-out-file <vmap-out-file>" << endl;
  cout << "        Writes the visibility map in the specified file. Currently, only the first one." << endl;
  cout << "    -t or --vertical-first" << endl;
  cout << "        Rescale vertically first (instead of horizontally)." << endl;
  cout << "    -q or --quiet" << endl;
  cout << "        Quiet mode." << endl;
  cout << "    --help" << endl;
  cout << "        This help." << endl;
}/*}}}*/
  

/*** AUXILIARY I/O FUNCTIONS ***/

/* convert the image in the right format */
guchar *
rgb_buffer_from_image (pngwriter * png)
{/*{{{*/
  gint x, y, k, bpp;
  gint w, h;
  guchar *buffer;

  /* get info from the image */
  w = png->getwidth ();
  h = png->getheight ();
  bpp = 3;                      // we assume an RGB image here 

  /* allocate memory to store w * h * bpp unsigned chars */
  buffer = g_try_new (guchar, bpp * w * h);
  g_assert (buffer != NULL);

  /* start iteration (always y first, then x, then colours) */
  for (y = 0; y < h; y++)
    {
      for (x = 0; x < w; x++)
        {
          for (k = 0; k < bpp; k++)
            {
              /* read the image channel k at position x,y */
              buffer[(y * w + x) * bpp + k] =
                (guchar) (png->dread (x + 1, y + 1, k + 1) * 255);
              /* note : the x+1,y+1,k+1 on the right side are
               *        specific the pngwriter library */
            }
        }
    }

  return buffer;
}/*}}}*/

/* readout the multizie image */
LqrRetVal
write_carver_to_image (LqrCarver * r, pngwriter * png)
{/*{{{*/
  gint x, y;
  guchar * rgb;
  gdouble red, green, blue;
  gint w, h;

  /* make sure the image is RGB */
  CATCH_F (lqr_carver_get_bpp(r) == 3);

  /* resize the image canvas as needed to
   * fit for the new size
   * (remember it may be transposed) */
  w = lqr_carver_get_width (r);
  h = lqr_carver_get_height (r);
  png->resize (w, h);

  /* initialize image reading */
  lqr_carver_scan_reset (r);

  /* readout (no nedd to init rgb) */
  while (lqr_carver_scan(r, &x, &y, &rgb))
    {
      /* convert the output into doubles */
      red = (gdouble) rgb[0] / 255;
      green = (gdouble) rgb[1] / 255;
      blue = (gdouble) rgb[2] / 255;

      /* plot (pngwriter's coordinates start from 1,1) */
      png->plot (x + 1, y + 1, red, green, blue);
    }

  return LQR_OK;
}/*}}}*/


/*** PROGRESS INDICATOR ***/

/* set up the progress functions */
LqrRetVal
my_progress_init (const gchar * message)
{/*{{{*/
  printf ("%s --------------------  0.00%% (00:00.00)", message);
  fflush (stdout);
  clock_start = clock();
  return LQR_OK;
}/*}}}*/

LqrRetVal
my_progress_update (gdouble percentage)
{/*{{{*/
  gint i;
  gfloat p;
  for (i = 0; i < 38; i++) {
    printf("\b");
  }
  for (i = 0; i < 20 * percentage; i++) {
    printf("*");
  }
  for (; i < 20; i++) {
    printf("-");
  }
  p = 100 * percentage;
  printf(" %s%.2f%%", p < 10 ? " " : "", p);

  gfloat t;
  clock_now = clock();
  t = (gfloat) (clock_now - clock_start) / CLOCKS_PER_SEC;

  i = (gint) t;

  gint min, sec, cent;

  min = i / 60;
  sec = i % 60;
  cent = (gint) ((t - i) * 100);
  printf (" (%s%i:%s%i.%s%i)", min < 10 ? "0" : "", min, sec < 10 ? "0" : "", sec, cent < 10 ? "0" : "", cent);
  fflush (stdout);
  return LQR_OK;
}/*}}}*/

LqrRetVal
my_progress_end (const gchar * message)
{/*{{{*/
  gint i;
  for (i = 0; i < 38; i++) {
    printf("\b");
  }
  printf ("********************   %s", message);

  gfloat t;
  clock_now = clock();
  t = (gfloat) (clock_now - clock_start) / CLOCKS_PER_SEC;

  i = (gint) t;

  gint min, sec, cent;

  min = i / 60;
  sec = i % 60;
  cent = (gint) ((t - i) * 100);
  printf (" (%s%i:%s%i.%s%i)\n", min < 10 ? "0" : "", min, sec < 10 ? "0" : "", sec, cent < 10 ? "0" : "", cent);

  fflush (stdout);
  return LQR_OK;
}/*}}}*/

/* setup the progress machinery */
void
init_progress (LqrProgress * progress)
{/*{{{*/
  lqr_progress_set_init (progress, my_progress_init);
  lqr_progress_set_update (progress, my_progress_update);
  lqr_progress_set_end (progress, my_progress_end);
  lqr_progress_set_init_width_message(progress, "Resizing width  :");
  lqr_progress_set_init_height_message(progress, "Resizing height :");
  lqr_progress_set_end_width_message(progress, "done");
  lqr_progress_set_end_height_message(progress, "done");
  lqr_progress_set_update_step (progress, 0.01);
}/*}}}*/


/*** VISIBILTY MAPS ***/

/* convert a visibility map in binary and stores it in a file */
LqrRetVal save_vmap_to_file (LqrVMap *vmap, gchar * name)
{/*{{{*/
  FILE *sink;
  gint *buffer;
  gint width, height, depth, orientation;
  gint y, x;
  gint32 vs;

  buffer = lqr_vmap_get_data(vmap);
  width = lqr_vmap_get_width(vmap);
  height = lqr_vmap_get_height(vmap);
  depth = lqr_vmap_get_depth(vmap);
  orientation = lqr_vmap_get_orientation(vmap);

  /* open file */
  if ((sink = fopen(name, "wb")) == NULL)
    {
      cerr << "error opening outfile : " << name << endl;
      return LQR_ERROR;
    }

  /* VMAP : filtype */
  fprintf(sink, "VMAP[");

  /* HEAD : the header */
  fprintf(sink, "HEAD[");

  fprintf(sink, "[width=%i]", width);
  fprintf(sink, "[height=%i]", height);
  fprintf(sink, "[orientation=%i]", orientation);
  fprintf(sink, "[depth=%i]", depth);
  fprintf(sink, "[comment=()]");

  /* close HEAD */
  fprintf(sink, "]");

  /* BODY : the data */
  fprintf(sink, "BODY[");

  /* vmap is a buffer of gint's */
  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
	  vs = buffer[y * width + x];
	  fprintf(sink, "%c%c%c%c", (vs >> 24) & 0xFF, ((vs >> 16) & 0xFF), ((vs >> 8) & 0xFF), vs & 0xFF );
	}
    }
  /* close BODY */
  fprintf(sink, "]");

  /* close VMAP */
  fprintf(sink, "]");

  /* close file */
  fclose(sink);

  return LQR_OK;
}/*}}}*/

/* reads a binary file and converts it into a visibility map */
LqrVMap * load_vmap_from_file (gchar *name)
{/*{{{*/
  FILE *input;
  gint x, y, z0;
  gint w, h, depth, orientation;
  gint c, i;
  guchar i1, i2, i3, i4;
  gint32 vs;
  gboolean w_ok, h_ok, d_ok, o_ok;
  gchar read_buffer[RBS];
  gchar read_tag[RBS];
  gint *buffer;
  LqrVMap *vmap;

  /* flags */
  w_ok = h_ok = d_ok = o_ok = FALSE;
  
  w = h = depth = orientation = 0;

  /* open file */
  CHECK_OR_N (((input = fopen (name, "rb")) != NULL), "can't open vmap file");

  /* read filetype */
  CHECK_OR_N (fscanf(input, "VMAP[") != EOF, "not a VMAP file");

  /* read the header */
  CHECK_OR_N (fscanf(input, "HEAD[") != EOF, "missing vmap header");

  /* scan the header */
  while (1) {
    CHECK_OR_N ((c = getc(input)) != EOF, "corrupted vmap header");
    if (c == ']')
      {
	/* the header has ended */
	break;
      }
    ungetc(c, input);

    /* start reading a tag */
    CHECK_OR_N (fscanf(input, "[") != EOF, "corrupted vmap header");

    /* start reading the tag name */
    i = 0;
    while (((c = getc(input)) != EOF) && (i < RBS))
      {
	if (c == '=')
	  {
	    /* the tag name has ended */
	    break;
	  }
	read_tag[i++] = c;
      }
    CHECK_OR_N (i < RBS, "vmap tag name too long");

    read_tag[i] = '\0';

    /* start reading the tag content */
    i = 0;
    while (((c = getc(input)) != EOF) && (i < RBS))
      {
	if (c == ']')
	  {
	    /* the tag has ended */
	    break;
	  }
	read_buffer[i++] = c;
      }
    CHECK_OR_N (i < RBS, "tag content too long");
    read_buffer[i] = '\0';

    /* set the corresponding variable and the flag */
    if (strncmp(read_tag, "width", RBS) == 0)
      {
	w = atoi(read_buffer);
	w_ok = TRUE;
      }
    else if (strncmp(read_tag, "height", RBS) == 0)
      {
	h = atoi(read_buffer);
	h_ok = TRUE;
      }
    else if (strncmp(read_tag, "depth", RBS) == 0)
      {
	depth = atoi(read_buffer);
	d_ok = TRUE;
      }
    else if (strncmp(read_tag, "orientation", RBS) == 0)
      {
	orientation = atoi(read_buffer);
	o_ok = TRUE;
      }
    else if (strncmp(read_tag, "comment", RBS) == 0)
      {
	/* discard comments */
      }
    else
      {
	cerr << "warning : unknown tag : " << read_tag << endl;
      }
  }

  /* check if all the needed quantities are there */
  CHECK_OR_N (w_ok && h_ok && d_ok && o_ok, "missing vmap tags");

  /* start reading the data */
  CHECK_OR_N (fscanf(input, "BODY[") != EOF, "missing vmap body");

  /* allocate memory for the vmap buffer */
  TRY_N_N (buffer = g_try_new0(gint, w * h));

  /* scan the data */
  y = x = 0;
  while ((y < h) && ((c = getc(input)) != EOF))
    {
      ungetc (c, input);

      /* read a 32 bit integer */
      CHECK_OR_N (fscanf(input, "%c%c%c%c", &i1, &i2, &i3, &i4) == 4, "vmap data corrupted");
      vs = i4 + (i3 << 8) + (i2 << 16) + (i1 << 24);
      
      /* set it in the buffer */
      z0 = y * w + x;
      buffer[z0] = vs;

      /* update the coordinates */
      x++;
      if (x == w)
        {
	  x = 0;
	  y++;
	}
    }


  /* test if the amount of data read is correct */
  CHECK_OR_N ((x == 0) && (y == h), "vmap data corrupted");

  /* end reading the data */
  CHECK_OR_N (fscanf(input, "]") != EOF, "unterminated vmap body");

  /* end reading the vmap file */
  CHECK_OR_N (fscanf(input, "]") != EOF, "unterminated vmap file");

  /* create the vmap object with all the data aquired */
  TRY_N_N (vmap = lqr_vmap_new (buffer, w, h, depth, orientation));

  return vmap;
}/*}}}*/

/*** EXTRA ***/

void info_msg(const gchar * msg, const gchar *name)
{/*{{{*/
  if (!quiet)
    {
      cout << "  + " << msg << " " << name << endl << flush;
    }
}/*}}}*/
