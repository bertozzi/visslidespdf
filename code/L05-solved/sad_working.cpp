void SAD_Disparity(const cv::Mat &l, const cv::Mat &r, unsigned short w_size, cv::Mat &out)
{
  out = cv::Mat::zeros(l.size(), CV_8UC1);
  int minSAD, SAD;
  int d, c;

  for (int v = w_size/2; v < (l.rows - w_size/2); ++v)
  {
    for (int u = w_size/2; u < (l.cols - w_size/2); ++u)
    {
      c = u;
      int row, col_l, col_r;
      for (int i = 0; i < MAX_DISPARITY && (c - i) > 1; i++)
      {
        SAD = 0;
        for (int wrows = -w_size/2; wrows <= w_size / 2; ++wrows)
        {
          for (int wcols = -w_size/2; wcols <= w_size / 2; ++wcols)
          {
            row = v + wrows;
            col_l = c + wcols;
            col_r = c - i+ wcols;
            SAD += abs(l.data[col_l + row * l.cols] - r.data[col_r + row * r.cols]);
          }
        }
        if (i == 0 || SAD < minSAD)
        {
          minSAD = SAD;
          d = i;
        }
      }
      out.data[u + v * l.cols] = d;
    }
  }
}
