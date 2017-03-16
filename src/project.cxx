#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"

#include <string>
#include <iostream>

bool is_file_exist(const char *fileName)
{
	std::ifstream infile(fileName);
  return infile.good();
}

int main(int argc, char *argv[])
{
	if( argc < 3 )
    {
    std::cerr << "Usage: "
              << argv[0]
              << " InputDicomDirectory OutputDicomDirectory [series]"
              << std::endl;
    return EXIT_FAILURE;
    }

	std::string inputCheck = argv[1];
	if( inputCheck.find(".dcm") == std::string::npos)
	{
	std::cerr << argv[1] 
						<< " needs to be dicom file format!"
						<< std::endl;

	return EXIT_FAILURE;
	}

	if(is_file_exist(argv[1]))
	{
		std::cerr << argv[1]
							<< " does not exist!"
							<< std::endl;

		return EXIT_FAILURE;
	}

    //specify dimensions 
    typedef signed short InputPixelType;
    const unsigned int InputDimension = 3;

    typedef itk::Image<InputPixelType, InputDimension> InputImageType;

    //read
    typedef itk::ImageFileReader<InputImageType> ReaderType;

			
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[1]);
	
	return 0;
}
