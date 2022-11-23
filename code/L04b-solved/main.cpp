// std
#include <iostream>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui.hpp>

// eigen
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

// utils
#include "utils.h"


// 
using namespace std;
using namespace Eigen;


int main(int argc, char **argv) {
  double k;

  if (argc < 3) 
  {
    std::cerr << "Usage lab5_2 <image_filename> <camera_params_filename>" << std::endl; 
    return 0;
  }
  if(argc==4)
  {
    k=atof(argv[3]);
  }
  else
  {
    k=50.0;
  }

  // output image
  cv::Mat ipm_image(cv::Size(400, 400), CV_8UC3, cv::Scalar(0, 0, 0));

  // load image from file
  cv::Mat input;
  input = cv::imread(argv[1]);

  cv::namedWindow("Input", cv::WINDOW_AUTOSIZE );
  imshow("Input", input);
  cv::waitKey(10);

  // load camera params
  CameraParams params;
  LoadCameraParams(argv[2], params);

  Eigen::Matrix<float, 3, 4> K;
  K << params.ku,       0.0, params.u0, 0.0,
    0.0, params.kv, params.v0, 0.0,
    0.0,       0.0,       1.0, 0.0;

  Eigen::Matrix<float, 4, 4> RT;
  cv::Affine3f RT_inv = params.RT.inv();
  RT << RT_inv.matrix(0,0), RT_inv.matrix(0,1), RT_inv.matrix(0,2), RT_inv.matrix(0,3),
     RT_inv.matrix(1,0), RT_inv.matrix(1,1), RT_inv.matrix(1,2), RT_inv.matrix(1,3),
     RT_inv.matrix(2,0), RT_inv.matrix(2,1), RT_inv.matrix(2,2), RT_inv.matrix(2,3),
     0,                  0,                  0,                  1;

  /*
   * YOUR CODE HERE: Choose a planar constraint and realize an inverse perspective mapping
   * 1) compute the perspective matrix P
   * 2) choose a planar constraint (ex. y=0) to be used during the inverse perspective mapping
   * 3) compute the inverse perspective matrix corresponding to the chosen constraint
   * 4) realize the inverse perspective mapping
   */ 

  //
  //
  // Voglio trovare la trasformazione inversa che porta da pixel (u,v) al corrispondente X,Y,Z assumendo che Y=0
  //
  //

  // Iniziamo calcolando la matrice M
  Eigen::Matrix<float, 3, 4> M;
  M = K*RT;

  // Quando trasformo da mondo (X,0,Z,1) a (u,v,w) tramite M vediamo che la seconda colonna e' irrilevante (Y=0)
  Eigen::Matrix3f tmp;
  tmp <<  M(0,0), M(0,2), M(0,3),
      M(1,0), M(1,2), M(1,3),
      M(2,0), M(2,2), M(2,3);

  // tmp e' quindi la matrice che porta da (X,Z,1) a (u,v,w), ed e' quella che dobbiamo invertire per fare il percorso contrario
  // (u,v,1) -> (X,Z,W)
  Eigen::Matrix3f IPM;
  IPM = tmp.inverse();

  // creiamo ora un'immagine che rappresenta una mappa sul piano y=0
  //
  //
  //
  //        --> colonne
  //       |
  //       |      ---------------------------
  //       V      |                         |
  //      righe   |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |                         |
  //              |            ^ Z          |
  //              |            |            |
  //              |            |            |
  //              |            - - > X      |
  //              ---------------------------
  //
  // c'e' una corrispondenza diretta tra colonne/X e righe/Z, a meno di un fatto di scala k e di un offset


  for(int ii = 0; ii < input.rows; ii++)
  {
    for(int jj = 0; jj < input.cols; jj++)
    {
      double w = (IPM(2,0)*jj+IPM(2,1)*ii+IPM(2,2));		    //omogenea mondo
      double x = (IPM(0,0)*jj+IPM(0,1)*ii+IPM(0,2))/w;		//x mondo
      double z = (IPM(1,0)*jj+IPM(1,1)*ii+IPM(1,2))/w;		//z mondo

      //fattore di scala a piacere
      x*=k;
      z*=k;

      // offset delle origini dei due sistemi di riferimento
      double z_ipm = ipm_image.rows - z;
      double x_ipm = ipm_image.cols/2 + x;

      //Creazione dell'immagine
      if(z_ipm >=0 && z_ipm < 400 && x_ipm >=0 && x_ipm < 400)
      {
	for(int kk = 0; kk < ipm_image.channels(); kk++)
	{
	  ipm_image.data[((int)x_ipm+(int)z_ipm*ipm_image.cols)*ipm_image.elemSize()+kk]=input.data[(jj+ii*input.cols)*input.elemSize()+kk];
	}
      }	
    }
  }

  cv::namedWindow("IPM", cv::WINDOW_AUTOSIZE );
  imshow("IPM", ipm_image);


  // Qui faccio l'operazione inversa ovvero dal punto mondo capisco dove si proietta e pesco i valori del pixel

  cv::Mat ipm_densa(cv::Size(400, 400), CV_8UC3, cv::Scalar(0, 0, 0));
  for(int ii = 0; ii < ipm_densa.rows; ii++)
  {
    double z_off=ipm_densa.rows;
    double x_off=ipm_densa.rows/2;
    for(int jj = 0; jj < ipm_densa.cols; jj++)
    {
       Eigen::Vector4f world_point;
       world_point.x() = jj - x_off;
       world_point.y() = 0.0;
       world_point.z() = z_off - ii;
       world_point.w() = k;

       Eigen::Vector3f img_point;
       img_point = M * world_point;
       int u = img_point.x()/img_point.z();
       int v = img_point.y()/img_point.z();
       if(u>=0 && u<input.cols && v>=0 && v<input.rows)
       {
	for(int kk = 0; kk < ipm_densa.channels(); kk++)
	 ipm_densa.data[(jj+ii*ipm_densa.cols)*ipm_densa.elemSize()+kk]=input.data[(u+v*input.cols)*input.elemSize()+kk];
       }
    }
  }
  cv::namedWindow("IPM DENSA", cv::WINDOW_AUTOSIZE );
  imshow("IPM DENSA", ipm_densa);




  std::cout << "Press any key to quit" << std::endl;

  cv::waitKey(0);

  return 0;
}
