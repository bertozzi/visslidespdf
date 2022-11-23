//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>

void myfilter2D(const cv::Mat& src, const cv::Mat& krn, cv::Mat& out, int stride=1);
void gaussianKrnl(float sigma, int r, cv::Mat& krnl);
void GaussianBlur(const cv::Mat& src, float sigma, int r, cv::Mat& out, int stride=1);


struct ArgumentList {
  std::string image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;
  int ksize = 3;
  int stride = 1;
  float sigma = 1.0f;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    exit(0);
  }

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

    cv::Mat image = cv::imread(frame_name, cv::IMREAD_GRAYSCALE);
    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    //display image
    cv::namedWindow("original image", cv::WINDOW_NORMAL);
    cv::imshow("original image", image);


    // PROCESSING

    cv::Mat blurred, blurdisplay;
    GaussianBlur(image, sigma, ksize/2, blurred, stride);

    blurred.convertTo(blurdisplay, CV_8UC1);

    cv::namedWindow("Gaussian", cv::WINDOW_NORMAL);
    cv::imshow("Gaussian", blurdisplay);

#if 0
    // SOBEL FILTERING
    // void cv::Sobel(InputArray src, OutputArray dst, int ddepth, int dx, int dy, int ksize = 3, double scale = 1, double delta = 0, int borderType = BORDER_DEFAULT)
    //sobel verticale come trasposto dell'orizzontale

    cv::Mat g, gx, gy, agx, agy, ag;
    cv::Mat my_gx, my_gy;
    cv::Mat h_sobel = (
       cv::Mat_<float>(3, 3) <<  -1, 0, 1,
	                         -2, 0, 2,
	                         -1, 0, 1
      );

    cv::Mat v_sobel = h_sobel.t();

    myfilter2D(image, h_sobel, my_gx, stride);
    myfilter2D(image, v_sobel, my_gy, stride);
    my_gx.convertTo(gx, CV_32FC1);
    my_gy.convertTo(gy, CV_32FC1);

    // compute magnitude
    cv::pow(gx.mul(gx) + gy.mul(gy), 0.5, g);
    // compute orientation
    cv::Mat orientation(gx.size(), CV_32FC1);
    float *dest = (float *)orientation.data;
    float *srcx = (float *)gx.data;
    float *srcy = (float *)gy.data;
    float *magn = (float *)g.data;
    for(int i=0; i<gx.rows*gx.cols; ++i)
      dest[i] = magn[i]>50 ? atan2f(srcy[i], srcx[i]) + 2*CV_PI: 0;
    // scale on 0-255 range
    cv::convertScaleAbs(gx, agx);
    cv::convertScaleAbs(gy, agy);
    cv::convertScaleAbs(g, ag);
    cv::namedWindow("sobel verticale", cv::WINDOW_NORMAL);
    cv::imshow("sobel verticale", agx);
    cv::namedWindow("sobel orizzontale", cv::WINDOW_NORMAL);
    cv::imshow("sobel orizzontale", agy);
    cv::namedWindow("sobel magnitude", cv::WINDOW_NORMAL);
    cv::imshow("sobel magnitude", ag);

    // trick to display orientation
    cv::Mat adjMap;
    cv::convertScaleAbs(orientation, adjMap, 255 / (2*CV_PI));
    cv::Mat falseColorsMap;
    cv::applyColorMap(adjMap, falseColorsMap, cv::COLORMAP_JET);
    cv::namedWindow("sobel orientation", cv::WINDOW_NORMAL);
    cv::imshow("sobel orientation", falseColorsMap);
#endif


    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    //here you can implement some looping logic using key value:
    // - pause
    // - stop
    // - step back
    // - step forward
    // - loop on the same frame


    switch(key)
    {
      case 'G':
	sigma *=2;
	std::cout << "Sigma: " << sigma << std::endl;
	break;
      case 'g':
	sigma /=2;
	std::cout << "Sigma: " << sigma << std::endl;
	break;
      case 's':
	if(stride != 1)
	  --stride;
	std::cout << "Stride: " << stride << std::endl;
	break;
      case 'S':
	++stride;
	std::cout << "Stride: " << stride << std::endl;
	break;
      case 'c':
	cv::destroyAllWindows();
	break;
      case 'p':
	std::cout << "Mat = "<< std::endl << image << std::endl;
	break;
      case 'k':
	{
	  static int sindex=0;
	  int values[]={3, 5, 7, 11 ,13};
	  ksize = values[++sindex%5];
	  std::cout << "Setting Kernel size to: " << ksize << std::endl;
	}
	break;
      case 'q':
	exit(0);
	break;
    }

    frame_number++;
  }

  return 0;
}

#include <unistd.h>
bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t = 0;

  while ((c = getopt (argc, argv, "hi:t:")) != -1)
    switch (c)
    {
      case 't':
        args.wait_t = atoi(optarg);
        break;
      case 'i':
        args.image_name = optarg;
        break;
      case 'h':
      default:
        std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
        std::cout<<"exit:  type q"<<std::endl<<std::endl;
        std::cout<<"Allowed options:"<<std::endl<<
          "   -h                       produce help message"<<std::endl<<
          "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
          "   -t arg                   wait before next frame (ms)"<<std::endl<<std::endl;
        return false;
    }
  return true;
}


void addPadding(const cv::Mat image, cv::Mat &out, int vPadding, int hPadding){
  out = cv::Mat(image.rows+vPadding*2, image.cols+hPadding*2, image.type(), cv::Scalar(0));

  for(int row=vPadding; row < out.rows-vPadding; ++row)
  {
    for(int col=hPadding; col < out.cols-hPadding; ++col)
    {
      for(int k=0; k < out.channels(); ++k)
      {
	out.data[((row*out.cols+col)*out.elemSize() + k*out.elemSize1())] = image.data[(((row-vPadding)*image.cols+col-hPadding)*image.elemSize() + k*image.elemSize1())];
      }
    }
  }
}

/*
 Src: singolo canale uint8
 Krn: singolo canale float32 di dimensioni dispari
 Out: singolo canale int32
 Stride: intero (default 1)
*/
void myfilter2D(const cv::Mat& src, const cv::Mat& krn, cv::Mat& out, int stride)
{

  if(!src.rows%2 || !src.cols%2)
  {
    std::cerr << "myfilter2D(): ERROR krn has not odd size!" << std::endl;
    exit(1);
  }

  int outsizey = (src.rows + (krn.rows/2)*2 - krn.rows)/(float)stride + 1;
  int outsizex = (src.cols + (krn.cols/2)*2 - krn.cols)/(float)stride + 1;
  out = cv::Mat(outsizey, outsizex, CV_32SC1);

  cv::Mat image;
  addPadding(src, image, krn.rows/2, krn.cols/2);


  int xc = krn.cols/2;
  int yc = krn.rows/2;


  int *outbuffer = (int *)   out.data;
  float *kernel  = (float *) krn.data; 

  for(int i = 0 ; i < out.rows; ++i)
  {
    for(int j = 0; j < out.cols; ++j)
    {
      int origy = i*stride + yc;
      int origx = j*stride + xc;
      float sum = 0;
      for(int ki = -yc; ki <= yc; ++ki)
      {
	for(int kj = -xc; kj <= xc; ++kj)
	{
	  sum += image.data[(origy+ki)*image.cols + (origx+kj)] * kernel[(ki+yc)*krn.cols + (kj+xc)];
	}
      }
      outbuffer[i*out.cols + j] = sum;
    }
  }
}


void gaussianKrnl(float sigma, int r, cv::Mat& krnl){
  float kernelSum=0;
  krnl = cv::Mat(r*2+1, 1, CV_32FC1);

  int yc = krnl.rows/2;

  float sigma2 = pow(sigma,2);

  for(int i = 0; i <= yc; i++){
    int y2 = pow(i-yc, 2);
    float gaussValue = pow(M_E, -(y2)/(2*sigma2));

    kernelSum += gaussValue;

    if(i != yc){
      kernelSum += gaussValue;
    }

    ((float *)krnl.data)[i]             = gaussValue;
    ((float *)krnl.data)[krnl.rows-i-1] = gaussValue;
  }

  //Normalize.
  for(int i = 0; i < krnl.rows; i++){
      ((float *)krnl.data)[i] /= kernelSum;
  }
}

void GaussianBlur(const cv::Mat& src, float sigma, int r, cv::Mat& out, int stride)
{
  cv::Mat vg, hg;

  gaussianKrnl(sigma, r, vg);
  hg=vg.t();
  std::cout << "DEBUG: Horizontal Gaussian Kernel:\n" << hg << "\nSum: " << cv::sum(hg)[0] << std::endl;

  cv::Mat tmp;
  myfilter2D(src, hg, tmp, stride);
  tmp.convertTo(tmp, CV_8UC1);
  myfilter2D(tmp, vg, out, stride);
}

