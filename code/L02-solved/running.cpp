//OpneCV
//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

struct ArgumentList {
  std::string image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
  unsigned int threshold;                  //!< binary threshold
  unsigned int running;                    //!< running average window
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.threshold=50;
  args.running=10;

  while ((c = getopt (argc, argv, "hi:t:k:w:")) != -1)
    switch (c)
    {
      case 'i':
	args.image_name = optarg;
	break;
      case 'w':
	args.running = atoi(optarg);
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
	  "   -w arg                   running average window (default 10)"<<std::endl<<
	  "   -k arg                   threshold value (default 50)"<<std::endl<<
	  "   -t arg                   wait before next frame (ms) [default = 0]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;

  std::cout<<"Simple program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  ////data
  //prev frame

  std::vector<cv::Mat> m_pframe_bg;

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
    // RUNNING AVERAGE

    if(!m_pframe_bg.empty())
    {
      cv::Mat bgf(image.size(),CV_32FC1,cv::Scalar(0)), bg, fg;

      for(std::vector<cv::Mat>::const_iterator ii = m_pframe_bg.begin(); ii != m_pframe_bg.end(); ++ii)
      {
	cv::accumulate(*ii,bgf); // only CV_32FC1 or CV64FC1

      }
      bgf /= m_pframe_bg.size();

      bgf.convertTo(bg, CV_16SC1);
      image.convertTo(fg, CV_16SC1);
      cv::threshold(cv::abs(fg-bg), fg, args.threshold, 255, cv::THRESH_BINARY);



      bg.convertTo(bg, CV_8UC1);
      fg.convertTo(fg, CV_8UC1);
      cv::namedWindow("Running Average-BG",cv::WINDOW_NORMAL);
      cv::namedWindow("Running Average",cv::WINDOW_NORMAL);
      cv::imshow("Running Average-BG", bg);
      cv::imshow("Running Average", fg);

    }
    
    m_pframe_bg.push_back(image);
    if(m_pframe_bg.size()>args.running)
    {
      m_pframe_bg.erase(m_pframe_bg.begin());
    }

    std::cout << "SIZE : " << m_pframe_bg.size() << std::endl;

  
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
