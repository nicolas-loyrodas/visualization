/*

    Author: Nicolas Loy Rodas

*/
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>

#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkAxesActor.h>
#include <vtkMetaImageWriter.h>
#include <vtkCenterOfMass.h>

#include <sstream>

void volRenderingVisualization(  vtkSmartPointer<vtkImageData> imageData );
bool saveAsMHD(vtkSmartPointer<vtkImageData> volume, std::string outputFile, bool center);

void printHelp()
{
    std::cout << "DICOM to MHD: Help " << std::endl;
    std::cout << "-h Print this help message" << std::endl;
    std::cout << "-i <Input folder containing the DICOM images>" << std::endl;
    std::cout << "-o <Output MHD file>" << std::endl;
    std::cout << "-f <Filter table threshold>" << std::endl;
    std::cout << "-c <Option to compute offset of MHD file to center the model>" << std::endl;
    std::cout << "-v <Include to visualize the CT with volume rendering>" << std::endl;
}

int main(int argc, char* argv[])
{
    std::string inputFolder = "";
    std::string output = "/output/output";
    bool TABLE_FILTERING = false;
    bool VISUALIZATION = false;
    bool SAVE_AS_MHD = false;
    bool CENTER_MHD = false;

    //! Verify input arguments
    if(argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if(std::string (argv[i]) == "-h")
            {
                printHelp();
                return -1;
            }
            if(std::string (argv[i]) == "-i")
            {
                inputFolder = argv[i+1];
                std::cout << "Input folder containing the DICOM images " << inputFolder << std::endl;
            }
            if(std::string (argv[i]) == "-o")
            {
                output = argv[i+1];
                SAVE_AS_MHD = true;
                std::cout << "Output MHD file " << output << ".mhd" << std::endl;
            }
            if(std::string (argv[i]) == "-f")
            {
                TABLE_FILTERING = true;
                std::cout << "Table filtering ON" << std::endl;
            }
            if(std::string (argv[i]) == "-v")
            {
                VISUALIZATION = true;
                std::cout << "Visualization ON" << std::endl;
            }
            if(std::string (argv[i]) == "-c")
            {
                CENTER_MHD = true;
                std::cout << "Centering MHD ON" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Not enough Arguments..." << std::endl;
        printHelp();
        return -1;
    }

   //! Read all the DICOM files in the specified directory.
   vtkSmartPointer<vtkDICOMImageReader> reader =
      vtkSmartPointer<vtkDICOMImageReader>::New();
   reader->SetDirectoryName(inputFolder.c_str());
   reader->Update();

   vtkSmartPointer<vtkImageData> volume = vtkSmartPointer<vtkImageData>::New();
   volume->ShallowCopy(reader->GetOutput());

   double sp[3];
   volume->GetSpacing(sp);
   int* dims = volume->GetDimensions();

   float minVal = volume->GetScalarRange()[0];

    //! To filter-out the table from the CT volume
    if( TABLE_FILTERING )
    {
        std::cout << "Filtering table from CT volume...";
        int filtering_threshold = 60; //! Adjust accordingly
        for (int x = 0; x < dims[0]; x++)
        {
           for (int z = 0; z < dims[2]; z++)
           {
                for (int y = 0; y < filtering_threshold; y++)
                {
                    volume->SetScalarComponentFromFloat(x, y, z, 0, minVal);
                }
           }
        }
        std::cout << "done\n";
    }

    if( SAVE_AS_MHD )
    {
        std::cout << "Saving MHD file " << output << ".mhd" << std::endl;
        if( saveAsMHD( volume, output, CENTER_MHD ))
            std::cout << "File saved correctly " << std::endl;
    }

    if( VISUALIZATION )
    {
        std::cout << "Starting Volume Rendered Visualization " << std::endl;
        volRenderingVisualization( volume );
    }


    return EXIT_SUCCESS;
}

void volRenderingVisualization(  vtkSmartPointer<vtkImageData> imageData )
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
      vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
      vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
      vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
      vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
      vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
      vtkSmartPointer<vtkPiecewiseFunction> gradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
      vtkSmartPointer<vtkPiecewiseFunction> scalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
      vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
      vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();

      renderer->SetBackground(0.1, 0.2, 0.3);

      renderWindow->AddRenderer(renderer);
      renderWindow->SetSize(1500, 1500);

      renderWindowInteractor->SetInteractorStyle(interactorStyle);
      renderWindowInteractor->SetRenderWindow(renderWindow);

      volumeMapper->SetBlendModeToComposite();
      //volumeMapper->SetRequestedRenderMode( vtkSmartVolumeMapper::GPURenderMode );
      volumeMapper->SetRequestedRenderMode( vtkSmartVolumeMapper::DefaultRenderMode );
      //volumeMapper->SetRequestedRenderMode( vtkSmartVolumeMapper::RayCastAndTextureRenderMode );
      //volumeMapper->SetRequestedRenderMode( vtkSmartVolumeMapper::RayCastRenderMode );
      volumeMapper->SetInput( imageData );

      volumeProperty->ShadeOn();
      volumeProperty->SetInterpolationTypeToLinear();

      volumeProperty->SetAmbient(0.1);
      volumeProperty->SetDiffuse(0.9);
      volumeProperty->SetSpecular(0.2);
      volumeProperty->SetSpecularPower(10.0);

      gradientOpacity->AddPoint(0.0, 0.0);
      gradientOpacity->AddPoint(2000.0, 1.0);
      volumeProperty->SetGradientOpacity(gradientOpacity);

      scalarOpacity->AddPoint(-800.0, 0.0);
      scalarOpacity->AddPoint(-750.0, 1.0);
      scalarOpacity->AddPoint(-350.0, 1.0);
      scalarOpacity->AddPoint(-300.0, 0.0);
      scalarOpacity->AddPoint(-200.0, 0.0);
      scalarOpacity->AddPoint(-100.0, 1.0);
      scalarOpacity->AddPoint(1000.0, 0.0);
      //scalarOpacity->AddPoint(1703.0, 0.0);
      scalarOpacity->AddPoint(2750.0, 0.0);
      scalarOpacity->AddPoint(2976.0, 1.0);
      scalarOpacity->AddPoint(3000.0, 0.0);
      volumeProperty->SetScalarOpacity(scalarOpacity);

      color->AddRGBPoint(-750.0, 0.08, 0.05, 0.03);
      color->AddRGBPoint(-350.0, 0.39, 0.25, 0.16);
      color->AddRGBPoint(-200.0, 0.80, 0.80, 0.80);
      color->AddRGBPoint(2750.0, 0.70, 0.70, 0.70);
      color->AddRGBPoint(3000.0, 0.35, 0.35, 0.35);
      volumeProperty->SetColor(color);

      volume->SetMapper(volumeMapper);
      volume->SetProperty(volumeProperty);
      renderer->AddVolume(volume);
      renderer->ResetCamera();

      vtkSmartPointer<vtkAxesActor> Worldaxes = vtkSmartPointer<vtkAxesActor>::New();
      Worldaxes->SetTotalLength(1000, 1000, 1000);
      Worldaxes->AxisLabelsOff();
      renderer->AddActor(Worldaxes);

      renderWindow->Render();
    renderWindowInteractor->Start();
}

bool saveAsMHD( vtkSmartPointer<vtkImageData> volume, std::string outputFile, bool center)
{
    if( center )
    {
        int dims[3];
        double elementSpacing[3];
        volume->GetDimensions(dims);
        volume->GetSpacing(elementSpacing);
        double origin[3];
        //! Offset computation
        origin[0] = dims[0] * elementSpacing[0] * 0.5 ;
        origin[1] = dims[1] * elementSpacing[1] * 0.5 ;
        origin[2] = dims[2] * elementSpacing[2] * 0.5 ;

        volume->SetOrigin( origin );
        volume->Modified();
    }

    outputFile = outputFile + ".";
    vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
    writer->SetFileName( outputFile.c_str() );
    writer->SetInput( volume );
    writer->SetCompression( false );
    writer->Write();

    return true;
}




