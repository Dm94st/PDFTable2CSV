#include "imagefromfile.h"

ImageFromFile::ImageFromFile(const std::string &fileName):
    m_fileName(fileName)
{
  if(fileName.empty()){std::cerr << "Path is wrong or empty..!" <<std::endl;}
}

cv::Mat ImageFromFile::GetImage()
{
  cv::Mat sourceImage;
  try
  {
    sourceImage = cv::imread(m_fileName);
    if(sourceImage.empty()) throw std::invalid_argument("Error while reading image!");
  }
  catch(const std::exception & ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return sourceImage;
}
