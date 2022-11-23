//OpenCV
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
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {
  int c;

  while ((c = getopt (argc, argv, "hi:")) != -1)
    switch (c)
    {
      case 'i':
	args.image_name = optarg;
	break;
      case 'h':
      default:
	std::cout<<"usage: " << argv[0] << " -i <image_name>"<<std::endl;
	std::cout<<"exit:  type q"<<std::endl<<std::endl;
	std::cout<<"Allowed options:"<<std::endl<<
	  "   -h                       produce help message"<<std::endl<<
	  "   -i arg                   image name. Use %0xd format for multiple images."<<std::endl<<std::endl;
	return false;
    }
  return true;
}

//erosion con elemento strutturante sempre in formato matriciale
void erosion(const cv::Mat & image, const cv::Mat & stru, int uanchor, int vanchor, cv::Mat & out)
{
  if(image.type()!=CV_8UC1)
  {
    std::cerr<<"erosion: input image must be 8UC1"<<std::endl;
    exit(1);
  }

  out = cv::Mat(image.rows,image.cols,CV_8UC1);
  image.copyTo(out);

  //per semplicita' applichiamo il filtro solo dove si sovrappone completamente all'immagine
  for(int r=0;r<image.rows-stru.rows;++r)
  {
    for(int c=0;c<image.cols-stru.cols;++c)
    {
      //nel pixel corrispondente al punto principale dell'elemento strutturante abbiamo un 1?
      if(image.data[(r+vanchor)*image.cols + c + uanchor] > 0 )
      {
	//moltiplico pixel per pixel l'elemento strutturante con l'immagine
	//
	//cv::Rect permette di accedere ad una sottomatrice posizionata in (c,r) di dimensione stru.cols, stru.rows
	cv::Mat intersection = stru.mul(image(cv::Rect(c,r,stru.cols, stru.rows)));

	//verifico se il numero di 1 sia lo stesso dell'elemento strutturante
	if(cv::countNonZero(intersection) < cv::countNonZero(stru))
	  out.data[(r+vanchor)*image.cols + c + uanchor]=0; //se cosi' non e', annullo

	//
	// codice equivalente
	//
	//				bool exitf = false;
	//				//allora controlliamo anche gli altri pixel
	//				for(int rs=0;rs<stru.rows && !exitf;++rs)
	//				{
	//					for(int cs=0;cs<stru.cols && !exitf;++cs)
	//					{
	//						//verifico che tutti gli 1 dell'elemento strutturante corrispondanto ad altrettanti 1 nell'immagine
	//						if(stru.data[rs*stru.cols + cs]>0 && image.data[(r+rs)*image.cols + c + cs]==0)
	//						{
	//							//se non e' cosi' metto a 0 il punto principale ed esco
	//							out.data[(r+vanchor)*image.cols + c + uanchor]=0;
	//							exitf=true;
	//						}
	//					}
	//				}
      }
    }
  }
}

//dilation con elemento strutturante sempre in formato matriciale
void dilation(const cv::Mat & image, const cv::Mat & stru, int uanchor, int vanchor, cv::Mat & out)
{
  if(image.type()!=CV_8UC1)
  {
    std::cerr<<"erosion: input image must be 8UC1"<<std::endl;
    exit(1);
  }

  out = cv::Mat(image.rows,image.cols,CV_8UC1);
  image.copyTo(out);

  //per semplicita' applichiamo il filtro solo dove si sovrappone completamente all'immagine
  for(int r=0;r<image.rows-stru.rows;++r)
  {
    for(int c=0;c<image.cols-stru.cols;++c)
    {
      //nel pixel corrispondente al punto principale dell'elemento strutturante abbiamo un 1?
      if(image.data[(r+vanchor)*image.cols + c + uanchor] > 0 )
      {
	//metto a 1 tutti i pixel che sono ad 1 anche nell'elemento strutturante

	//prendo il massimo tra stru e la corrispondente finestra sull'immagine
	cv::Mat unionm = cv::max(stru, out(cv::Rect(c,r,stru.cols, stru.rows)));

	//copio in destinazione
	unionm.copyTo(out(cv::Rect(c,r,stru.cols, stru.rows)));

	//
	// codice equivalente
	//
	//mettiamo a 1 tutti i vicini che corrispondono ad un 1 nell'elemento strutturante
	//				for(int rs=0;rs<stru.rows;++rs)
	//				{
	//					for(int cs=0;cs<stru.cols;++cs)
	//					{
	//						if(stru.data[rs*stru.cols + cs]>0)
	//							out.data[(r+rs)*image.cols + c + cs]=255;
	//
	//					}
	//				}
      }
    }
  }
}


int main(int argc, char **argv)
{
  char frame_name[256];

  std::cout<<"Simple program."<<std::endl;

  //////////////////////
  //parse argument list:
  //////////////////////
  ArgumentList args;
  if(!ParseInputs(args, argc, argv)) {
    return 1;
  }

  //generating file name
  sprintf(frame_name,"%s",args.image_name.c_str());

  //opening file
  std::cout<<"Opening "<<frame_name<<std::endl;

  //Per questo esercizione l'immagine e' sicuramente a toni di grigio
  cv::Mat image = cv::imread(frame_name,CV_8UC1);

  if(image.empty())
  {
    std::cout<<"Unable to open "<<frame_name<<std::endl;
    return 1;
  }

  //////////////////////
  //processing code here

  //ES1 Soglia adattiva
  //
  //immagine di uscita
  cv::Mat best_bin(image.rows,image.cols,CV_8UC1);

  int best_th=0;
  double best_dist=std::numeric_limits<double>::max();

  //
  // Come detto l'immagine non e' perfettamente bimodale
  //
  // Esistono almeno 3 livelli: lo sfondo nero, gli organi e i non organi
  //
  // In realta' nemmeno lo sfondo e' perfettamente nero, ma contiene vari toni di grigio vicini al nero
  //
  // In questa situazione il metodo presentato trovera' una soglia non ideale, ma possiamo mitigare il problema
  //
  // Assumiamo di sapere che lo sfondo nero in realta' sia formato da pixel nel range [0-50], ed escludiamoli dal calcolo della soglia
  //
  //

  //livello di grigio che assumiamo sia il massimo nello sfondo.
  int max_sfondo = 50;

  for(int t=max_sfondo;t<=255;++t)
  {
    cv::Mat bin(image.rows,image.cols,CV_8UC1);

    //media dei pixel sopra soglia
    double abovem=0;
    //media dei pixel sotto soglia
    double belowm=0;
    //numero di pixel sopra soglia
    int abovec=0;
    //numero di pixel sotto soglia
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
	//escludo dal calcolo delle statistiche i punti che assumo facciano parte dell sfondo
	if(image.data[j*image.channels()]>=max_sfondo)
	{
	  belowm+=image.data[j*image.channels()];
	  belowc++;
	  bin.data[j]=0;
	}
      }
    }
    //calcolo le medie
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
	//escludo dal calcolo delle statistiche i punti che assumo facciano parte dell sfondo
	if(image.data[j*image.channels()]>=max_sfondo)
	  sigmab+=(image.data[j*image.channels()]-belowm)*(image.data[j*image.channels()]-belowm);
      }
    }
    //sigma2*pixel_above + sigma2*pixel_below
    double dist = sigmaa + sigmab;

    //aggiorno la soglia migliore
    if(best_dist > dist)
    {
      best_dist = dist;
      best_th = t;
      //mi salvo l'attuale immagine binarizzata
      bin.copyTo(best_bin);
    }
  }
  std::cout<<"best th "<<best_th<<" best_dist "<<best_dist<<std::endl;
  std::cout<<"La soglia trovata non e' effettivamente la migliore possibile, a casua della non bimodalita' dell'immagine"<<std::endl;

  //ES3 Morfologia Matematica
  //

  //immagini di uscita con elemento strutturante 3x3
  cv::Mat eroded3;
  cv::Mat dilated3;
  cv::Mat closed3;
  cv::Mat opened3;

  //elemento strutturante 3x3
  //
  //   1  1  1
  //   1  1* 1
  //   1  1  1

  // punto principale (2,2)
  cv::Mat estru3(3,3,CV_8UC1);
  estru3.setTo(255);

  erosion(best_bin, estru3, 1,1, eroded3);
  dilation(best_bin, estru3, 1,1, dilated3);

  //opening = erosion+dilation
  dilation(eroded3, estru3, 1,1, opened3);
  //closing = dilation+erosion
  erosion(dilated3, estru3, 1,1, closed3);


  //immagini di uscita con elemento strutturante 7x7
  cv::Mat eroded7;
  cv::Mat dilated7;
  cv::Mat closed7;
  cv::Mat opened7;

  //elemento strutturante 7x7
  //
  //   1  1  1  1  1  1  1
  //   1  1  1  1  1  1  1
  //   1  1  1  1  1  1  1
  //   1  1  1  1* 1  1  1
  //   1  1  1  1  1  1  1
  //   1  1  1  1  1  1  1
  //   1  1  1  1  1  1  1

  // punto principale (3,3)
  cv::Mat estru7(7,7,CV_8UC1);
  estru7.setTo(255);

  erosion(best_bin, estru7, 3,3, eroded7);
  dilation(best_bin, estru7, 3,3, dilated7);

  //opening = erosion+dilation
  dilation(eroded7, estru7, 3,3, opened7);
  //closing = dilation+erosion
  erosion(dilated7, estru7, 3,3, closed7);

  cv::namedWindow("best_bin", cv::WINDOW_AUTOSIZE);
  cv::imshow("best_bin", best_bin);

  cv::namedWindow("erosion3x3", cv::WINDOW_AUTOSIZE);
  cv::imshow("erosion3x3", eroded3);

  cv::namedWindow("dilation3x3", cv::WINDOW_AUTOSIZE);
  cv::imshow("dilation3x3", dilated3);

  cv::namedWindow("closing3x3", cv::WINDOW_AUTOSIZE);
  cv::imshow("closing3x3", closed3);

  cv::namedWindow("opening3x3", cv::WINDOW_AUTOSIZE);
  cv::imshow("opening3x3", opened3);

  cv::namedWindow("erosion7x7", cv::WINDOW_AUTOSIZE);
  cv::imshow("erosion7x7", eroded7);

  cv::namedWindow("dilation7x7", cv::WINDOW_AUTOSIZE);
  cv::imshow("dilation7x7", dilated7);

  cv::namedWindow("closing7x7", cv::WINDOW_AUTOSIZE);
  cv::imshow("closing7x7", closed7);

  cv::namedWindow("opening7x7", cv::WINDOW_AUTOSIZE);
  cv::imshow("opening7x7", opened7);

  //wait for key
  cv::waitKey();

  return 0;
}
