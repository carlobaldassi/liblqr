/*
 *
 * Image model, handles loading and saving images
*/

#include <iostream>
#include <fstream>
#include <sstream>

#include "opencv2/opencv.hpp"

#include "image.h"


Image::Image() {
}


/**
 * Load image from a file
 */
bool Image::load_file(std::string infile) {
  // Identify the image format
  char header[5];
  std::ifstream istream;
  istream.open(infile, std::ios::binary|std::ios::in);
  istream.read((char *)&header, 4);
  istream.close();

  if (identify_format(header, this->file_format, this->file_extension) < 0) {
    std::cout << "Cannot identify image format" << std::endl;
    return false;
  }

  // Open and resize the image with OpenCV
  src = cv::imread(infile);
  if (!src.data) {
    std::cout << "Unable to read in the image" << std::endl;
    return false;
  }

  return true;
}


/**
 * Save image to a file
 */
bool Image::save_file(std::string outfile, std::string file_format, int quality) {
  // TODO: Quality
  return cv::imwrite(outfile, this->src);
}

/**
 * Get the image width
 */
int Image::get_width() {
  return this->src.cols;
}

/**
 * Get the image height
 */
int Image::get_height() {
  return this->src.rows;
}

/**
 * Detect the image format (and file extension) based on its header
 */
bool Image::identify_format(char* header, std::string &file_format, std::string &file_extension) {
  if (!header) {
     return false;
  }

  unsigned char testread[5];
  memcpy(testread, header, 4);
  testread[4] = 0;

  if (!strcmp((char *)testread, "GIF8")) {
    file_format = "gif";
    file_extension = ".gif";
  } else if (testread[0] == 0xff && testread[1] == 0xd8) {
    file_format = "jpeg";
    file_extension = ".jpg";
  } else if (testread[0] == 0x89 && testread[1] == 0x50 && testread[2] == 0x4e && testread[3] == 0x47) {
    file_format = "png";
    file_extension = ".png";
  } else if (testread[0] == 0x49 && testread[1] == 0x49 && testread[2] == 0x2A) {
    file_format = "tiff";
    file_extension = ".tif";
  } else if (testread[0] == 0x42 && testread[1] == 0x4d) {
    file_format = "bmp";
    file_extension = ".bmp";
  } else {
    std::cout << "Error: cannot identify image format." << std::endl;
    return false;
  }

  std::cout << "Image format is: " << file_format << std::endl;
  return true;
}
