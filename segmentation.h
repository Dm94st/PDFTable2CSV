#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "settings.h"
#include "cellCsv.hpp"
#include "ocr.h"

#include <regex>
#include <codecvt>
#include <locale>

#include <iostream>
#include <iomanip>

#include "converter.h"

#include <iostream>
#include <string>
#include <vector>

using namespace settings;

class Segmentation
{
public:
  Segmentation();

  void preProcess()
  {
    cv::Mat blobBox;
    cv::RotatedRect rotRect;

    cv::Mat inputImage = GetImage();

    cv::Mat croppedImage = ResizeAndCropImage(inputImage);

    cv::Mat contrastImage;
    cv::Mat sharpnessImage;

    std::vector<std::vector<cv::Rect>> groupedBoundingRects;

    ContrastInc(croppedImage, contrastImage);
    SharpnessInc(contrastImage, sharpnessImage);

    cv::Mat imProc = sharpnessImage.clone();

    CleanStamp(sharpnessImage);

    GrayScale(imProc);
    GaussianBlur(imProc, GausW, GausH);
    AdaptiveThreshold(imProc);

    cv::Mat horLines, verLines;
    horLines = ErodeImage(imProc.clone(), cv::MORPH_RECT, 27, 1); //27, 1

    verLines = ErodeImage(imProc.clone(), cv::MORPH_RECT, 1, 38); //1, 20
    verLines = DilateImage(verLines, cv::MORPH_RECT, 2, 32); //2, 17

    DeskewImage(sharpnessImage, horLines);
    DeskewImage(imProc, horLines);
    DeskewImage(verLines, horLines);
    DeskewImage(croppedImage, horLines);
    DeskewImage(horLines, horLines);

    FindBiggestBlob(imProc.clone(), blobBox, cv::MORPH_RECT, 3, 3);
    RectAroundBiggestBlob(blobBox, rotRect);

    // Define array for y - coordinates of horizontal lines
    std::vector<int> yCoords = CalulateProjection(horLines, SET_HORIZONTAL);

    groupedBoundingRects = DrawBorders(croppedImage, sharpnessImage, rotRect, yCoords, verLines);
    WriteResult(croppedImage, sharpnessImage, groupedBoundingRects);
  }

private:
  virtual cv::Mat GetImage() = 0;

  cv::Mat ResizeAndCropImage(cv::Mat &inputImage, bool showStep = false);
  void GrayScale(cv::Mat &inputImage, bool showStep = false);
  void GaussianBlur(cv::Mat &inputImage, int W, int H,  bool showStep = false);
  void AdaptiveThreshold(cv::Mat &inputImage, bool showStep = false);

  std::vector<int> CalulateProjection(const cv::Mat &lines, int method, bool showStep = false);
  std::vector<std::vector<cv::Rect>> DrawBorders(cv::Mat &srcImage, cv::Mat &inputImage, \
                                                 const cv::RotatedRect &blobBox, const std::vector<int> yCoords, const cv::Mat &mask, bool showStep = false);
  void RectAroundBiggestBlob(const cv::Mat &biggestBlob, cv::RotatedRect &rotRect, bool showStep = false);

  void WriteResult(cv::Mat &srcImage, cv::Mat &inputImage, const std::vector<std::vector<cv::Rect>> &groupedRect);
  void ShowResult(){} // TODO

  /*Helper functions*/
  cv::Mat SetKernel(int morphShape, int w, int h);
  cv::Mat ErodeImage(const cv::Mat &inputImage, int morphShape, int kerW, int kerH, bool showStep = false);
  cv::Mat DilateImage(const cv::Mat &inputImage, int morphShape, int kerW, int kerH, bool showStep = false);
  int CountWhite(const cv::Mat &inputImage);

  std::vector<std::vector<cv::Rect>> GroupCells(const std::vector<cv::Rect> &rects, bool showStep = false);

  void SortCells(std::vector<cv::Rect> &cells, bool showStep = false);

  void FindBiggestBlob(cv::Mat inputImage, cv::Mat &biggestBlob, int morphShape, int kerW, int kerH, bool showStep = false);

  void CleanStamp(cv::Mat &inputImage, bool showStep = false);

  void DeskewImage(cv::Mat &inputImage, const cv::Mat &mask, bool showStep = false);

  void ContrastInc(const cv::Mat &inputImage, cv::Mat &outputImage, bool showStep = false);

  void SharpnessInc(const cv::Mat &inputImage, cv::Mat &outputImage, bool showStep = false);

  void DrawRect(cv::Mat inputImage, const cv::RotatedRect & rotRect);

};

#endif // SEGMENTATION_H
