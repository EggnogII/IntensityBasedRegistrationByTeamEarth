#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkTranslationTransform.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#

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
    
    //connect to registration object
    registration->SetTransform(transform);
    registration->SetOptimizer(optimzer);
    registration->SetInterpolator(interpolator);
    registration->SetFixedImagePyramid(fixedImagePyramid);
    registration->SetMovingImagePyramid(movingImagePyramid);
    
    //hard set the file reader to the arguments
    typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
    typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

    //Need to consider how this will work for dcm files or if we want to use some other file format.
    fixedImageReader->SetFileName(fixedImageFile);
    movingImageReader->SetFileName(movingImageFile);
    //std::cout << "MADE IT" << std::endl;
    return 0;
}
