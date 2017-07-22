#include "ocr.h"
#include "wchar.h"
#include "locale.h"

OCR::OCR()
{
  m_tesserApi = new tesseract::TessBaseAPI();
  if (m_tesserApi->Init(nullptr, lang)) {
         std::cerr << "Could not initialize tesseract.\n" << std::endl;
         exit(1);
     }
}

std::wstring OCR::extractText(const cv::Mat &srcPic)
{
  char * outText;

  m_tesserApi->SetImage((uchar*)srcPic.data, srcPic.size().width, srcPic.size().height, srcPic.channels(), srcPic.step);
  m_tesserApi->Recognize(0);

  outText = m_tesserApi->GetUTF8Text();

  // Convert UTF-8 to Unicode
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> UTF8_UTF_16_CONVERTER;
  return UTF8_UTF_16_CONVERTER.from_bytes(outText);
}
