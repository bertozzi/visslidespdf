void SAD_Disparity(const cv::Mat& left, const cv::Mat& right, unsigned short wSize, cv::Mat& output){
  output = cv::Mat::zeros(left.rows, left.cols, CV_8UC1);
  cv::Mat bLeft, bRight;
  unsigned short offset = wSize/2;
  cv::copyMakeBorder(right, bRight, offset, offset, offset, offset, cv::BORDER_CONSTANT, 0);
  cv::copyMakeBorder(left, bLeft, offset, offset, offset, offset, cv::BORDER_CONSTANT, 0);
  for(int r = 0; r < left.rows; r++){
    for(int c = 0; c < left.cols; c++){
      cv::Rect lRect(c, r, wSize, wSize);
      cv::Mat wLeft = bLeft(lRect);
      double bestSum = std::numeric_limits<double>::max();
      int d;
      for(int slide = std::max(0, c - 127); slide <= c; slide++){
        cv::Rect rRect(slide, r, wSize, wSize);
        cv::Mat wRight = bRight(rRect);
        double sum = 0;
        for(int e = 0; e < wSize; e++){
          for(int f = 0; f < wSize; f++){
            sum += sqrt(pow(wLeft.at<uint8_t>(e,f) - wRight.at<uint8_t>(e,f), 2));
          }
        }
        if(sum < bestSum){
          bestSum = sum;
          d = sqrt(pow(c - slide, 2));
        }
      }
      output.at<uint8_t>(r, c) = d;
    }
  }
}

