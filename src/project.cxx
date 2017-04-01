#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkTranslationTransform.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkCastImageFilter.h"

#include <iostream>

int main(int argc, char *argv[])
{
	if( argc < 3 )
    {
    std::cerr << "Usage: "
              << argv[0]
              << "FixedImage, MovingImage"
              << std::endl;
    return EXIT_FAILURE;
    }

    //specify dimensions 
    const unsigned int Dimension = 3;
    typedef unsigned short PixelType;

    const std::string fixedImageFile = argv[1];
    const std::string movingImageFile = argv[2];
    //const std::string outputImageFile = argv[3];

    typedef itk::Image<PixelType, Dimension> FixedImageType;
    typedef itk::Image<PixelType, Dimension> MovingImageType;
    
    typedef float InternalPixelType;

    typedef itk::Image<InternalPixelType, Dimension> InternalImageType;

    typedef itk::TranslationTransform<double, Dimension> TransformType;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
	typedef itk::LinearInterpolateImageFunction<InternalImageType, double> InterpolatorType;
    typedef itk::MattesMutualInformationImageToImageMetric<InternalImageType, InternalImageType > MetricType;
    typedef itk::MultiResolutionImageRegistrationMethod<InternalImageType, InternalImageType >   RegistrationType;

    //specify filter
    typedef itk::MultiResolutionPyramidImageFilter<InternalImageType, InternalImageType> FixedImagePyramidType;
    typedef itk::MultiResolutionPyramidImageFilter<InternalImageType, InternalImageType> MovingImagePyramidType;
    
    //component initialization
    TransformType::Pointer transform = TransformType::New();
    OptimizerType::Pointer optimzer = OptimizerType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    RegistrationType::Pointer registration = RegistrationType::New();
    MetricType::Pointer metric = MetricType::New();

    FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
    MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();

    std::cout << "Components initialized." << std::endl;
    
    //connect to registration object
    registration->SetTransform(transform);
    registration->SetOptimizer(optimzer);
    registration->SetInterpolator(interpolator);
    registration->SetFixedImagePyramid(fixedImagePyramid);
    registration->SetMovingImagePyramid(movingImagePyramid);
    
    std::cout << "Components connected to Registration Object." << std::endl;
    //hard set the file reader to the arguments
        
    typedef itk::ImageSeriesReader<FixedImageType> FixedImageReaderType;
    typedef itk::GDCMSeriesFileNames MovingNamesGeneratorType;
    typedef itk::ImageSeriesReader<MovingImageType> MovingImageReaderType;

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingNamesGeneratorType::Pointer nameGenerator = MovingNamesGeneratorType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    
    typedef itk::GDCMImageIO FixedImageIOType;
    FixedImageIOType::Pointer FixedDicomIO = FixedImageIOType::New();
    fixedImageReader->SetImageIO(FixedDicomIO);


    /*
        Optionally at this point we can use 

        nameGenerator->SetUseSeriesDetails(true);
        nameGenerator->AddSeriesRestriction("");
        
        "" denotes the restriction
        "0020 | 0011" Series Number
        "0018 | 0024" Sequence Name
        "0018 | 0050" Slice Thickness
        "0028 | 0010" Rows
        "0028 | 0011" Columns
    */
    nameGenerator->SetUseSeriesDetails(true);
    nameGenerator->AddSeriesRestriction("0008 | 0021");
    nameGenerator->SetDirectory(argv[2]);

/* Test block to see if we can see the details of the DICOM series of images */
    
    std::cout << "This Directory contains the series" << std::endl;
    std::cout << argv[2] << std::endl;

    typedef std::vector<std::string> SeriesIDContainer;

    const SeriesIDContainer & seriesUID = nameGenerator->GetSeriesUIDs();
    SeriesIDContainer::const_iterator itr = seriesUID.begin();
    SeriesIDContainer::const_iterator end = seriesUID.end();
    while (itr != end)
    {
        std::cout << itr->c_str() << std::endl;
        ++itr;
    }
//Another Test Block
    std::string seriesIdentifier;
    seriesIdentifier = seriesUID.begin()->c_str();
    std::cout << std::endl;
    std::cout << "Reading Series: " << std::endl << std::endl;
    std::cout << seriesIdentifier << std::endl;
    std::cout << std::endl << std::endl;

//End test block

    //Read
    typedef std::vector<std::string> FileNamesContainer;
    FileNamesContainer fileNames;

    fileNames = nameGenerator->GetFileNames(seriesIdentifier);
    //need a 'reader' of sorts
    movingImageReader->SetFileNames(fileNames);
    try
    {
        movingImageReader->Update();
    }
    catch(itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
        return EXIT_FAILURE;
    }
    //end reading

    //cast to internal image type
    typedef itk::CastImageFilter<FixedImageType, InternalImageType> FixedCastFilterType;
    typedef itk::CastImageFilter<MovingImageType, InternalImageType> MovingCastFilterType;
    
    FixedCastFilterType::Pointer fixedCaster = FixedCastFilterType::New();
    MovingCastFilterType::Pointer movingCaster = MovingCastFilterType::New();

    std::cout << "Casting to internal image type." << std::endl;

    //set output of image reader to input of caster filter.
    //input to registration are coming from caster filter.
    fixedCaster->SetInput(fixedImageReader->GetOutput());
    movingCaster->SetInput(movingImageReader->GetOutput());

    registration->SetFixedImage(fixedCaster->GetOutput());
    registration->SetMovingImage(movingCaster->GetOutput());

    fixedCaster->Update();

    registration->SetFixedImageRegion(fixedCaster->GetOutput()->GetBufferedRegion());

    typedef RegistrationType::ParametersType ParametersType;
    ParametersType initialParameters(transform->GetNumberOfParameters());

    initialParameters[0] = 0.0;  // Initial offset in mm along X
    initialParameters[1] = 0.0;  // Initial offset in mm along Y
    initialParameters[2] = 0.0;  // Initial offset in mm along Z
    
    registration->SetInitialTransformParameters(initialParameters);
    

    return 0;
}
