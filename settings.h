#pragma once

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define RED_CV CV_RGB(255, 0, 0)
#define GREEN_CV CV_RGB(0, 255, 0)
#define WHITE_CV CV_RGB(255, 255, 255)

namespace settings
{
  // Path until PDF file
  extern std::string inPath;
  // Path until destination folder
  extern std::string outPath;
  // Language recognition
  extern const char * lang;

  /*Variables for contrast */
  const double alpha = 1.2; //[1-3]
  const int beta = -20; //[1-100]

  /*Variables for resize image*/
  const int width = 2560; //2560, 1249
  const int height = 1890; //1890, 900

  /*Variables for crop image*/
  const int xBeg = 20;
  const int yBeg = 20;
  const int xEnd = 2520; //2520, 1200
  const int yEnd = 1850; //1850, 860

  /*Variables for adaptiveThreshold image*/
  const int blockSize = 15;
  const int C = 3;

  /*Gaussian convolution kernel*/
  const int GausW = 3;
  const int GausH = 3;

  /*Minimum non - zero pixels on ROI*/
  const int nonZero = 50;

  /*Maximum gap between two lines */
  const int lineGap = 10;

  /*Gaps for ROI*/
  const int leftGap = -5; //5
  const int rightGap = -10; //10

  /*Color for horizontal lines*/
  const cv::Scalar colVer = RED_CV;
  const cv::Scalar colHor = GREEN_CV;
  const int sizeHor = 4;
  const int sizeVer = 4;

  /*Minimum area for detecting stamp*/
  const int minStampArea = 35000;

  /*Gap between cells*/
  const int cellGap = 5;

  /*DPI for extracted images from PDF*/
  const int dpi = 300;
}

enum{SET_VERTICAL, SET_HORIZONTAL};
