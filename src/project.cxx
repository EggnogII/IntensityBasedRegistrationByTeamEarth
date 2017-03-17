#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include <iostream>

int main(int argc, char *argv[])
{
	if( argc < 2 )
    {
    std::cerr << "Usage: "
              << argv[0]
              << " InputDicomDirectory"
              << std::endl;
    return EXIT_FAILURE;
    }

    //specify dimensions 
    typedef signed short InputPixelType;
    const unsigned int InputDimension = 3;

    typedef itk::Image<InputPixelType, InputDimension> InputImageType;


    typedef itk::GDCMImageIO ImageIOType;
    typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
    typedef itk::ImageSeriesReader<InputImageType> ReaderType;


    //read
    ImageIOType::Pointer gdcmIO = ImageIOType::New();
    InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
    inputNames->SetInputDirectory(argv[1]);

    const ReaderType::FileNamesContainer &filenames = inputNames->GetInputFileNames();

    ReaderType::Pointer reader  = ReaderType::New();
    reader->SetImageIO(gdcmIO);
    reader->SetFileNames(filenames);

	
	return 0;
}
