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
  std::string image_name;		    //!< image file name
  int wait_t;                     //!< waiting time
  unsigned int running;                    //!< running average window
  unsigned int threshold;                  //!< binary threshold
  float alpha;                    //!< exponential running average weight

};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.wait_t=0;
  args.threshold=50;
  args.running=10;
  args.alpha=0.5;


  while ((c = getopt (argc, argv, "hi:t:k:w:a:")) != -1)
    switch (c)
    {
      case 'a':
	args.alpha = atof(optarg);
	break;
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


//////////////////////////////////////
void fgdiff(const cv::Mat& background, cv::Mat& fg, unsigned char threshold) {
  for(int ii = 0; ii < fg.rows; ++ii) {
    for(int jj = 0; jj < fg.cols; ++jj) {
      unsigned char& f = fg.data[(ii*fg.cols+jj)*fg.elemSize()];
      unsigned char b = background.data[(ii*background.cols+jj)*background.elemSize()];
      if(std::abs(int(f) - int(b)) <= threshold) {
	f = 0;
      }
    }
  }
}
/////////////////////////////////////

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
  cv::Mat m_pframe_bg;
  //exp average
  cv::Mat m_exp_av_bg;
  //moving average
  std::vector<cv::Mat> m_f_history;
  std::vector<cv::Mat> m_f_history_float;
  cv::Mat m_ma_bg;
  //background versione 2
  cv::Mat m_ma_bg_2;
  m_ma_bg_2 = cv::Scalar(0.0f);

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


    //////////////////////
    //processing code here
    ////BACKGROUND SUBTRACTION

    //////////////
    // PREV FRAME
    if(m_pframe_bg.empty()) {
      m_pframe_bg = image;
    } else {
      //we have a bg
      cv::Mat fg = image.clone();
      fgdiff(m_pframe_bg, fg, args.threshold);

      cv::namedWindow("PrevFrame-BG",cv::WINDOW_NORMAL);
      cv::namedWindow("PrevFrame",cv::WINDOW_NORMAL);
      cv::imshow("PrevFrame-BG", m_pframe_bg);
      cv::imshow("PrevFrame", fg);

      m_pframe_bg = image;
    }
    //////////////

    //////////////
    // EXP AVERAGE
    if(m_exp_av_bg.empty()) {
      m_exp_av_bg = image;
    } else {
      //we have a bg
      cv::Mat fg = image.clone();
      fgdiff(m_exp_av_bg, fg, args.threshold);

      cv::namedWindow("ExpAverage",cv::WINDOW_NORMAL);
      cv::namedWindow("ExpAverage-BG",cv::WINDOW_NORMAL);
      cv::imshow("ExpAverage-BG", m_exp_av_bg);
      cv::imshow("ExpAverage", fg);

      for(int ii = 0; ii < m_exp_av_bg.rows*m_exp_av_bg.cols; ++ii) {
	unsigned char& f = m_exp_av_bg.data[ii];
	unsigned char b = image.data[ii];

	f = static_cast<unsigned char>(args.alpha*f + (1-args.alpha)*b);
      }
    }
    ////////////////

    ///////////////////
    // MOVING AVERAGE
    //
    // segue lo schema proposto
    //
    if(m_f_history.size() < args.running) {
      m_f_history.push_back(image);
    } else {

      //QUESTO CONTROLLO VA BENE
      //
      //QUESTA PARTE VIENE ESEGUITA SOLO LA PRIMA VOLTA IN CUI ENTRO IN QUESTO BLOCCO, QUINDI AL FRAME args.running
      //DAI FRAME SUCCESSIVI m_ma_bg NON SARA' PIU' EMPTY, MA AGGIORNERO' IL BG COME INDICATO ALLA RIGA 220
      if(m_ma_bg.empty()) {
	m_ma_bg.create(image.rows, image.cols, CV_32FC1);
	m_ma_bg = cv::Scalar(0.0f);

	for(unsigned int ii = 0; ii < m_f_history.size(); ++ii) {
	  for(int u = 0; u < image.cols*image.rows; ++u)
	  {
	    float *bg = (float *)(m_ma_bg.data + u*m_ma_bg.elemSize());
	    float hi = m_f_history[ii].data[u];
	    *bg = *bg + hi;
	  }
	}

	m_ma_bg /= args.running;
      }

      /////////////////////////
      //questo e' necessario perche' la funzione fgdiff fornita lavora solo su immagini uchar
      //non sono necessarie riscalature particolari, essendo m_ma_bg un media di immagini uchar
      cv::Mat m_exp_av_bg_uchar(image.rows, image.cols, CV_8UC1);
      m_ma_bg.convertTo(m_exp_av_bg_uchar, CV_8UC1);
      ////////////////////////

      cv::Mat fg = image.clone();
      fgdiff(m_exp_av_bg_uchar, fg, args.threshold);

      cv::namedWindow("MovAverage",cv::WINDOW_NORMAL);
      cv::namedWindow("MovAverage-BG",cv::WINDOW_NORMAL);
      imshow("MovAverage-BG", m_exp_av_bg_uchar);
      imshow("MovAverage", fg);

      //adjust the background
      //m_ma_bg = (args.running*m_ma_bg - m_f_history.front() + image)/args.running;
      m_ma_bg *= args.running;

      for(int u = 0; u < image.cols*image.rows; ++u)
      {
	float * bg = (float *)(m_ma_bg.data + u*m_ma_bg.elemSize());
	float hi = m_f_history.front().data[u];
	*bg = *bg - hi;
      }

      for(int u = 0; u < image.cols*image.rows; ++u)
      {
	float * bg = (float *)(m_ma_bg.data + u*m_ma_bg.elemSize());
	float in = image.data[u];
	*bg = *bg + in;
      }

      m_ma_bg /= args.running;

      //adjust the buffer
      m_f_history.push_back(image);
      m_f_history.erase(m_f_history.begin());
    }
    /////////////////////

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
