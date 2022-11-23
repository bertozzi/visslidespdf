//OpneCV
//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
// stereo
#include <opencv2/calib3d.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdint.h>

const int MAX_DISPARITY=128;

struct ArgumentList {
  std::string left_image;
  std::string right_image;
  unsigned short max_d;
  unsigned short w;
};

void SAD_Disparity(const cv::Mat &l, const cv::Mat &r, unsigned short w_size, cv::Mat &out);
void VDisparity(const cv::Mat &disp, cv::Mat &out);
void VDisparity(const cv::Mat &l, const cv::Mat &r, cv::Mat &out);

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.max_d=MAX_DISPARITY;
  args.w=7;

  while ((c = getopt (argc, argv, "hl:r:d:w:")) != -1)
    switch (c)
    {
      case 'l':
	args.left_image = optarg;
	break;
      case 'r':
	args.right_image = optarg;
	break;
      case 'w':
	args.w = atoi(optarg);
	if(!(args.w%2) || !args.w)
	{
	  std::cerr << "Disparity window size must be an odd number>0" << std::endl;
	  exit(EXIT_FAILURE);
	}
	break;
      case 'd':
	args.max_d = atoi(optarg);
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -r arg                   right image name"<<std::endl<<
	  "   -l arg                   left image name"<<std::endl<<
	  "   -w arg                   disparity window size [default = 7]"<<std::endl<<
	  "   -d arg                   max disparity [default = " << MAX_DISPARITY << "]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

int main(int argc, char **argv)
{
  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  ////data
  //prev frame

  //opening file
  std::cout<<"Opening left image" << args.left_image <<std::endl;
  cv::Mat left = cv::imread(args.left_image.c_str(), CV_8UC1);
  if(left.empty())
  {
    std::cerr << "Unable to open " << args.left_image << std::endl;
    return 1;
  }

  //opening file
  std::cout<<"Opening right image" << args.right_image <<std::endl;
  cv::Mat right = cv::imread(args.right_image.c_str(), CV_8UC1);
  if(right.empty())
  {
    std::cerr << "Unable to open " << args.right_image << std::endl;
    return 1;
  }

  if(right.size() != left.size())
  {
    std::cerr << "The left and right images have different sizes" << std::endl;
    return 1;
  }


  cv::namedWindow("right", cv::WINDOW_NORMAL);
  cv::imshow("right", right);

  cv::namedWindow("left", cv::WINDOW_NORMAL);
  cv::imshow("left", left);

  cv::Mat disp;
  SAD_Disparity(left, right, args.w, disp);

  cv::namedWindow("Disparity", cv::WINDOW_NORMAL);
  cv::imshow("Disparity", disp*(255.0/MAX_DISPARITY));


  cv::Mat vdisp;
  VDisparity(disp, vdisp);

  cv::namedWindow("V-Disparity", cv::WINDOW_NORMAL);
  cv::imshow("V-Disparity", vdisp);

  cv::Mat vdisp2;
  VDisparity(left, right, vdisp2);

  cv::namedWindow("V-Disparity direct", cv::WINDOW_NORMAL);
  cv::imshow("V-Disparity direct", vdisp2);


  // OPENCV way for disparity

  cv::Mat cvdisp, cvdisp2;
  cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create(128,args.w); 
  stereo->compute(left, right, cvdisp);

  cvdisp.convertTo(cvdisp2, left.type());

  cv::namedWindow("OpenCV", cv::WINDOW_NORMAL);
  cv::imshow("OpenCV", cvdisp2);



  //wait for key or timeout
  cv::waitKey(0);

  return 0;
}

void SAD_Disparity(const cv::Mat &l, const cv::Mat &r, unsigned short w_size, cv::Mat &out)
{
  out = cv::Mat::zeros(l.size(), CV_8UC1);
  for(int row=w_size/2; row<(l.rows-w_size/2); ++row)
  {
    for(int col=w_size/2; col<(l.cols-w_size/2); ++col)
    {
      unsigned int minSAD=UINT_MAX;
      int minSAD_d=0;
      for(int d=0;d<MAX_DISPARITY && (col-d)>1; d++)
      {
	unsigned int SAD=0;
	for(int dr=-w_size/2; dr<=w_size/2; ++dr)
	{
	  for(int dc=-w_size/2; dc<=w_size/2; ++dc)
	  {
	    int adj_r  = row + dr;
	    int adj_lc = col + dc;
	    int adj_rc = col - d + dc;
	    SAD += abs( l.data[(adj_r*l.cols+adj_lc)*l.elemSize1()] - r.data[(adj_r*r.cols+adj_rc)*r.elemSize1()] );
	  }
	}
	if(SAD<minSAD)
	{
	  minSAD=SAD;
	  minSAD_d=d;
	}
      }
      out.data[(row*l.cols+col)*out.elemSize1()] = minSAD_d; 
    }
  }
}

void VDisparity(const cv::Mat &l, const cv::Mat &r, cv::Mat &out)
{
  out=cv::Mat::zeros(l.rows, MAX_DISPARITY, CV_8UC1);
  for(int row=0; row<l.rows; ++row)
  {
    for(int c=0; c<l.cols; ++c)
    {
      for(int d=0;d<MAX_DISPARITY && (c-d)>0; d++)
      {
	if( l.data[(row*l.cols+c)]==r.data[(row*l.cols+c-d)] )
	  ++out.data[(row*MAX_DISPARITY+d)];
      }
    }
  }

}

void VDisparity(const cv::Mat &disp, cv::Mat &out)
{
  out=cv::Mat::zeros(disp.rows, MAX_DISPARITY, CV_8UC1);
  for(int r=0; r<disp.rows; ++r)
  {
    for(int c=0; c<disp.cols; ++c)
    {
      out.data[r*MAX_DISPARITY + disp.data[r*disp.cols + c]] += 1;
    }
  }
}
