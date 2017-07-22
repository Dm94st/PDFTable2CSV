#ifndef IMAGEFROMFILE_H
#define IMAGEFROMFILE_H

#include "segmentation.h"

class ImageFromFile : public Segmentation
{
public:
  ImageFromFile(const std::string &fileName);

private:
  std::string m_fileName;
  cv::Mat GetImage() override;
};

#endif // IMAGEFROMFILE_H
