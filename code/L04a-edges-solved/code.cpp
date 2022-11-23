//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

#define _USE_MATH_DEFINES
#define CONV 1
#define GAUSSVERT 2
#define GAUSSBLUR 3
#define BILATERAL 4
#define SOBEL 5
#define NMS 7
#define CANNY 9
//getopt()
#include <unistd.h>

using namespace cv;

struct ArgumentList {
  std::string image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
};


bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;

  //If followed by a colon then a value is required
  while ((c = getopt (argc, argv, "hi:t:")) != -1)
    switch (c)
    {
      case 'i':
        args.image_name = optarg;
        break;
      case 't':
        args.wait_t = atoi(optarg);
        break;
      case 'h':
      default:
        std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
        std::cout<<"exit:  type q"<<std::endl<<std::endl;
        std::cout<<"Allowed options:"<<std::endl<<
          "   -h                       produce help message"<<std::endl<<
          "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
          "   -t arg                   wait before next frame (ms) [default = 0]"<<std::endl<<std::endl<<std::endl;
        return false;
    }
  return true;
}

void initVerticalSobel(cv::Mat& krn){
  ((float *)krn.data)[0]=-1;
  ((float *)krn.data)[2]=1;
  ((float *)krn.data)[0+1*3]=-2;
  ((float *)krn.data)[2+1*3]=2;
  ((float *)krn.data)[0+2*3]=-1;
  ((float *)krn.data)[2+2*3]=1;
  ((float *)krn.data)[1]=0;
  ((float *)krn.data)[4]=0;  
  ((float *)krn.data)[7]=0;
}
void initHorizontalSobel(cv::Mat& krn){
  ((float *)krn.data)[0]=-1;
  ((float *)krn.data)[1]=-2;
  ((float *)krn.data)[2]=-1;
  ((float *)krn.data)[3]=0;
  ((float *)krn.data)[4]=0;
  ((float *)krn.data)[5]=0;
  ((float *)krn.data)[6]=1;
  ((float *)krn.data)[7]=2;
  ((float *)krn.data)[8]=1;
}

cv::Mat * addPadding(cv::Mat image,int vPadding,int hPadding){
  cv::Mat *result=new cv::Mat(image.rows+vPadding*2,image.cols+hPadding*2,CV_8UC1);

  for(int row=0;row<result->rows;row++){
    for(int col=0;col<result->cols;col++){
      for(int c=image.elemSize()-1;c>=0;c--){
        int resultIndex=(col+row*result->cols)*result->elemSize()+c;
        if(col<hPadding || col>image.cols+hPadding || row<vPadding || row>image.rows+vPadding){
          result->data[resultIndex]=0;
        }
        else{
          int sourceIndex=(col-hPadding+(row-vPadding)*image.cols)*image.elemSize()+c;
          result->data[resultIndex]=image.data[sourceIndex];
        }
      }
    }
  }
  return result;
}

int32_t getConvValue(const Mat& src,const Mat& krn,int i,int j){
  float kernelValue=0;
  for(int ki=0;ki<krn.rows;ki++){
    for(int kj=0;kj<krn.cols;kj++){
      kernelValue+=((int)src.data[(j+kj)+(i+ki)*src.cols])*((float *)krn.data)[kj+ki*krn.cols];
    }
  }
  return (int)kernelValue;

}

//Es1
void conv(const cv::Mat& src, const cv::Mat& krn, cv::Mat& out, int stride=1){
  out=cv::Mat(src.rows,src.cols,CV_32SC1);

  Mat image=*addPadding(src,krn.rows/2,krn.cols/2);

  int xc=krn.cols/2;
  int yc=krn.rows/2;


  for(int i=yc;i<image.rows-yc;i+=stride){
    for(int j=xc;j<image.cols-xc;j+=stride){
      int kernelValue=getConvValue(image,krn,i-yc,j-xc);
      ((int *) out.data)[(j-xc)+(i-yc)*out.cols]=kernelValue;
    }
  }
}


//Es2
//Controllare correttezza
void gaussianKrnl(float sigma, int r, cv::Mat& krnl){
  float kernelSum=0;
  krnl=Mat(r*2+1,1,CV_32FC1);
  
  int yc=krnl.rows/2;

  float sigma2=pow(sigma,2);

  for(int i=0;i<=yc;i++){
    int y2=pow(i-yc,2);
    float gaussValue=pow(M_E,-(y2)/(2*sigma2));

    kernelSum+=gaussValue;

    if(i!=yc){
      kernelSum+=gaussValue;
    }

    ((float *)krnl.data)[i]=gaussValue;
    ((float *)krnl.data)[krnl.rows-i-1]=gaussValue;
  }

  //Normalize.
  for(int i=0;i<krnl.rows;i++){
    for(int j=0;j<krnl.cols;j++){
       ((float *)krnl.data)[j+i*krnl.cols]= ((float *)krnl.data)[j+i*krnl.cols]/kernelSum;
    }
  }
}


//Es3
void GaussianBlur(const cv::Mat& src, float sigma, int r, cv::Mat& 
out, int stride=1){
  Mat verticalGaussianKrnl;
  gaussianKrnl(sigma,r,verticalGaussianKrnl);

  Mat tmpImage;
  conv(src,verticalGaussianKrnl,tmpImage,stride);

  //Conv returns a CV_32SC1
  tmpImage.convertTo(tmpImage,CV_8UC1);
  namedWindow("debug",WINDOW_NORMAL);
  imshow("debug", tmpImage);
  waitKey(0);
  destroyWindow("debug");

  Mat horizontalGaussianKrnl(1,2*r+1,verticalGaussianKrnl.type());
  for(int i=0;i<2*r+1;i++){
    ((float *)horizontalGaussianKrnl.data)[i]=((float *)verticalGaussianKrnl.data)[i];
  }

  conv(tmpImage,horizontalGaussianKrnl,out,stride);
  out.convertTo(out,CV_8UC1);
}


//Metodi extra es4
void gaussianKrnlBidimensional(float sigma, int r,Mat& krnl){
  // float kernelSum=0;
  krnl=Mat(r*2+1,2*r+1,CV_32FC1);
  
  int xc=krnl.cols/2;
  int yc=krnl.rows/2;

  float sigma2=pow(sigma,2);

  for(int i=0;i<=yc;i++){
    for(int j=0;j<=xc;j++){
      int x2=pow(j-xc,2);
      int y2=pow(i-yc,2);
      float gaussValue=pow(M_E,-(x2+y2)/(2*sigma2));

      // kernelSum+=gaussValue;

      // //Elements that are not on square simmetry are calculated 2 times, so, each time we must count only 2 value inserted.
      // if((i==yc && j!=xc) || (i!=yc && j==xc)){
      //   kernelSum+=gaussValue;
      // }
      // else if(i!=yc || j!=xc){//Elements can be propagated by simmetry, but you need to count them four times.
      //   kernelSum=kernelSum+3*gaussValue;
      // }

      //the propagation work only in rectangle and/or vertical/horizontal lines.
      ((float *)krnl.data)[j+i*krnl.cols]=gaussValue;
      ((float *)krnl.data)[(krnl.cols-j-1)+(krnl.rows-i-1)*krnl.cols]=gaussValue;
      ((float *)krnl.data)[(krnl.cols-j-1)+(i)*krnl.cols]=gaussValue;
      ((float *)krnl.data)[(j)+(krnl.rows-i-1)*krnl.cols]=gaussValue;
    }
  }

  //Normalize.
  // for(int i=0;i<krnl.rows;i++){
  //   for(int j=0;j<krnl.cols;j++){
  //      ((float *)krnl.data)[j+i*krnl.cols]= ((float *)krnl.data)[j+i*krnl.cols]/kernelSum;
  //   }
  // }
}

void getBilateralKrnl(const Mat& src,const Mat& gaussianKrn, Mat& krn,int xc,int yc,int r,int sigma){
  krn=Mat(2*r+1,2*r+1,CV_32FC1);

  int center=src.data[xc+yc*src.cols];
  int sigma2=pow(sigma,2);

  float sumBilateralValue=0;
  for(int i=-r;i<=r;i++){
    for(int j=-r;j<=r;j++){
      int dist=abs(center-src.data[(xc-j)+(yc-i)*src.cols]);
      float bilateralValue=pow(M_E,-dist/(2*sigma2))*((float *)gaussianKrn.data)[j+r+(i+r)*krn.cols];
      sumBilateralValue+=bilateralValue;
      ((float *)krn.data)[j+r+(i+r)*krn.cols]=bilateralValue;
    }
  }

  for(int i=0;i<krn.rows;i++){
    for(int j=0;j<krn.cols;j++){
      ((float *)krn.data)[j+i*krn.cols]=((float *)krn.data)[j+i*krn.cols]/sumBilateralValue;
    }
  }
}

int32_t getBilateralValue(const Mat& src,const Mat& bilateralKrn,int i,int j){
  float kernelValue=0;
  for(int ki=0;ki<bilateralKrn.rows;ki++){
    for(int kj=0;kj<bilateralKrn.cols;kj++){
      kernelValue+=((int)src.data[(j+kj)+(i+ki)*src.cols])*
                        ((float *)bilateralKrn.data)[kj+ki*bilateralKrn.cols];
    }
  }
  return (int)kernelValue;

}

//Es4
void bilateralFilter(const cv::Mat src, cv::Mat &dst, int d, float 
sigmaR, float sigmaD){
  int r=d/2;
  Mat gaussianKrnl;
  gaussianKrnlBidimensional(sigmaD,r,gaussianKrnl);

  dst=cv::Mat(src.rows,src.cols,CV_8UC1);

  Mat image=*addPadding(src,r,r);
  int xc=r;
  int yc=r;
  Mat bilateralKrnl;
  for(int i=yc;i<(image.rows-yc);i++){
    for(int j=xc;j<(image.cols-xc);j++){

      getBilateralKrnl(image,gaussianKrnl,bilateralKrnl,j,i,r,sigmaR);

      int value=getBilateralValue(image,bilateralKrnl,i-yc,j-xc);

      dst.data[(j-xc)+(i-yc)*dst.cols]=value;

    }
  }
}

void magnitudeSobel(Mat& magn,const Mat& xDerivate,const Mat& yDerivate){
  for(int i=0;i<magn.rows;i++){
    for(int j=0;j<magn.cols;j++){
      float x=pow(((int *)xDerivate.data)[j+i*xDerivate.cols],2);
      float y=pow(((int *)yDerivate.data)[j+i*yDerivate.cols],2);

      ((float *)magn.data)[j+i*magn.cols]=sqrt(x+y);
    }

  }
}

void orientationSobel(Mat& orient,const Mat& xDerivate,const Mat& yDerivate){
  for(int i=0;i<orient.rows;i++){
    for(int j=0;j<orient.cols;j++){
      float x=((int *)xDerivate.data)[j+i*xDerivate.cols];
      float y=((int *)yDerivate.data)[j+i*yDerivate.cols];

      ((float *)orient.data)[j+i*orient.cols]=atan2(y,x);
    }
  }
}

//Es5
void sobel3x3(const cv::Mat& src, cv::Mat& magn, cv::Mat& orient){
  int kernelDim=3;
  magn=Mat(src.rows,src.cols,CV_32FC1);
  orient=Mat(src.rows,src.cols,CV_32FC1);

  Mat verticalSobel(kernelDim,kernelDim,CV_32FC1);
  Mat horizontalSobel(kernelDim,kernelDim,CV_32FC1);

  initVerticalSobel(verticalSobel);
  initHorizontalSobel(horizontalSobel);

  Mat xDerivate;
  Mat yDerivate;
  conv(src,horizontalSobel,yDerivate,1); //Returns int32
  conv(src,verticalSobel,xDerivate,1); //Returns int32

  magnitudeSobel(magn,xDerivate,yDerivate);
  orientationSobel(orient,xDerivate,yDerivate);
}

//Es6 TODO

//Es7
template <typename T>
float bilinear(const cv::Mat& src, float r, float c){
  if(r<0){
    r=0;
  }
  if(c<0){
    c=0;
  }
  if(r>src.rows-1){
    r=src.rows-1;
  }
  if(c>src.cols-1){
    c=src.cols-1;
  }
  
  float yDist=r-(int)r;
  float xDist=c-(int)c;

  int value=src.at<T>(r,c)*(1-yDist)*(1-xDist);
  if(r<src.cols-1)
    value+=src.at<T>(r+1,c)*(yDist)*(1-xDist);
  if(c<src.cols-1)
    value+=src.at<T>(r,c+1)*(1-yDist)*(xDist);
  if(r<src.rows-1 && c<src.cols-1)
    value+=src.at<T>(r+1,c+1)*yDist*xDist; 

  return value;
}

//Es8
int findPeaks(const cv::Mat& magn, const cv::Mat& orient, cv::Mat& out){
  out=Mat(magn.rows,magn.cols,magn.type());
  for(int r=0;r<magn.rows;r++){
    for(int c=0;c<magn.cols;c++){
      float theta=orient.at<float>(r,c);
      float e1x=c+cos(theta);
      float e1y=r+sin(theta);
      float e2x=c-cos(theta);
      float e2y=r-sin(theta);

      float neigh1PixelValue=bilinear<float>(magn,e1y,e1x);
      float neigh2PixelValue=bilinear<float>(magn,e2y,e2x);

      float pixelValue=magn.at<float>(r,c);

      if(pixelValue<neigh1PixelValue || pixelValue<neigh2PixelValue){
        pixelValue=0;
      }
      ((float *)out.data)[c+r*magn.cols]=pixelValue;
    }
  }
}


//Es9 
struct pixel_t{
  int r;
  int c;
};

int doubleTh(const cv::Mat& magn, cv::Mat& out, float t1, float t2){
  std::vector<pixel_t> whitePixels;
  std::vector<pixel_t> grayPixels;
  out=Mat(magn.rows,magn.cols,CV_8UC1);

  for(int r=0;r<magn.rows;r++){
    for(int c=0;c<magn.cols;c++){
     if(magn.at<float>(r,c)>=t2){
      out.data[c+r*magn.cols]=255; 
      pixel_t pixel={.r=r,.c=c};
      whitePixels.push_back(pixel);
     }
     else if(magn.at<float>(r,c)<=t1){
      out.data[c+r*magn.cols]=0;
     }
     else{
       //For the hysteresis part
      out.data[c+r*magn.cols]=(t1+t2)/2;
      pixel_t pixel={.r=r,.c=c};
      grayPixels.push_back(pixel);
     }
    }
  }

  while(whitePixels.size()!=0){
    pixel_t pixel=whitePixels.back();
    whitePixels.pop_back();
    int r=pixel.r;
    int c=pixel.c;
    for(int i=-1;i<=1;i++){
      for(int j=-1;j<=1;j++){
        if(i!=0 || j!=0){    
          int actualC=c+j;
          int actualR=r+i;   
          if(actualC>0 && actualC<out.cols && actualR>0 && actualR<out.rows &&  out.at<unsigned char>(actualR,actualC)>t1 &&  out.at<unsigned char>(actualR,actualC)<t2)
          {
              
              out.data[actualC+actualR*magn.cols]=255; 
              pixel_t newPixel={.r=actualR,.c=actualC};
              whitePixels.push_back(newPixel);
          }
        }
      }
    }
  }
  for(pixel_t pixel : grayPixels){
    int r=pixel.r;
    int c=pixel.c;
    if(out.at<unsigned char>(r,c)!=255){
      out.data[c+r*magn.cols]=0;
    }
  }
 }


bool testImage(cv::Mat image){
  std::cout<<image.type()<<std::endl;
  if(image.empty())
  {
    std::cout<<"Unable to open "<<std::endl;
    return false;
  }
  return true;
}

int main(int argc, char **argv)
{
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

  cv::namedWindow("image", cv::WINDOW_NORMAL);

  cv::Mat image;
  cv::Mat resultImage;

  while(!exit_loop)
  {

    //opening file
    std::cout<<"Insert the number of the operation you want to do:"<<std::endl<<
              "1-  conv"<<std::endl<<
              "2-  gauss vertical"<<std::endl<<
              "3-  gauss blur"<<std::endl<<
              "4-  bilateral"<<std::endl<<
              "5-  sobel"<<std::endl<<
              "7-  Non Maxima Suppression"<<std::endl<<
              "9-  Canny"<<std::endl;
    sprintf(frame_name,"%s",args.image_name.c_str());

    int operation;
    std::cin>>operation;
    std::cout<<"Opening "<<frame_name<<std::endl;


    if(operation==CONV){
      image = cv::imread(frame_name,CV_8UC1);


      int kernelDim=3;
      cv::Mat convKernel(kernelDim,kernelDim,CV_32FC1);
      initVerticalSobel(convKernel);

      conv(image,convKernel,resultImage,1);
      resultImage.convertTo(resultImage,CV_8UC1);

      imshow("image", resultImage);
    }
    else if(operation==GAUSSVERT){
      cv::Mat krnl;
      int sigma=5;
      gaussianKrnl(sigma,2,krnl);
      for(int i=0;i<krnl.rows;i++){
        for(int j=0;j<krnl.cols;j++){
          std::cout<<((float *)krnl.data)[j+i*krnl.cols]<<" ";
        }
        std::cout<<std::endl;
      }
    }
    else if(operation==GAUSSBLUR){
      image = cv::imread(frame_name,CV_8UC1);

      int sigma=5;
      int r=3;
      GaussianBlur(image,sigma,r,resultImage);

      imshow("image", resultImage);
    }
    else if(operation==BILATERAL){
      image = cv::imread(frame_name,CV_8UC1);
      bilateralFilter(image,resultImage,5,5,5);
      imshow("image", resultImage);
    }
    else if(operation==SOBEL){
      image=imread(frame_name,CV_8UC1);
      Mat magn;
      Mat orient;
      sobel3x3(image,magn,orient);

      magn.convertTo(magn,CV_8UC1);
      imshow("image",magn);

      Mat adjMap;
      convertScaleAbs(orient, adjMap, 255 / (2*M_PI));
      Mat falseColorsMap;
      applyColorMap(adjMap, falseColorsMap, cv::COLORMAP_AUTUMN);
      imshow("Out", falseColorsMap);
    }
    else if(operation==NMS){
      image=imread(frame_name,CV_8UC1);
      bilateralFilter(image,resultImage,3,5,5);
      Mat magn;
      Mat orient;
      sobel3x3(resultImage,magn,orient);
      findPeaks(magn,orient,resultImage);

      resultImage.convertTo(resultImage,CV_8UC1);
      imshow("image",resultImage);
      
      namedWindow("magn",WINDOW_NORMAL);
      magn.convertTo(magn,CV_8UC1);
      imshow("magn",magn);
      waitKey();


    }
    if(operation==CANNY){
      image=imread(frame_name,CV_8UC1);
      bilateralFilter(image,resultImage,3,5,5);

      Mat out;
      Mat magn;
      Mat orient;

      sobel3x3(resultImage,magn,orient);
      findPeaks(magn,orient,resultImage);

      doubleTh(resultImage,out,20,100);

      imshow("image",out);
      namedWindow("preThresholding",WINDOW_NORMAL);

      resultImage.convertTo(resultImage,CV_8UC1);
      imshow("preThresholding",resultImage);

      waitKey();
    }

    //wait for key or timeout
    unsigned char key = cv::waitKey(args.wait_t);
    std::cout<<"key "<<int(key)<<std::endl;

    if(key == 'q')
      exit_loop = true;

  }

  return 0;
}
