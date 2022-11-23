//OpneCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//std:
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>        // std::time

//getopt()
#include <unistd.h>

struct ArgumentList {
  std::string image_name;                   //!< image file name
  int m_padding;                            //!< waiting time
  int m_wait;                               //!< waiting time
  int u_tl;
  int v_tl;
  int width_crop;
  int height_crop;
  unsigned int ex;
  bool random_crop;
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;
  args.m_wait=0;
  args.m_padding=5;
  args.u_tl=100;
  args.v_tl=100;
  args.width_crop=100;
  args.height_crop=100;
  args.random_crop=false;
  args.ex=0xFFFFFFFFu;

  while ((c = getopt (argc, argv, "hi:t:p:u:v:W:H:rx:")) != -1)
    switch (c)
    {
      case 'i':
	args.image_name = optarg;
	break;
      case 'p':
	args.m_padding = atoi(optarg);
	break;
      case 'u':
	args.u_tl = atoi(optarg);
	break;
      case 'v':
	args.v_tl = atoi(optarg);
	break;
      case 'H':
	args.height_crop = atoi(optarg);
	break;
      case 'W':
	args.width_crop = atoi(optarg);
	break;
      case 'r':
	args.random_crop = true;
	break;
      case 'x':
	args.ex = (1<<atoi(optarg));
	break;
      case 't':
	args.m_wait = atoi(optarg);
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<
	  "   -p arg                   padding size (default 1)"<<std::endl<<
	  "   -u arg                   crop column (default 100)"<<std::endl<<
	  "   -v arg                   crop row (default 100)"<<std::endl<<
	  "   -W arg                   crop width (default 100)"<<std::endl<<
	  "   -H arg                   crop height (default 100)"<<std::endl<<
	  "   -r                       random crop"<<std::endl<<
	  "   -x arg                   exercises (default all)"<<std::endl<<
	  "   -t arg                   wait before next frame (ms) [default = 0]"<<std::endl<<std::endl<<std::endl;
	return false;
    }
  return true;
}

unsigned char openandwait(const char *windowname, cv::Mat &img, const bool sera=true)
{
    cv::namedWindow(windowname, cv::WINDOW_AUTOSIZE); // cv::WINDOW_NORMAL
    cv::imshow(windowname, img);
    unsigned char key=cv::waitKey();
    if(key=='q')
      exit(EXIT_SUCCESS);
    if(sera)
      cv::destroyWindow(windowname);
    return key;
}

int main(int argc, char **argv)
{
  int frame_number = 0;
  char frame_name[256];
  bool exit_loop = false;
  unsigned char key;

  std::srand(time(NULL));

  std::cout<<"L01"<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
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

    cv::Mat image;
    // Non c'e' modo di distinguere un'immagine Bayer da una generica immagine in bianco e nero
    //
    // L'ideale sarebbe aggiungere nell'header una indicazione in merito
    //
    // Nel nostro caso ci limitiamo a controllare il nome de file
    if(args.image_name.find("RGGB")!=std::string::npos || args.image_name.find("GBRG")!=std::string::npos || args.image_name.find("BGGR")!=std::string::npos)
      image = cv::imread(frame_name, cv::IMREAD_GRAYSCALE);
    else
      image = cv::imread(frame_name);

    if(image.empty())
    {
      std::cout<<"Unable to open "<<frame_name<<std::endl;
      return 1;
    }

    openandwait("Original Image", image, false);


    //////////////////////
    //processing code here
    if(args.ex&(1<<2))
    {

      //ES2 DOWNSAMPLE 2x
      cv::Mat out(image.rows/2,image.cols/2,image.type());
      for(int r=0;r<out.rows;++r)
      {
	for(int c=0;c<out.cols;++c)
	{
	  for(int k=0;k<out.channels();++k)
	  {
	    out.data[((r*out.cols+c)*out.elemSize() + k*out.elemSize1())] = image.data[((r*image.cols+c)*image.elemSize()*2 + k*image.elemSize1())];
	  }
	}
      }

      openandwait("downsample 2x", out);
    }


    //ES3 DOWNSAMPLE 2x VERT
    if(args.ex&(1<<3))
    {
    cv::Mat outv(image.rows/2,image.cols,image.type());
    for(int r=0;r<outv.rows;++r)
    {
      for(int c=0;c<outv.cols;++c)
      {
	for(int k=0;k<outv.channels();++k)
	{
	  outv.data[((r*outv.cols+c)*outv.elemSize() + k*outv.elemSize1())] = image.data[((r*image.cols*2+c)*image.elemSize() + k*image.elemSize1())];
	}
      }
    }

    openandwait("downsample 2x VERT", outv);
    }


    //ES4 DOWNSAMPLE 2x HORIZ
    if(args.ex&(1<<4))
    {
    cv::Mat outh(image.rows,image.cols/2,image.type());
    for(int r=0;r<outh.rows;++r)
    {
      for(int c=0;c<outh.cols;++c)
      {
	for(int k=0;k<outh.channels();++k)
	{
	  outh.data[((r*outh.cols+c)*outh.elemSize() + k*outh.elemSize1())] = image.data[((r*image.cols+c*2)*image.elemSize() + k*image.elemSize1())];
	}
      }
    }

    openandwait("downsample 2x HORIZ", outh);
    }

    //ES5 FILP HORIZ
    if(args.ex&(1<<5))
    {
    cv::Mat outfh(image.rows,image.cols,image.type());
    for(int r=0;r<outfh.rows;++r)
    {
      for(int c=0;c<outfh.cols;++c)
      {
	for(int k=0;k<outfh.channels();++k)
	{
	  outfh.data[((r*outfh.cols+c)*outfh.elemSize() + k*outfh.elemSize1())] = image.data[((r*image.cols+(image.cols-1-c))*image.elemSize() + k*image.elemSize1())];
	}
      }
    }

    openandwait("Horizontal FLIP", outfh);
    }

    //ES6 FILP VERT
    if(args.ex&(1<<6))
    {
    cv::Mat outfv(image.rows,image.cols,image.type());
    for(int r=0;r<outfv.rows;++r)
    {
      for(int c=0;c<outfv.cols;++c)
      {
	for(int k=0;k<outfv.channels();++k)
	{
	  outfv.data[((r*outfv.cols+c)*outfv.elemSize() + k*outfv.elemSize1())] = image.data[(((image.rows-r-1)*image.cols+c)*image.elemSize() + k*image.elemSize1())];
	}
      }
    }

    openandwait("vertical flip", outfv);
    }

    //ES7 and 7bis CROP
    if(args.ex&(1<<7))
    {
    if(args.random_crop)
    {
      args.u_tl=rand()%image.cols;
      args.v_tl=rand()%image.rows;
      args.width_crop=rand()%image.cols+1;
      args.height_crop=rand()%image.rows+1;
    }

    //cv::Mat outcrop(args.height_crop,args.width_crop,image.type());
    //outcrop.setTo(0);
    cv::Mat outcrop=cv::Mat::zeros(args.height_crop,args.width_crop,image.type());
    for(int r=0;r<outcrop.rows;++r)
    {
      for(int c=0;c<outcrop.cols;++c)
      {
	//controlliamo di essere dentro all'immagine di partenza
	if((r+args.v_tl)>=0 && (r+args.v_tl)<(image.rows-1) && (c+args.u_tl)>=0 && (c+args.u_tl)<image.cols)
	{
	  for(int k=0;k<outcrop.channels();++k)
	  {
	    outcrop.data[((r*outcrop.cols+c)*outcrop.elemSize() + k)*outcrop.elemSize1()] = image.data[(((r+args.v_tl)*image.cols+c+args.u_tl)*image.elemSize() + k)*image.elemSize1()];
	  }
	}
      }
    }


    openandwait("cropped", outcrop);
    }

    //ES8 ZERO PADDING
    if(args.ex&(1<<8))
    {
    cv::Mat outpad(image.rows + 2*args.m_padding, image.cols + 2*args.m_padding, image.type(), cv::Scalar(0, 0, 0));
    //outpad.setTo(0);

    for(int r=args.m_padding;r<(outpad.rows-args.m_padding);++r)
    {
      for(int c=args.m_padding;c<(outpad.cols-args.m_padding);++c)
      {
	for(int k=0;k<outpad.channels();++k)
	{
	  outpad.data[((r*outpad.cols+c)*outpad.elemSize() + k*outpad.elemSize1())] = image.data[(((r-args.m_padding)*image.cols+c-args.m_padding)*image.elemSize() + k*image.elemSize1())];
	}
      }
    }

    openandwait("PADDED", outpad);
    }

    //ES9 MIX BLOCKS
    if(args.ex&(1<<9))
    {
    cv::Mat outmix(image.rows,image.cols,image.type());

    //vettore che contiene la posizione top-left dei 4 blocchi che voglio mescolare
    std::vector<std::vector<int> > tlv = { {0,0}, {0,image.cols/2}, {image.rows/2,0}, {image.rows/2,image.cols/2} };

    //li mescolo (si poteva fare anche con un while e rand)
    std::random_shuffle(tlv.begin(),tlv.end());

    //ciclo sui blocchi
    for(int br=0;br<2;++br)
    {
      for(int bc=0;bc<2;++bc)
      {
	//ogni blocco e' largo cols/2 e alto rows/2, indipendentemente dal blocco
	for(int r=0;r<image.rows/2;++r)
	{
	  for(int c=0;c<image.cols/2;++c)
	  {
	    //il numero di canali e' sempre lo stesso
	    for(int k=0;k<outmix.channels();++k)
	    {
	      //dato un blocco (br,bc), questa e' la sua posizione top-left nell'immagine destinazione
	      int dest_r = br*image.rows/2;
	      int dest_c = bc*image.cols/2;

	      //dato un blocco (br,bc) dell'immagine di destinazione, questa e' la sua posizione top-left nell'immagine originale
	      int orig_r = tlv[br*2 + bc][0];
	      int orig_c = tlv[br*2 + bc][1];

	      outmix.data[(((r+dest_r)*image.cols + c + dest_c)*outmix.elemSize() + k*outmix.elemSize1())] = image.data[(((r+orig_r)*image.cols + c + orig_c)*image.elemSize() + k*outmix.elemSize1())];
	    }
	  }
	}
      }
    }

    openandwait("Mixed Blocks", outmix);
    }

    //ES10 COLOR MIX
    if((args.ex&(1<<10)) && image.type() == CV_8UC3)
    {
    //solo se a colori
      cv::Mat outmixcolor(image.rows,image.cols,image.type());
      std::vector<int> channels = {0,1,2};

      std::random_shuffle(channels.begin(),channels.end());

      for(int r=0;r<image.rows;++r)
      {
	for(int c=0;c<image.cols;++c)
	{
	  for(int k=0;k<image.channels();++k)
	  {
	    //con channels[k] vado a pescare il canale nell'immagine originale che corrispondera' al canale k nella destinazione
	    outmixcolor.data[((r*image.cols + c)*outmixcolor.elemSize() + k*outmixcolor.elemSize1())] = image.data[((r*image.cols + c)*image.elemSize() + channels[k]*image.elemSize1())];
	  }
	}
      }

      openandwait("Mixed colors", outmixcolor);
    }

    /////////////////////

    // ES11, 12, 13
    //
    // Non c'e' modo di distinguere un'immagine Bayer da una generica immagine in bianco e nero
    //
    // L'ideale sarebbe aggiungere nell'header una indicazione in merito
    //
    // Nel nostro caso ci limitiamo a controllare il nome de file
    //
    // Assumeremo quindi che le immagini Bayer siano sempre 8UC1
    if((args.ex&(1<<12)) || args.ex&(1<<11) || args.ex&(1<<13))
    {
      if(args.image_name.find("RGGB")!=std::string::npos || args.image_name.find("GBRG")!=std::string::npos || args.image_name.find("BGGR")!=std::string::npos)
      {
	//due immagini di destinazione per esercizio 11 e 12
	cv::Mat out_gray_down(image.rows/2,image.cols/2,CV_8UC1);
	cv::Mat out_gray_down2(image.rows/2,image.cols/2,CV_8UC1);

	for(int r=0;r<out_gray_down.rows;++r)
	{
	  for(int c=0;c<out_gray_down.cols;++c)
	  {
	    //mi posizione sull'angolo in alto a sinistra del blocco 2x2 nell'immagine Bayer
	    int orig_u = c*2;
	    int orig_v = r*2;

	    //leggo i 4 pixel del pattern
	    int upperleft  = image.data[orig_v*image.cols+orig_u];
	    int upperright = image.data[orig_v*image.cols+orig_u + 1];
	    int lowerleft  = image.data[(orig_v+1)*image.cols+orig_u];
	    int lowerright = image.data[(orig_v+1)*image.cols+orig_u+1];

	    //in base al particolare pattenr, compongo il tono di grigio
	    if(args.image_name.find("RGGB")!=std::string::npos)
	    {
	      //ES11
	      out_gray_down.data[(r*out_gray_down.cols+c)]   = (upperright+lowerleft)/2;
	      //ES12
	      out_gray_down2.data[(r*out_gray_down.cols+c)] = 0.3*float(upperleft) + 0.59*float(upperright+lowerleft)/2.0 + 0.11*float(lowerright);
	    }

	    if(args.image_name.find("GBRG")!=std::string::npos)
	    {
	      //ES11
	      out_gray_down.data[(r*out_gray_down.cols+c)]   = (upperleft+lowerright)/2;
	      //ES12
	      out_gray_down2.data[(r*out_gray_down.cols+c)] = 0.3*float(lowerleft) + 0.59*float(upperleft+lowerright)/2.0 + 0.11*float(upperright);
	    }

	    if(args.image_name.find("BGGR")!=std::string::npos)
	    {
	      //ES11
	      out_gray_down.data[(r*out_gray_down.cols+c)]   = (upperright+lowerleft)/2;
	      //ES12
	      out_gray_down2.data[(r*out_gray_down.cols+c)] = 0.3*float(lowerright) + 0.59*float(upperright+lowerleft)/2.0 + 0.11*float(upperleft);
	    }
	  }
	}

	openandwait("Downsample debayer", out_gray_down, false);
	openandwait("Luminance debayer", out_gray_down2, false);

      //ES13
      //
      //immagine di destinazione
      //questa sara' un'immagine a colori RGB, stessa dimensione di quella in ingresso
	cv::Mat out_color(image.rows,image.cols,CV_8UC3);
	for(int r=0;r<out_color.rows;++r)
	{
	  for(int c=0;c<out_color.cols;++c)
	  {
	    //mi posizione sull'angolo in alto a sinistra del blocco 2x2 nell'immagine Bayer
	    int orig_u = c;
	    int orig_v = r;

	    //leggo i 4 pixel del pattern
	    int upperleft  = image.data[orig_v*image.cols+orig_u];
	    int upperright = image.data[orig_v*image.cols+orig_u + 1];
	    int lowerleft  = image.data[(orig_v+1)*image.cols+orig_u];
	    int lowerright = image.data[(orig_v+1)*image.cols+orig_u+1];

	    // in questo caso ci spostiamo di una riga e una colonna alla volta
	    // il pattern cambia continuamente!
	    //
	    // il particolare pattern su cui mi trovo dipende dalla struttura del file originale e dalla riga e colonna corrente
	    //
	    //
	    // ES. RGGB
	    //
	    // R G R G R G
	    // G B G B G B
	    // R G R G R G
	    // G B G B G B
	    //
	    // r,c=0,0 -> RGGB
	    // r,c=1,0 -> GBRG
	    // r,c=0,1 -> GRBG
	    // r,c=1,1 -> BGGR
	    //
	    // ecc. ecc.


	    bool isRGGB = args.image_name.find("RGGB")!=std::string::npos;
	    bool isGBRG = args.image_name.find("GBRG")!=std::string::npos;
	    bool isBGGR = args.image_name.find("BGGR")!=std::string::npos;

	    //RGGB
	    if((isRGGB && r%2==0 && c%2==0) || (isGBRG && r%2==1 && c%2==0) || (isBGGR && r%2==1 && c%2==1))
	    {
	      out_color.data[(r*out_color.cols+c)*3]   = lowerright;
	      out_color.data[(r*out_color.cols+c)*3+1] = (upperright+lowerleft)/2;
	      out_color.data[(r*out_color.cols+c)*3+2] = upperleft;
	    }

	    //GBRG
	    if((isRGGB && r%2==1 && c%2==0) || (isGBRG && r%2==0 && c%2==0) || (isBGGR && r%2==0 && c%2==1))
	    {
	      out_color.data[(r*out_color.cols+c)*3]   = upperright;
	      out_color.data[(r*out_color.cols+c)*3+1] = (upperleft+lowerright)/2;
	      out_color.data[(r*out_color.cols+c)*3+2] = lowerleft;
	    }

	    //BGGR
	    if((isRGGB && r%2==1 && c%2==1) || (isGBRG && r%2==0 && c%2==1) || (isBGGR && r%2==0 && c%2==0))
	    {
	      out_color.data[(r*out_color.cols+c)*3]   = upperleft;
	      out_color.data[(r*out_color.cols+c)*3+1] = (upperright+lowerleft)/2;
	      out_color.data[(r*out_color.cols+c)*3+2] = lowerright;
	    }

	    //GRBG
	    if((isRGGB && r%2==0 && c%2==1) || (isGBRG && r%2==1 && c%2==1) || (isBGGR && r%2==1 && c%2==0))
	    {
	      out_color.data[(r*out_color.cols+c)*3]   = lowerleft;
	      out_color.data[(r*out_color.cols+c)*3+1] = (upperleft+lowerright)/2;
	      out_color.data[(r*out_color.cols+c)*3+2] = upperright;
	    }
	  }
	}

	openandwait("SIMPLE debayer", out_color, false);

	//ES13bis
	//
	//immagine di destinazione
	//questa sara' un'immagine a colori RGB, downsample 2x
	cv::Mat out_color_down(image.rows/2,image.cols/2,CV_8UC3);
	for(int r=0;r<out_color_down.rows;++r)
	{
	  for(int c=0;c<out_color_down.cols;++c)
	  {
	    //mi posizione sull'angolo in alto a sinistra del blocco 2x2 nell'immagine Bayer
	    int orig_u = c*2;
	    int orig_v = r*2;

	    //leggo i 4 pixel del pattern
	    int upperleft  = image.data[orig_v*image.cols+orig_u];
	    int upperright = image.data[orig_v*image.cols+orig_u + 1];
	    int lowerleft  = image.data[(orig_v+1)*image.cols+orig_u];
	    int lowerright = image.data[(orig_v+1)*image.cols+orig_u+1];

	    // in questo caso ci spostiamo di 2 riga e 2 colonna alla volta
	    // il pattern resta lo stesso!

	    //in base la particolare pattern, estraggo R,B e G come media dei verdi
	    if(args.image_name.find("RGGB")!=std::string::npos)
	    {
	      out_color_down.data[(r*out_color_down.cols+c)*3]   = lowerright;
	      out_color_down.data[(r*out_color_down.cols+c)*3+1] = (upperright+lowerleft)/2;
	      out_color_down.data[(r*out_color_down.cols+c)*3+2] = upperleft;
	    }

	    if(args.image_name.find("GBRG")!=std::string::npos)
	    {
	      out_color_down.data[(r*out_color_down.cols+c)*3]   = upperright;
	      out_color_down.data[(r*out_color_down.cols+c)*3+1] = (upperleft+lowerright)/2;
	      out_color_down.data[(r*out_color_down.cols+c)*3+2] = lowerleft;
	    }

	    if(args.image_name.find("BGGR")!=std::string::npos)
	    {
	      out_color_down.data[(r*out_color_down.cols+c)*3]   = upperleft;
	      out_color_down.data[(r*out_color_down.cols+c)*3+1] = (upperright+lowerleft)/2;
	      out_color_down.data[(r*out_color_down.cols+c)*3+2] = lowerright;
	    }
	  }
	}

	openandwait("SIMPLE debayer", out_color, false);
	openandwait("DOWN debayer", out_color_down, false);
      }
    }




    //wait for key or timeout
    key = cv::waitKey(args.m_wait);
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
