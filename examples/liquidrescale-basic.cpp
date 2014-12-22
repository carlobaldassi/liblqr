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

#include <lqr.h>
#include <getopt.h>
#include "liquidrescale-basic.h"
#include "opencv2/opencv.hpp"
#include "image.h"


using namespace std;

gchar *infile = NULL;
gchar *outfile = NULL;
gint new_width = 0;
gint new_height = 0;
gfloat rigidity = 0;
gint max_step = 1;

/*** MAIN ***/

int
main(int argc, char **argv)
{/*{{{*/

    /*** read the command line ***/
    TRAP(parse_command_line(argc, argv));

    /*** open input and output files ***/
    Image image;
    image.load_file(infile);

    /*** old and new size ***/
    gint old_width = image.get_width();
    gint old_height = image.get_height();

    new_width = (new_width ? new_width : old_width);
    new_height = (new_height ? new_height : old_height);

    if (new_width < 2) {
        cerr << "The width should be greater than 2" << endl;
        exit(1);
    }

    if (new_height < 2) {
        cerr << "The height should be greater than 2" << endl;
        exit(1);
    }

    /*** print some information on screen ***/
    cout << "resizing " << infile << " [" << old_width << "x" << old_height << "] into " <<
        outfile << " [" << new_width << "x" << new_height << "]" << endl << flush;

    /* convert the image into rgb buffers to use them with the library */

    guchar *rgb_buffer;

    TRAP_N(rgb_buffer = rgb_buffer_from_image(&image));

    /**** (I) GENERATE THE MULTISIZE REPRESENTATION ****/

    /* (I.1) swallow the buffer in a (minimal) LqrCarver object 
     *       (arguments are width, height and number of colour channels) */
    LqrCarver *carver;
    TRAP_N(carver = lqr_carver_new(rgb_buffer, old_width, old_height, 3));

    /* (I.2) initialize the carver (with default values),
     *          so that we can do the resizing */
    TRAP(lqr_carver_init(carver, max_step, rigidity));

    /**** (II) LIQUID RESCALE ****/

    TRAP(lqr_carver_resize(carver, new_width, new_height));

    /**** (III) READOUT THE MULTISIZE IMAGE ****/

    TRAP(write_carver_to_image(carver, image));

    /**** (IV) DESTROY THE CARVER OBJECT ****/

    lqr_carver_destroy(carver);


    return 0;
}/*}}}*/

/*** PARSE COMMAND LINE ***/

LqrRetVal
parse_command_line(int argc, char **argv)
{/*{{{*/
    int i;
    int c;
    struct option lopts[] = {
        {"file", required_argument, NULL, 'f'},
        {"out-file", required_argument, NULL, 'o'},
        {"width", required_argument, NULL, 'w'},
        {"height", required_argument, NULL, 'h'},
        {"rigidity", required_argument, NULL, 'r'},
        {"max-step", required_argument, NULL, 's'},
        {"help", no_argument, NULL, '#'},
        {NULL, 0, NULL, 0}
    };

    while ((c = getopt_long(argc, argv, "f:,o:,w:,h:,r:,s:", lopts, &i)) != EOF) {
        switch (c) {
            case 'f':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'w':
                new_width = atoi(optarg);
                break;
            case 'h':
                new_height = atoi(optarg);
                break;
            case 'r':
                rigidity = atof(optarg);
                break;
            case 's':
                max_step = atoi(optarg);
                break;
            case '#':
                help(argv[0]);
                exit(0);
            default:
                cerr << "Error parsing command line. Use " << argv[0] << " --help for usage instructions." << endl;
                return LQR_ERROR;
        }
    }

    if (!infile) {
        cerr << "Input file missing." << endl;
        return LQR_ERROR;
    }

    if (!outfile) {
        cerr << "Output file missing." << endl;
        return LQR_ERROR;
    }

    if (!new_width && !new_height) {
        cerr << "Either --width or --height has to be specified." << endl;
        return LQR_ERROR;
    }

    if (rigidity < 0) {
        cerr << "Rigidity cannot be negative." << endl;
        return LQR_ERROR;
    }

    if (max_step < 0) {
        cerr << "Max step cannot be negative." << endl;
        return LQR_ERROR;
    }

    return LQR_OK;
}/*}}}*/

void
help(char *command)
{/*{{{*/
    cout << "Usage: " << command << " -f <file> -o <out-file> [ -w <width> | -h <height> ] [ ... ]" << endl;
    cout << "  Options:" << endl;
    cout << "    -f <file> or --file <file>" << endl;
    cout << "        Specifies input file. Must be in PNG format, in RGB without alpha channel" << endl;
    cout << "    -o <out-file> or --out-file <out-file>" << endl;
    cout << "        Specifies the output file." << endl;
    cout << "    -w <width> or --width <width>" << endl;
    cout << "        The new width. It must be greater than 2." << endl;
    cout << "        If it is 0, or it is not given, the width is unchanged." << endl;
    cout << "    -h <height> or --height <height>" << endl;
    cout << "        Same as -w for the height." << endl;
    cout << "    -r <rigidity> or --rigidity < rigidity>" << endl;
    cout << "        Seams rigidity. Any non-negative value is allowed. Defaults to 0." << endl;
    cout << "    -s <max-step> or --max-step <max-step>" << endl;
    cout << "        Maximum seam transversal step. Default value is 1." << endl;
    cout << "    --help" << endl;
    cout << "        This help." << endl;
}/*}}}*/

/*** AUXILIARY I/O FUNCTIONS ***/

/* convert the image in the right format */
guchar *
rgb_buffer_from_image(Image *img)
{/*{{{*/

    gint w, h;

    /* get info from the image */
    w = img->get_width();
    h = img->get_height();
    // we assume an RGB image here
    
    // Create the image buffer
    int buffer_size = w * 3 * h;
    
    /* allocate memory to store w * h * channels unsigned chars */
    
    // This will automatically be freed by liblqr
    guchar *rgb_buffer = (guchar *)malloc(sizeof(guchar)*buffer_size);
    g_assert(rgb_buffer != NULL);
    memcpy(rgb_buffer, (unsigned char*)(img->src.data), buffer_size);
    
    return rgb_buffer;
}/*}}}*/


/*
 * Readout the multisize image
 */
LqrRetVal
write_carver_to_image(LqrCarver *r, Image &image) {
    int x, y;
    guchar *rgb;
    int w, h;
    
    // Make sure the image is RGB
    if (lqr_carver_get_channels(r) != 3) {
        std::cerr << "Error: Image is not RGB" << std::endl;
    }
    
    // resize the image canvas as needed to fit for the new size
    w = lqr_carver_get_width(r);
    h = lqr_carver_get_height(r);
    
    std::vector<unsigned char> buffer(w * 3 * h);
    
    // Initialize image reading
    lqr_carver_scan_reset(r);
    
    // Readout (no need to init rgb)

    while (lqr_carver_scan(r, &x, &y, &rgb)) {
        
        int index = (x + y*w) * 3;
        
        buffer[index] = rgb[0];
        buffer[index+1] = rgb[1];
        buffer[index+2] = rgb[2];
    }
    
    // Update the original image with the rescaled version
    cv::Mat dest(cv::Size(w, h), CV_8UC3, &buffer[0], cv::Mat::AUTO_STEP);
    dest.copyTo(image.src);
    
    image.save_file(outfile, "", 1);
    
    return LQR_OK;
}

/*
 * Flip an image
 */
int flip(Image &image) {
    cv::flip(image.src, image.src, 1);
    
    return 0;
}

/*
 * Rotate an image
 */
int rotate(Image &image, double angle) {
    int len = std::max(image.get_width(), image.get_height());
    cv::Point2f pt(len / 2., len / 2.);
    cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(image.src, image.src, r, cv::Size(len, len));
    
    return 0;
}




