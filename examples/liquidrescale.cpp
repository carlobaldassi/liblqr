#include <pngwriter.h>
#include <lqr/lqr.h>

using namespace std;

/*** AUXILIARY FUNCTIONS DECLARATIONS ***/

guchar *rgb_buffer_from_image (pngwriter * png);
gboolean write_raster_to_image (LqrRaster * r, pngwriter * png);

gboolean my_progress_init (const gchar * message);
gboolean my_progress_update (gdouble percentage);
gboolean my_progress_end (const gchar * message);
void init_progress (LqrProgress * progress);

/*** MAIN ***/

int
main (int argc, char **argv)
{
  /*** read commandline ***/

  if (argc < 5)
    {
      cerr << "error: usage: " << argv[0] << " <input_file> <output_file> <x> <y>" << endl;
      cerr << "       * files must be RGB in PNG format" << endl;
      cerr << "       * setting <x> = 0 or <y> = 0 means keep original size"
        << endl;
      exit (1);
    }

  /*** open input and output files ***/

  char *infile;
  char *outfile;
  infile = argv[1];
  outfile = argv[2];

  /* setup outfile */
  pngwriter png (1, 1, 0, outfile);
  /* read image */
  png.readfromfile (infile);

  /*** old and new size ***/

  int w0 = png.getwidth ();
  int h0 = png.getheight ();

  int w1 = atoi (argv[3]);
  int h1 = atoi (argv[4]);
  w1 = (w1 ? w1 : w0);
  h1 = (h1 ? h1 : h0);

  /*** print some information on screen ***/

  cout << "resizing " << infile << " [" << w0 << "x" << h0 << "] into " <<
    outfile << " [" << w1 << "x" << h1 << "]" << endl << flush;



  /**** (I) GENERATE THE MULTISIZE REPRESENTATION ****/

  /* (1) generate a buffer (this must be done outside the library) */
  guchar *rgb_buffer;
  rgb_buffer = rgb_buffer_from_image (&png);

  /* (2) swallow the buffer in a (minimal) LqrRaster object 
   *     (arguments are width, height and number of color channels) */
  LqrRaster *rasta;
  rasta = lqr_raster_new (rgb_buffer, w0, h0, 3);

  /* (3) initialize the raster (with default values),
   *     so that we can do the resizing
   *     (arguments are max seam step and seam rigidity) */
  lqr_raster_init (rasta, 1, 0);


  /**** (II) SET UP THE PROGRESS INDICATOR (OPTIONAL) ****/

  /* (1) generate a progress with default values */
  LqrProgress *progress = lqr_progress_new ();
  /* (2) set up with custom commands */
  init_progress (progress);
  /* (3) attach the progress to out multisize image */
  lqr_raster_set_progress (rasta, progress);


  /**** (III) LIQUID RESCALE ****/

  lqr_raster_resize (rasta, w1, h1);


  /**** (IV) READOUT THE MULTISIZE IMAGE ****/

  write_raster_to_image (rasta, &png);




  /*** close file (write the image on disk) ***/

  png.close ();

  return 0;
}

/*** AUXILIARY I/O FUNCTIONS ***/

/* convert the image in the right format */
guchar *
rgb_buffer_from_image (pngwriter * png)
{
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

  /* start iteration (always y first, then x, then colors) */
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
}

/* readout the multizie image */
gboolean
write_raster_to_image (LqrRaster * r, pngwriter * png)
{
  gint x, y;
  gdouble red, green, blue;
  gint w, h;

  /* make sure the image is RGB */
  TRY_F_F (r->bpp == 3);

  /* resize the image canvas as needed to
   * fit for the new size
   * (remember it may be transposed) */
  w = lqr_raster_get_width (r);
  h = lqr_raster_get_height (r);
  png->resize (w, h);

  /* initialize image reading */
  lqr_raster_read_reset (r);

  do
    {
      x = lqr_raster_read_x (r);
      y = lqr_raster_read_y (r);
      /* readout the pixel information */
      red = (gdouble) lqr_raster_read_c (r, 0) / 255;
      green = (gdouble) lqr_raster_read_c (r, 1) / 255;
      blue = (gdouble) lqr_raster_read_c (r, 2) / 255;

      png->plot (x + 1, y + 1, red, green, blue);
    }
  while (lqr_raster_read_next (r));

  return TRUE;
}

/*** PROGRESS INDICATOR ***/

/* set up some simple progress functions */
gboolean
my_progress_init (const gchar * message)
{
  printf ("%s ", message);
  fflush (stdout);
  return TRUE;
}

gboolean
my_progress_update (gdouble percentage)
{
  printf ("+");
  fflush (stdout);
  return TRUE;
}

gboolean
my_progress_end (const gchar * message)
{
  printf (" %s\n", message);
  fflush (stdout);
  return TRUE;
}

/* setup the progress machinery (use standard messages) */
void
init_progress (LqrProgress * progress)
{
  progress->init = my_progress_init;
  progress->update = my_progress_update;
  progress->end = my_progress_end;
}
