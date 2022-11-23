void SAD_Disparity(const cv::Mat &l, const cv::Mat &r, unsigned short w_size, cv::Mat &out)
{
  out = cv::Mat::zeros(l.size(), CV_8UC1);
  for(int row=w_size/2; row<(l.rows-w_size/2); ++row)
  {
    for(int c=w_size/2; c<(l.cols-w_size/2); ++c)
    {
      unsigned int minSAD=UINT_MAX;
      int minSAD_d=0;
      for(int d=0;d<MAX_DISPARITY && (c-d)>1; d++)
      {
        unsigned int SAD=0;
        for(int dr=-w_size/2; dr<=w_size/2; ++dr)
        {
          for(int dc=-w_size/2; dc<=w_size/2; ++dc)
          {
            int adj_r  = row + dr;
            int adj_lc = c + dc;
            int adj_rc = c - d + dc;
            SAD += abs( l.data[(adj_r*l.cols+adj_lc)*l.elemSize1()] - r.data[(adj_r*r.cols+adj_rc)*r.elemSize1()] );
          }
        }
        if(SAD<minSAD)
        {
          minSAD=SAD;
          minSAD_d=d;
        }
      }
      out.data[(row*l.cols+c)*out.elemSize1()] = minSAD_d;
    }
  }
}
