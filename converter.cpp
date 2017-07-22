#include "converter.h"

Converter::Converter(const std::string &inputFile, const std::string &outputFile, const int &dpi):
  m_inputFile(inputFile), \
  m_outputFile("-sOutputFile=" + outputFile + GetFilename(m_inputFile).c_str() + "_" + "page_%d.png"), \
  m_dpi("-r" + std::to_string(dpi) )
{
  InitArgv();
  m_instCode = gsapi_new_instance(&m_inst, nullptr); //get instance of gs
  if (m_instCode < 0)
    throw std::bad_alloc();
  else
    m_instCode = gsapi_set_arg_encoding(m_inst, GS_ARG_ENCODING_UTF8); //get instance code

  if(!IsRaster())
    throw std::invalid_argument(std::string(RED) + std::string("Fail! PDF file is not raster! \n") + std::string(RESET));
}

Converter::~Converter()
{
  RemoveFiles();
  m_exitCode = gsapi_exit(m_inst);
  if ((m_exitCode == 0) || (m_exitCode == gs_error_Quit))
  {
    gsapi_delete_instance(m_inst);
  }
}

bool Converter::ToPNG()
{
  if (m_instCode == 0)
  {
    gsapi_init_with_args(m_inst, m_gsargc, m_gsargv); // start process
    return 0;
  }
  return 1;
}

const std::string Converter::GetPath() const
{
  size_t lastSlash = m_outputFile.find_last_of("/");
  size_t firstSlash = m_outputFile.find("/");
  return m_outputFile.substr(firstSlash, m_outputFile.length() - firstSlash - (m_outputFile.length() - lastSlash));
}

const std::vector<std::string> Converter::ListPages() const
{
  std::vector<std::string> pages;
  DIR *pDIR = nullptr;
  struct dirent *entry = nullptr;

  if((pDIR = opendir(GetPath().c_str())))
  {
    while((entry = readdir(pDIR)))
    {
      if(std::regex_match(entry->d_name, std::regex(GetFilename(m_inputFile) + "_page_[[:digit:]]{1,}.png"))){
        pages.push_back(GetPath() + "/" + entry->d_name);}
    }
    closedir(pDIR);

    if (!pages.empty())
    {
      return pages;
    }

    else
    {
      std::cerr << RED << "Fail! Directory or PDF file does not contain images! \n" << RESET;
      exit(1);
    }
  }
  return pages;
}

// Remove PNG files after recognize
bool Converter::RemoveFiles() const
{
  std::vector<std::string> pages = ListPages();
  for(auto page = pages.begin(); page != pages.end(); ++page)
  {
    if( ::remove(page->c_str()) != 0 )
    {
      std::cerr << RED << "Error deleting file" << RESET << std::endl;
      continue;
    }
    else
    {
      std::cout << GREEN << "File successfully deleted" << RESET << std::endl;
    }
  }
  return 0;
}

void Converter::InitArgv()
{
  // Set options
  m_gsargv[0] = const_cast<char*>("ps2pdf");
  std::cout<<m_gsargv[0]<<std::endl;

  m_gsargv[1] = const_cast<char*>("-q");
  std::cout<<m_gsargv[1]<<std::endl;

  m_gsargv[2] = const_cast<char*>("-dNOPAUSE");
  std::cout<<m_gsargv[2]<<std::endl;

  m_gsargv[3] = const_cast<char*>("-sDEVICE=png16m");
  std::cout<<m_gsargv[3]<<std::endl;

  m_gsargv[4] = const_cast<char*>(m_dpi.c_str());
  std::cout<<m_gsargv[4]<<std::endl;

  m_gsargv[5] = const_cast<char*>(m_outputFile.c_str());
  std::cout<<m_gsargv[5]<<std::endl;

  m_gsargv[6] = const_cast<char*>(m_inputFile.c_str());
  std::cout<<m_gsargv[6]<<std::endl;

  m_gsargv[7] = const_cast<char*>("-c");
  std::cout<<m_gsargv[7]<<std::endl;

  m_gsargv[8] = const_cast<char*>("quit");
  std::cout<<m_gsargv[8]<<std::endl;
}

// PDF file is a raster?
bool Converter::IsRaster() const
{
  std::ifstream in(m_inputFile, std::ios::binary);
  char * fileBuf = nullptr;
  const char * imgByte = "496d616765";
  bool status = false;

  if(in.is_open())
  {
    std::cout<<"File is open"<<std::endl;
    in.seekg (0, in.end); // Set cursor at the end
    const int length = in.tellg(); // Calculate size of file
    in.seekg (0, in.beg); // Set cursor at the begin

    fileBuf = new char[length]; // Create buffer for the first 200 bytes
    in.read(fileBuf, 10000); // Read first 10000 bytes

    in.close(); // Close file

    std::stringstream bytes;

    // Write first 10000 bytes
    for (int i = 0; i < 10000; ++i)
    {
      bytes << std::hex << std::setw(2) << std::setfill('0')
            << (int)fileBuf[i];
    }

    int posFind = bytes.str().find(imgByte);

    if(posFind != -1)
      status =  true;

    /*std::cout << bytes.str() <<std::endl;*/

    std::cout <<"Found at "<< posFind <<std::endl;

    delete [] fileBuf;
    fileBuf = nullptr;
  }

  else
    throw std::invalid_argument(std::string(RED) + "Error opening file\n" + std::string(RESET));

  return status;
}
