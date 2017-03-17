#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"

#include <iostream>

int parseArg(std::string); //parsing function
unsigned int numHistogramBins; //necessary to gather information from DICOM
unsigned int numIterations;
unsigned int multiResLevels;
float learnRate;

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

    //read
    typedef itk::ImageFileReader<InputImageType> ReaderType;

    ReaderType::Pointer reader = ReaderType::New();
    
    reader->SetFileName(argv[1]);
    std::string argFileName = argv[1];
    parseArg(argFileName);
	
	return 0;
}

//UNDER CONSTRUCTION
int parseArg(std::string argfile)
{
	//open argFile
	FILE * argFile;
	argFile = fopen(argfile.c_str(), "r");
	if (!argFile)
	{
		std::cerr << "Can't open file." << std::endl;
		return EXIT_FAILURE;
	}

	//parse file
	return 0;
}