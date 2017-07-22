#include "segmentation.h"
#include "imagefromfile.h"

/*Program for extracting structured text information from graphical documents that contains information about affilated persons
 * REQUIREMENTS:
 * OpenCV 3.2 - Image processing and pattern recognition
 * Tesseract - OCR API 3.05.00 - Recognize text from each cell
 * Ghostscript 9.21 API - split PDF file
 * CellCSV - write result in CSV file (https://github.com/d4nF/CellCSV)
*/

/* Usage:
 * Path until source PDF file;
 * Path until output csv's;
 * Recognition language
*/

std::string settings::inPath;
std::string settings::outPath;
const char * settings::lang;

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    // Expect 4 arguments: the program name, path until source PDF file, path until output csv's, recognition language
    std::cerr << "Usage: " << argv[0] << "<srcPDFfile> <outputCSVfile> [lang]"
              << std::endl;
    return 1;
  }
  else
  {
    for(int i = 1; i < argc; ++i)
    {
      settings::inPath = argv[i++]; // src
      settings::outPath = argv[i++]; // dst

      if(argc == 4)
        settings::lang = argv[i++]; // lang
      else
        settings::lang = "rus";
        break;

    }
  }

  std::cout<<"src: " <<settings::inPath<<"\n" \
           <<"dst: " <<settings::outPath<<"\n"
           <<"lang: "<<settings::lang<<std::endl;

  // Array with paths until png pages
  std::vector<std::string> pagesVec;

  try
  {
    // Initialize converter
    Converter * initConv = new Converter(settings::inPath, settings::outPath, settings::dpi); // in:argv[1], out:argv[2]

    // Split PDF file on pages
    initConv->ToPNG();
    pagesVec = initConv->ListPages();

    // Processing every page
    for(auto p:pagesVec)
    {
      Segmentation * page = new ImageFromFile(p);
      page->preProcess();
    }

    // Delete png files(pages) and clean memory
    delete initConv;

  }

  catch(std::exception const &ex)
  {
    std::cerr << ex.what();
  }


  /* For a single page
  Segmentation * a = new ImageFromFile("/Users/V3r0n/Downloads/page_2.png");
  a->preProcess();
  */

  return 0;
}

