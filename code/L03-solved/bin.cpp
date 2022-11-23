
		cv::Mat best_bin(image.rows,image.cols,CV_8UC1);

		int best_th;
		double best_dist=std::numeric_limits<double>::max();

		for(int t=80;t<=254;++t)
		{
			cv::Mat bin(image.rows,image.cols,CV_8UC1);

			double abovem=0;
			double belowm=0;
			int abovec=0;
			int belowc=0;
			for(int j=0;j<image.cols*image.rows;++j)
			{
				if( image.data[j*image.channels()] >= t)
				{
					abovem+=image.data[j*image.channels()];
					abovec++;
					bin.data[j]=255;
				}
				else
				{
					if(image.data[j*image.channels()]>=80)
					{
						belowm+=image.data[j*image.channels()];
						belowc++;
						bin.data[j]=0;
					}
				}
			}
			abovem/=abovec;
			belowm/=belowc;

			double sigmaa=0;
			double sigmab=0;
			for(int j=0;j<image.cols*image.rows;++j)
			{
				if( image.data[j*image.channels()] >= t)
				{
					sigmaa+=(image.data[j*image.channels()]-abovem)*(image.data[j*image.channels()]-abovem);
				}
				else
				{
					if(image.data[j*image.channels()]>=80)
						sigmab+=(image.data[j*image.channels()]-belowm)*(image.data[j*image.channels()]-belowm);
				}
			}
			double dist = sigmaa*abovec/(abovec-1) + sigmab*belowc/(belowc-1);
			if(best_dist > dist)
			{
				best_dist = dist;
				best_th = t;
				bin.copyTo(best_bin);
			}
			//std::cout<<"th "<<t<<" dist "<<dist<<std::endl;
		}

		std::cout<<"best th "<<best_th<<" best_dist "<<best_dist<<std::endl;
