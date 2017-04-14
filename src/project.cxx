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
#include "itkCommand.h"

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
    OBSERVER
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
    OptimizerType::Pointer optimizer = OptimizerType::New();
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    RegistrationType::Pointer registration = RegistrationType::New();
    MetricType::Pointer metric = MetricType::New();

    FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
    MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();

    std::cout << "Components initialized." << std::endl;
    
    //connect to registration object
    registration->SetTransform(transform);
    registration->SetOptimizer(optimizer);
    registration->SetInterpolator(interpolator);
    registration->SetMetric(metric); 
    registration->SetFixedImagePyramid(fixedImagePyramid);
    registration->SetMovingImagePyramid(movingImagePyramid);
    
    
    std::cout << "Components connected to Registration Object." << std::endl;
    //hard set the file reader to the arguments
        
    typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
    typedef itk::GDCMSeriesFileNames MovingNamesGeneratorType;
    typedef itk::ImageSeriesReader<MovingImageType> MovingImageReaderType;

    FixedImageReaderType::Pointer fixedImageReader = FixedImageReaderType::New();
    MovingNamesGeneratorType::Pointer nameGenerator = MovingNamesGeneratorType::New();
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

    fixedImageReader->SetFileName(fixedImageFile);
    
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
    std::cout << movingImageFile << std::endl;

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
    try
    {
        fixedImageReader->Update();
    }
    catch(itk::ExceptionObject & e)
    {
        std::cerr << "Exception in Fixed File Reader " << std::endl;
        std::cerr << e << std::endl;
        return EXIT_FAILURE;
    }
    typedef std::vector<std::string> FileNamesContainer;
    FileNamesContainer fileNames;

    fileNames = nameGenerator->GetFileNames(seriesIdentifier);
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
    
    std::cout << "Setting Readers." << std::endl;

    registration->SetFixedImage(fixedCaster->GetOutput());
    registration->SetMovingImage(movingCaster->GetOutput());
    std::cout << "Connecting registration object to Caster Output." << std::endl;

    fixedCaster->Update();
    std::cout << "Fixed Caster Update" << std::endl;

    registration->SetFixedImageRegion(fixedCaster->GetOutput()->GetBufferedRegion());

    std::cout << "Connecting registration object to Caster Output." << std::endl;

    typedef RegistrationType::ParametersType ParametersType;
    ParametersType initialParameters(transform->GetNumberOfParameters());

    initialParameters[0] = 0.0;  // Initial offset in mm along X
    initialParameters[1] = 0.0;  // Initial offset in mm along Y
    initialParameters[2] = 0.0;  // Initial offset in mm along Z
    
    registration->SetInitialTransformParameters(initialParameters);

    //Metric Set
    metric->SetNumberOfHistogramBins(128);
    metric->SetNumberOfSpatialSamples(50000); 
    metric->ReinitializeSeed(76926294);

    optimizer->SetNumberOfIterations(200);
    optimizer->SetRelaxationFactor(0.9);

    //If we add an Observer with Command it will be here
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    typedef RegistrationInterfaceCommand<RegistrationType> CommandType;
    CommandType::Pointer command = CommandType::New();
    registration->AddObserver(itk::IterationEvent(), command);

    std::cout << "Starting Registration Process..." << std::endl;
    //set level of Multi-Res levels
    registration->SetNumberOfLevels(3);
    std::cout << "Set Multi-Res Levels...." << std::endl;
    
    try
    {
        registration->Update();
        std::cout << optimizer->GetCurrentIteration() << " = ";
        std::cout << optimizer->GetValue() << " : ";
        std::cout << optimizer->GetCurrentPosition() << std::endl;
        std::cout << "Optimizer stop condition: " << registration->GetOptimizer()->GetStopConditionDescription() << std::endl;
    }

    catch(itk::ExceptionObject &err)
    {
        std::cout << "EXCEPTION!!!!! " << err << std::endl;
        return EXIT_FAILURE;
    }




    return 0;
}
