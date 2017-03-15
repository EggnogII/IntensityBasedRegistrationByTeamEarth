#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"

#include <iostream>

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