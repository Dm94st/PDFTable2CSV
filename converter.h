#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <regex>

#include "ghostscript/iapi.h"
#include "ghostscript/ierrors.h"
#include "dirent.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */

class Converter
{
public:

  Converter() = delete;

  // Default constructor
  Converter(const std::string &inputFile, const std::string &outputFile, const int &dpi);

  ~Converter();

  // Split PDF file to images
  bool ToPNG();

  const std::vector<std::string> ListPages() const;

  // Extract filename from path
  static const std::string GetFilename(const std::string& str)
  {
    auto pos = str.rfind("/");
    if(pos == std::string::npos)
      pos = -1;
    return std::string(str.begin() + pos + 1, str.end());
  }

private:
  const std::string m_inputFile;
  const std::string m_outputFile;
  const std::string m_dpi;

  void * m_inst = nullptr;

  char * m_gsargv[10] = {}; //array with args
  const int m_gsargc = 9;

  int m_instCode = 0;
  int m_exitCode = 0;

  // Initialize array with args
  void InitArgv();

  // PDF file is raster?
  bool IsRaster () const;

  // Extract path until PDF file
  const std::string GetPath() const;

  // Remove png files after conversation
  bool RemoveFiles() const;

};

#endif // CONVERTER_H
