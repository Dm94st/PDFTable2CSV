#include "segmentation.h"

Segmentation::Segmentation()
{

}

cv::Mat Segmentation::ResizeAndCropImage(cv::Mat &inputImage, bool showStep)
{  
  cv::resize(inputImage, inputImage, inputImage.cols > inputImage.rows ? cv::Size(width, height) : cv::Size(height, width), 0, 0, cv::INTER_AREA);
  if(showStep){cv::imshow("Resized image", inputImage); cv::waitKey(0);}
  return inputImage.cols > inputImage.rows ? inputImage(cv::Rect(xBeg, yBeg, xEnd, yEnd)) : inputImage(cv::Rect(yBeg, xBeg, yEnd, xEnd));
}

void Segmentation::GrayScale(cv::Mat &inputImage, bool showStep)
{
  std::cout <<"Depth Image for grayscale "<< inputImage.depth() <<"Channels = "<< inputImage.channels() << std::endl;
  cv::cvtColor(inputImage, inputImage, cv::COLOR_BGR2GRAY);
  if(showStep){cv::imshow("GrayScale image", inputImage); cv::waitKey(0);}
}

void Segmentation::GaussianBlur(cv::Mat &inputImage, int W, int H, bool showStep)
{
  cv::GaussianBlur(inputImage, inputImage, cv::Size(W, H), 0, 0);
  if(showStep){cv::imshow("Blur image", inputImage); cv::waitKey(0);}
}

void Segmentation::AdaptiveThreshold(cv::Mat &inputImage, bool showStep)
{
  cv::adaptiveThreshold(inputImage, inputImage, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, blockSize, C);
  if(showStep)
  {
    cv::imshow("Thresholded image", inputImage); cv::waitKey(0);
  }
}

cv::Mat Segmentation::SetKernel(int morphShape, int w, int h)
{
  /*
  MORPH_RECT    = 0,
  MORPH_CROSS   = 1,
  MORPH_ELLIPSE = 2
  */

  return cv::getStructuringElement(morphShape, cv::Size(w, h));
}

cv::Mat Segmentation::ErodeImage(const cv::Mat &inputImage, int morphShape, int kerW, int kerH, bool showStep)
{
  cv::erode(inputImage, inputImage, SetKernel(morphShape, kerW, kerH));
  if(showStep){cv::imshow("Eroded image", inputImage); cv::waitKey(0);}

  return inputImage;
}

cv::Mat Segmentation::DilateImage(const cv::Mat &inputImage, int morphShape, int kerW, int kerH, bool showStep)
{
  cv::dilate(inputImage, inputImage, SetKernel(morphShape, kerW, kerH));
  if(showStep){cv::imshow("Dilated image", inputImage); cv::waitKey(0);}

  return inputImage;
}

std::vector<int> Segmentation::CalulateProjection(const cv::Mat &lines, int method, bool showStep)
{
    /*Compute projections*/
    cv::Mat1f backProj;
    cv::Mat1b hist;
    cv::Mat histVisual = cv::Mat::zeros( lines.rows, lines.rows, CV_8UC3 );
    std::vector<int> coords;

    double min;
    double max;
    float thresholdVal;
    int sum;

    try
    {
      if(lines.channels() != 1)
      {
        throw std::domain_error("Unsupported channels count");
      }
    }

    catch(std::exception const &ex)
    {
      std::cerr << ex.what() << std::endl;
    }

    cv::reduce(lines, backProj, method == 0 ? 0 : 1, CV_REDUCE_SUM);

    cv::minMaxLoc(backProj, &min, &max);

    if(showStep)
    {
      if(method == SET_HORIZONTAL)
      {
        for(auto it = backProj.begin(); it!=backProj.end(); ++it)
        {
          std::cout <<"Min = "<< min/255 << " Max = " << max/255 << std::endl;
          std::cout <<"Size of backProj = "<< backProj.cols <<" h = "<<lines.rows<< std::endl;

          cv::Point begPoint((it - backProj.begin()), backProj.rows);
          cv::Point endPoint((it - backProj.begin()), backProj.rows - *it/255);
          std::cout<<*it/255<<std::endl;
          cv::line(histVisual, begPoint, endPoint, cv::Scalar(0, 0, 255), 2);
        }
        cv::imshow("Histogram", histVisual);
        cv::waitKey(0);
      }
    }

    /* Remove noise in histogram. White bins identify space lines, black bins identify text lines */
    switch(method)
    {
    case SET_VERTICAL:
      thresholdVal = (max / (lines.rows * 255)) * 100 > 80 ? max * 0.2 : max;
      sum = lines.cols;
      break;
    case SET_HORIZONTAL:
      thresholdVal = max * 0.1;
      sum = lines.rows;
      break;
    }

    hist = backProj > thresholdVal;

    // Get mean coordinate of white white pixels groups
    int x = 0;
    int count = 0;
    bool isSpace = false;

    for (int i = 0; i <  sum; ++i)
    {
      if (!isSpace)
      {
        if (hist(i))
        {
          isSpace = true;
          count = 1;
          x = i;
        }
      }

      else
      {
        if (!hist(i))
        {
          isSpace = false;
          coords.push_back(x / count);
        }
        else
        {
          x += i;
          count++;
        }
      }
    }

    return coords;

}

void Segmentation::SortCells(std::vector<cv::Rect> &cells, bool showStep)
{
  if(!cells.empty())
  {
    std::sort(cells.begin(), cells.end(),
    [](const cv::Rect &l, const cv::Rect &r)
    {
      if(l.y == r.y)
        return l.x < r.x;
      else
        return l.y < r.y;
    });

    if(showStep)
    {
      std::cout<<std::endl;
      for(auto &rect:cells)
      {
        std::cout<<rect<<std::endl;
      }
      std::cout<<std::endl;
    }
  }
}

std::vector<std::vector<cv::Rect>> Segmentation::GroupCells(const std::vector<cv::Rect> &rects, bool showStep)
{
  std::vector<std::vector<cv::Rect>> result;
  std::vector<cv::Rect> group;
  cv::Rect prev = {0, 0, 0, 0};

  for(auto it = rects.begin(); it != rects.end(); it++)
  {
    if(!prev.y || std::abs( it->y - prev.y ) <= cellGap)
    {
      group.push_back(*it);
    }
    else
    {
      result.emplace_back(group);
      group = {*it};
    }
    prev = *it;
  }

  if(!group.empty()){result.emplace_back(group);}

  if(showStep)
  {
    for(auto i = result.begin(); i != result.end(); i++)
    {
      for(auto j = i->begin(); j < i->end(); j++)
      {
        std::cout<<*j<<' ';
      }
      std::cout<<std::endl;
    }
  }

  return result;

}

void Segmentation::WriteResult(cv::Mat &srcImage, cv::Mat &inputImage, const std::vector<std::vector<cv::Rect>> &groupedRect)
{
  try
  {
    // Initialize Tesseract-API
    OCR * ocrInit = new OCR();

    const std::string imageName = Converter::GetFilename(inPath);

    // Index of page
    static int pageNum = 0;

    // Initialize csv writer
    ccsv::cellCsv csvWriter;

    // UTF-8 <-> UTF-16 converter
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> UTF8_UTF_16_CONVERTER;

    // Cell index
    int cellIdx = 0;

    // Write png image without compression
    const std::vector<int> compressParams = {CV_IMWRITE_PNG_COMPRESSION, 0};

    // Set locale
    std::locale wcoutLoc{std::wcout.getloc(), new std::codecvt_utf8<wchar_t>{}};
    std::wcout.imbue(wcoutLoc);

    for(auto i = groupedRect.begin(); i != groupedRect.end(); i++)
    {
      int raw = i - groupedRect.begin(); // Iterator to index
      for(auto j = i->begin(); j != i->end(); j++, cellIdx++)
      {
        int col = j - i->begin(); // convert iterator to index

        if(CountWhite(inputImage(*j)) == 0) // skip full white cells
        {
          continue;
        }

        else
        {
          // Upscale image to 2x for improve quality
          cv::Mat curCell;
          cv::pyrUp(srcImage(*j), curCell, cv::Size(srcImage(*j).cols*2, srcImage(*j).rows*2));

          // Recognize text on image
          std::wstring textCell = ocrInit->extractText(curCell);

          // Clean string from special characters
          textCell = std::regex_replace(textCell, std::wregex(L"[^0-9а-яА-Я]+"),  L" ");

          // Clean string from ending spaces
          textCell = [] (const std::wstring& str) -> std::wstring
          {
            size_t first = str.find_first_not_of(' ');
            if (std::wstring::npos == first)
            {
              return str;
            }
            size_t last = str.find_last_not_of(' ');
            return str.substr(first, (last - first + 1));
          }(textCell);

          csvWriter.setCell(col, raw, UTF8_UTF_16_CONVERTER.to_bytes(textCell));
          /*std::wcout << textCell<< std::endl;*/
        }
      }
    }

    delete ocrInit; // Release memory

    ocrInit = nullptr;

    csvWriter.dump(outPath + "/" + imageName + "_" + std::to_string(pageNum) + ".csv"); // Save as csv table

    pageNum++; // Move to next page
  }

  catch (std::exception& ex)
  {
    std::cerr<<"Caught exception while write result: "<< ex.what()<< std::endl;
  }
}

std::vector<std::vector<cv::Rect>> Segmentation::DrawBorders(cv::Mat &srcImage, cv::Mat &inputImage, const cv::RotatedRect &blobBox, \
                                                             const std::vector<int> yCoords, const cv::Mat &mask, bool showStep)
{
  std::vector<std::vector<cv::Point>> contours; // Array with counters that extracted from image
  std::vector<cv::Rect> boundRectArray; // Array with bounded rects
  std::vector<std::vector<cv::Rect>> groupedRect; // Array with grouped bounded rects

  try
  {
    // Empty image for pattern
    cv::Mat patternImage = cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_8UC1);

    for(auto yIt = yCoords.begin(); yIt != yCoords.end() - 1; ++yIt)
    {
      if(*(std::next(yIt)) - *yIt >= lineGap)
      {
        // Draw horizontal lines
        cv::Rect RectROI(blobBox.boundingRect().tl().x - leftGap, *yIt, inputImage.cols - \
                        (inputImage.cols - blobBox.boundingRect().width) + rightGap, *(std::next(yIt)) - *yIt);

        cv::rectangle(inputImage, RectROI , colHor, sizeHor);
        cv::rectangle(patternImage, RectROI , WHITE_CV, sizeHor);
        cv::rectangle(srcImage, RectROI , WHITE_CV, sizeHor);

        cv::Mat matROI = mask(RectROI);
        std::vector<int> xCoords = CalulateProjection(matROI, SET_VERTICAL, 0);

        for(auto xIt = xCoords.begin(); xIt != xCoords.end(); ++xIt)
        {
          /*Draw vertical lines*/
          cv::line(inputImage(RectROI), cv::Point(*xIt, 0), cv::Point(*xIt, mask(RectROI).rows), colVer, sizeVer);
          cv::line(patternImage(RectROI), cv::Point(*xIt, 0), cv::Point(*xIt, mask(RectROI).rows), WHITE_CV, sizeVer);
          cv::line(srcImage(RectROI), cv::Point(*xIt, 0), cv::Point(*xIt, mask(RectROI).rows), WHITE_CV, sizeVer);
        }
        
        /* cv::imshow("ROI: " + std::to_string(it - yCoords.begin()), inputImage(RectROI)) */

      }
    }

    cv::findContours(patternImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // Convert contours to bounding rects
    std::for_each(contours.begin(), contours.end(), [&boundRectArray] (std::vector<cv::Point>& ctr)
    {
      cv::Rect boundRect = cv::boundingRect(ctr);
      boundRectArray.emplace_back(boundRect);
    });

    // Sort bound rects from top-left to bottom-right
    SortCells(boundRectArray, 0);

    // Erase biggest external rectangle
    boundRectArray.erase(boundRectArray.begin());

    // Skip empty cells
    for(auto rect = boundRectArray.end(); rect != boundRectArray.begin(); --rect)
    {
      if(rect->width <= 6 || rect->height <= 3)
      {
        boundRectArray.erase(--rect);
      }
    }

    // Clustering rectangles based on 'Y' position
    groupedRect = GroupCells(boundRectArray, 0);

    if(showStep)
    {
      cv::imshow("Pattern", patternImage); cv::waitKey(0);
    }
  }

  catch (cv::Exception& ex)
  {
    std::cerr<<"Caught exception while drawBorders: "<<ex.msg << std::endl;
  }

  return groupedRect;
}

int Segmentation::CountWhite(const cv::Mat &inputImage)
{
  double imgSquare = inputImage.cols * inputImage.rows;
  int countPixels = 0;

  for( int y = 0; y < inputImage.rows; y++ )
  {
    for( int x = 0; x < inputImage.cols; x++ )
    {
      cv::Vec3b bgrPixel = inputImage.at<cv::Vec3b>(y, x);
      if(bgrPixel[0] > 252 && bgrPixel[1] > 252 && bgrPixel[2] > 252)
      {countPixels++;}

    }
  }

  double pct = 100 - countPixels/imgSquare * 100;
  return pct;

}


void Segmentation::FindBiggestBlob(cv::Mat inputImage, cv::Mat &biggestBlob, int morphShape, int kerW, int kerH, bool showStep)
{
  try
  {
    int maxArea = -1;
    cv::Point maxPt; // Declare point that belongs to biggest blob

    if(inputImage.channels() != 1)
    {
      throw std::invalid_argument("Image should be binary!");
    }

    // Dilate inputImage with default kernel
    DilateImage(inputImage, morphShape, kerW, kerH, 0);

    // Find biggest blob
    for(int y = 0; y < inputImage.size().height; y++)
    {
      uchar *row = inputImage.ptr(y);
      for(int x = 0; x < inputImage.size().width; x++)
      {
        if(row[x] >= 128)
        {
          int area = cv::floodFill(inputImage, cv::Point(x, y), CV_RGB(0, 0, 64));
          if(area > maxArea)
          {
            maxPt = cv::Point(x,y);
            maxArea = area;
          }
        }
      }
    }

    if(maxArea == -1)
    {
      throw std::domain_error("Error! Area should be greater than zero!");
    }

    // Turn the other blobs black
    cv::floodFill(inputImage, maxPt, CV_RGB(255, 255, 255));
    biggestBlob = inputImage < 255;
    cv::bitwise_not(biggestBlob, biggestBlob);
  }

  catch(std::exception const &ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  if(showStep){cv::imshow("Biggest blob", biggestBlob); cv::waitKey(0);}
}

void Segmentation::CleanStamp(cv::Mat &inputImage, bool showStep)
{
  cv::Mat hsv, mask, biggestBlob;
  cv::cvtColor(inputImage, hsv, cv::COLOR_BGR2HSV);
  cv::inRange(hsv, cv::Scalar(100, 50, 50), cv::Scalar(135, 255, 255), mask);

  if(cv::countNonZero(mask) > 0)
  {
    FindBiggestBlob(mask, biggestBlob, cv::MORPH_ELLIPSE, 11, 11, 0);
  }

  else
    return;

  if(cv::countNonZero(biggestBlob) > minStampArea)
  {
    inputImage.setTo(cv::Scalar(255,255,255), biggestBlob);
    if(showStep){cv::imshow("Clean image from stamp", inputImage); cv::waitKey(0);}
  }

  else
    return;
}

void Segmentation::RectAroundBiggestBlob(const cv::Mat &biggestBlob, cv::RotatedRect &rotRect, bool showStep)
{
  // Get rectangle around biggest blob
  try
  {
    std::vector<cv::Point> blobPts;

    if(biggestBlob.empty())
    {
      throw std::invalid_argument("Error! Size of image should be greater than zero!");
    }

    cv::findNonZero(biggestBlob, blobPts);
    rotRect = cv::minAreaRect(blobPts);

    if(rotRect.size.area() == 0 || blobPts.empty())
    {
      throw std::domain_error("Error! Area of rectangle should be greater than zero!");
    }

    if(showStep)
    {
      std::cout << "Angle of box = " << rotRect.angle \
                << " Width = "<< rotRect.size.width \
                << " Height = " << rotRect.size.height \
                <<"\n"<< rotRect.boundingRect()<< std::endl;
    }
  }

  catch(std::exception const &ex)
  {
    std::cerr << ex.what() << std::endl;
  }

}

void Segmentation::DeskewImage(cv::Mat &inputImage, const cv::Mat &mask, bool showStep)
{
  try
  {
    std::vector<cv::Vec4i> lines;
    cv::Size size = inputImage.size();
    cv::HoughLinesP(mask, lines, 1, CV_PI/360, 100, size.width / 6.f, 5 );

    double meanAngle = 0.0;

    if(!lines.empty())
    {
      int i = 0;
      for (auto it = lines.begin(); it != lines.end(); ++it, ++i)
      {

      meanAngle += atan2((double)lines[i][3] - lines[i][1],\
          (double)lines[i][2] - lines[i][0]);
      }
      meanAngle /= lines.size(); // mean angle, in radians.
    }

    cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f(size.width / 2.f, size.height / 2.f), meanAngle * 180 / CV_PI, 1.0);
    cv::warpAffine(inputImage, inputImage, rotMat, inputImage.size(), cv::INTER_CUBIC);

    if(showStep)
    {
      std::cout <<"Angle = "<< meanAngle * 180 / CV_PI << std::endl;
      std::cout <<"The number of all detected lines is "<< lines.size() << std::endl;

      cv::imshow("Deskew", inputImage);
      cv::waitKey(0);
    }

  }

  catch (cv::Exception& ex){std::cerr<<"Caught exception while deskewImage: "<<ex.msg << std::endl;}

}

void Segmentation::ContrastInc(const cv::Mat &inputImage, cv::Mat &outputImage, bool showStep)
{
  outputImage = cv::Mat::zeros(inputImage.size(), inputImage.type());
  for( int y = 0; y < inputImage.rows; y++ )
  {
    for( int x = 0; x < inputImage.cols; x++ )
    {
      for( int c = 0; c < 3; c++ )
      {
        outputImage.at<cv::Vec3b>(y,x)[c] = \
                       cv::saturate_cast<uchar>( alpha * ( inputImage.at<cv::Vec3b>(y,x)[c] ) + beta );
      }
    }
  }
  if(showStep){cv::imshow("Contrast image", outputImage); cv::waitKey(0);}
}

void Segmentation::SharpnessInc(const cv::Mat &inputImage, cv::Mat &outputImage, bool showStep)
{
  cv::GaussianBlur(inputImage, outputImage, cv::Size(0, 0), 3);
  cv::addWeighted(inputImage, 1.5, outputImage, -0.5, 0, outputImage);
  if(showStep){cv::imshow("Sharpness image", outputImage); cv::waitKey(0);}
}

void Segmentation::DrawRect(cv::Mat inputImage, const cv::RotatedRect &rotRect)
{
  cv::Point2f verticesRect[4];
  rotRect.points(verticesRect);
  for (int i = 0; i < 4; ++i)
  {
    line(inputImage, verticesRect[i], verticesRect[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
  }
}
