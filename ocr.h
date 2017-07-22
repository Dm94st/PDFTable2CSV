#ifndef OCR_H
#define OCR_H

#include "segmentation.h"
#include <tesseract/baseapi.h>

using namespace tesseract;
class OCR
{
public:
  OCR();
  ~OCR()
  {
    m_tesserApi->End();
  }

  std::wstring extractText(const cv::Mat &srcPic);

private:

  TessBaseAPI * m_tesserApi = nullptr;

};

#endif // OCR_H
