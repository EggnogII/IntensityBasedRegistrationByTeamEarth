#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
//#include "itkImageSeriesReader.h"
//#include "itkImageSeriesWriter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkTranslationTransform.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkCastImageFilter.h"
#include "itkCommand.h"

/*

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
                            COMMAND TEMPLATE
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

*/
template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command
{
public:
    typedef RegistrationInterfaceCommand Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);

protected:
    RegistrationInterfaceCommand(){};

public:
    typedef TRegistration RegistrationType;
    typedef RegistrationType * RegistrationPointer;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef OptimizerType * OptimizerPointer;

    void Execute(itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
    {
        if (!(itk::IterationEvent().CheckEvent(&event)))
        {
            return;
        }
        RegistrationPointer registration = static_cast<RegistrationPointer>(object);
        if (registration == ITK_NULLPTR)
        {
            return;
        }
        OptimizerPointer optimizer = static_cast<OptimizerPointer>(registration->GetModifiableOptimizer());

        std::cout << "////////////////////////////////////////////////////////////////////" << std::endl;
        std::cout << "Multi-Res Level: " << registration->GetCurrentLevel() << std::endl << std::endl;

        if (registration->GetCurrentLevel() == 0)
        {
            optimizer->SetMaximumStepLength(16.00);
            optimizer->SetMinimumStepLength(0.01);
        }

        else
        {
            optimizer->SetMaximumStepLength(optimizer->GetMaximumStepLength() / 4.0);
            optimizer->SetMinimumStepLength(optimizer->GetMinimumStepLength() / 10.0);
        }

    }

    void Execute(const itk::Object *, const itk::EventObject &) ITK_OVERRIDE
    {
        return;
    }
};


/*

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
                            OBSERVER
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

*/

class CommandIterationUpdate : public itk::Command
{
public:
    typedef CommandIterationUpdate Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);

protected:
    CommandIterationUpdate() {};

public:
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef const OptimizerType * OptimizerPointer;

    void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE
    {
        Execute( (const itk::Object *)caller, event);
    }

    void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
    {
        OptimizerPointer optimizer = static_cast<OptimizerPointer>(object);
        if (!(itk::IterationEvent().CheckEvent(&event)))
        {
            return;
        }
        std::cout << optimizer->GetCurrentIteration() << "  ";
        std::cout << optimizer->GetValue() << "  ";
        std::cout << optimizer->GetCurrentPosition() << std::endl;

    }
};


/*

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
                            MAIN
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

*/


#include <iostream>

int main(int argc, char *argv[])
{
	if( argc < 3 )
    {
    std::cerr << "Usage: "
              << argv[0]
              << " FixedImage, MovingImage"
              << std::endl;
    return EXIT_FAILURE;
    }

    const std::string fixedImageDirectory = argv[1];
    const std::string movingImageDirectory = argv[2];
    //const std::string outputImageFile = argv[3];

    //specify dimensions 
    const unsigned int Dimension = 2;
    
    typedef unsigned short PixelType;
    typedef itk::Image<PixelType, Dimension> ImageType;

    typedef itk::ImageFileReader<ImageType> ReaderType;

    ReaderType::Pointer fixedReader = ReaderType::New();
    ReaderType::Pointer movingReader = ReaderType::New();

    fixedReader->SetFileName(fixedImageDirectory);
    movingReader->SetFileName(movingImageDirectory);

    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

    fixedReader->SetImageIO(gdcmImageIO);
    movingReader->SetImageIO(gdcmImageIO);

    //Attempt to read
    try
    {
        fixedReader->Update();
        movingReader->Update();
    }
    catch(itk::ExceptionObject &e)
    {
        std::cerr << "Exception in File Reader " << std::endl << e << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Read Successful." << std::endl;

    typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, Dimension> InternalImageType;

    //Component Declaration
    typedef itk::TranslationTransform<double, Dimension> TransformType;
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef itk::LinearInterpolateImageFunction<InternalImageType, double> InterpolatorType;
    typedef itk::MattesMutualInformationImageToImageMetric<InternalImageType, InternalImageType> MetricType;
    typedef itk::MultiResolutionImageRegistrationMethod<InternalImageType, InternalImageType> RegistrationType;

    //Filter Declaration
    typedef itk::MultiResolutionPyramidImageFilter<InternalImageType, InternalImageType> FixedImagePyramidType;
    typedef itk::MultiResolutionPyramidImageFilter<InternalImageType, InternalImageType> MovingImagePyramidType;

    //Component Instantiation
    TransformType::Pointer transform = TransformType::New();
    OptimizerType::Pointer optimizer = OptimizerType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    RegistrationType::Pointer registration = RegistrationType::New();
    MetricType::Pointer metric = MetricType::New();

    //Filter Instantiation
    FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
    MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();

    //Connect Components to Registration Object
    registration->SetOptimizer(optimizer);
    registration->SetTransform(transform);
    registration->SetInterpolator(interpolator);
    registration->SetMetric(metric);

    //Connect Filter Components to Registration Object
    registration->SetFixedImagePyramid(fixedImagePyramid);
    registration->SetMovingImagePyramid(movingImagePyramid);

    //GDCM Reader?
    typedef itk::ImageFileReader<ImageType> FixedImageReaderType;
    typedef itk::ImageFileReader<ImageType> MovingImageReaderType;

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

    fixedImageReader->SetFileName(fixedImageDirectory);
    movingImageReader->SetFileName(movingImageDirectory);

    //Cast to Internal Image Type
    typedef itk::CastImageFilter<ImageType, InternalImageType> FixedCastFilterType;
    typedef itk::CastImageFilter<ImageType, InternalImageType> MovingCastFilterType;

    FixedCastFilterType::Pointer fixedCaster = FixedCastFilterType::New();
    MovingCastFilterType::Pointer movingCaster = MovingCastFilterType::New();

    fixedCaster->SetInput(fixedImageReader->GetOutput());
    movingCaster->SetInput(movingImageReader->GetOutput());

    registration->SetFixedImage(fixedCaster->GetOutput());
    registration->SetMovingImage(movingCaster->GetOutput());

    fixedCaster->Update();

    std::cout << "Fixed Caster Update Successful" << std::endl;

    registration->SetFixedImageRegion(fixedCaster->GetOutput()->GetBufferedRegion());

    //Initial Parameters Set Up
    typedef RegistrationType::ParametersType ParametersType;
    ParametersType initialParameters(transform->GetNumberOfParameters());

    initialParameters[0] = 0.0; //Initial offset in mm along x
    initialParameters[1] = 0.0; //Initial offset in mm along y

    registration->SetInitialTransformParameters(initialParameters);

    metric->SetNumberOfHistogramBins(128);
    metric->SetNumberOfSpatialSamples(50000);

    metric->ReinitializeSeed(76926294);

    optimizer->SetNumberOfIterations(200);
    optimizer->SetRelaxationFactor(0.9);

    //Create Command observer, connect with optimizer
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    //Instance of Interface Command and connect it to the registration object
    typedef RegistrationInterfaceCommand<RegistrationType> CommandType;
    CommandType::Pointer command = CommandType::New();
    registration->AddObserver(itk::IterationEvent(), command);

    //Set number of resolution levels
    registration->SetNumberOfLevels(3);

    try
    {
        registration->Update();
    }
    catch(itk::ExceptionObject &e)
    {
        std::cerr << "Exception registration update" << e << std::endl;
        return EXIT_FAILURE;
    }





    return 0;
}
