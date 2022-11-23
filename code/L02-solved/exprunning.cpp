//OpneCV
//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

struct ArgumentList {
  std::string image_name;	  //!< image file name
  int wait_t;                     //!< waiting time
  int threshold;                  //!< binary threshold
  float alpha;                    //!< exponential running average weight
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.threshold=50;
  args.alpha=0.5;


  while ((c = getopt (argc, argv, "hi:t:k:a:")) != -1)
    switch (c)
    {
      case 'a':
	args.alpha = atof(optarg);
	break;
      case 'i':
	args.image_name = optarg;
	break;
      case 'k':
	args.threshold = atoi(optarg);
	break;
      case 't':
	args.wait_t = atoi(optarg);
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                         produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -k arg                   threshold value (default 50)"<<std::endl<<
	  "   -t arg                   wait before next frame (ms) [default = 0]"<<
	  "   -a arg                   alpha param [default = 0.5]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;

  std::cout<<"Exprunning program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  ////data
  //prev frame
  cv::Mat m_pframe_bg;

  while(!exit_loop)
  {
    //generating file name
    //
    //multi frame case
    if(args.image_name.find('%') != std::string::npos)
      sprintf(frame_name,(const char*)(args.image_name.c_str()),frame_number);
    else //single frame case
      sprintf(frame_name,"%s",args.image_name.c_str());

    //opening file
    std::cout<<"Opening "<<frame_name<<std::endl;

    cv::Mat image = cv::imread(frame_name, CV_8UC1);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }


    //////////////////////
    //processing code here
    ////BACKGROUND SUBTRACTION

    //////////////
    // PREV FRAME
    if(m_pframe_bg.empty()) {
      image.copyTo(m_pframe_bg);
    } else {
      cv::Mat fg, bg;

      image.convertTo(fg, CV_16SC1);
      m_pframe_bg.convertTo(bg, CV_16SC1);

      cv::threshold(cv::abs(fg-bg), fg, args.threshold, 255, cv::THRESH_TOZERO);
      fg.convertTo(fg, CV_8UC1);

      cv::namedWindow("Exp Running-BG",cv::WINDOW_NORMAL);
      cv::namedWindow("Exp Running",cv::WINDOW_NORMAL);
      cv::imshow("Exp Running-BG", m_pframe_bg);
      cv::imshow("Exp Running", fg);

      m_pframe_bg=args.alpha*m_pframe_bg+(1-args.alpha)*image;
    }
      //display image
    cv::namedWindow("image", cv::WINDOW_NORMAL);
    cv::imshow("image", image);

    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    //here you can implement some looping logic using key value:
    // - pause
    // - stop
    // - step back
    // - step forward
    // - loop on the same frame
    if(key == 'q')
      exit_loop = true;

    frame_number++;
  }

  return 0;
}
