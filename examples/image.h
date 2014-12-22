
#ifndef __IMAGE_MODEL_H__
#define __IMAGE_MODEL_H__

#include <iostream>

#include "opencv2/opencv.hpp"


  class Image {
    public:
      cv::Mat src;
      std::string file_format;
      std::string file_extension;

      Image();
      bool load(std::ostringstream &instream);
      bool load_file(std::string infile);
      bool save(std::ostringstream &outstream, std::string file_format, int quality);
      bool save_file(std::string outfile, std::string file_format, int quality);
      int get_width();
      int get_height();

    private:
      bool identify_format(char* header, std::string &file_format, std::string &file_extension);
  };


#endif
