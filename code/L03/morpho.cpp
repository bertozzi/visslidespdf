//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>        // std::time
#include <unistd.h>       // getopt

struct ArgumentList {
  std::string image_name;		    //!< image file name
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;

  while ((c = getopt (argc, argv, "hi:")) != -1)
    switch (c)
    {
      case 'i':
	args.image_name = optarg;
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<std::endl;
	return false;
    }
  return true;
}


int main(int argc, char **argv)
{
  char frame_name[256];

  std::cout<<"Simple program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  //generating file name
  sprintf(frame_name,"%s",args.image_name.c_str());

  //opening file
  std::cout<<"Opening "<<frame_name<<std::endl;

  //Per questo esercizione l'immagine e' sicuramente a toni di grigio
  cv::Mat image = cv::imread(frame_name,CV_8UC1);

  if(image.empty())
  {
    std::cout<<"Unable to open "<<frame_name<<std::endl;
    return 1;
  }

  //////////////////////
  //processing code here

  //ES1 Soglia adattiva
  //

  // esempio da OpenCV  da sostituire con il vostro codice (e comunque non ignora i pixel<50)
  cv::Mat binary;
  cv::threshold(image, binary, 0, 255, cv::THRESH_OTSU);

  //ES3 Morfologia Matematica
  //

  //elemento strutturante 3x3
  //
  //   1  1  1
  //   1  1* 1
  //   1  1  1

  //display image
  cv::namedWindow("thresholded", cv::WINDOW_AUTOSIZE);
  cv::imshow("thresholded", binary);
  cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
  cv::imshow("image", image);



  //wait for key
  cv::waitKey();

  return 0;
}
