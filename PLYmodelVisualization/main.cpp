#include <iostream>

//vtk
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLine.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkAxesActor.h>
#include <vtkPLYReader.h>
#include <vtkCenterOfMass.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include <boost/filesystem.hpp>
#include <vtkSphereSource.h>

#include <eigen3/Eigen/Geometry>

using namespace Eigen;

bool loadCameraPose(std::string folder, int index, Matrix4d* matrix);
vtkSmartPointer<vtkTransform> eigenTransformToVTK(Matrix4d* eigenMat);
Eigen::Matrix4d computeRotationMatrix(std::string axis, double angleInDegs, Eigen::Matrix4d transformationToCenterOfMass);

int NB_ROTATIONS = 200;
double STEP = 5;

void printHelp()
{
    std::cout << "PLY Visualization: Help " << std::endl;
    std::cout << "-h Print this help message" << std::endl;
    std::cout << "-i <Input .ply file to visualize (full path without file extension)>" << std::endl;
    std::cout << "-a <Axis to rotate object>" << std::endl;
}

int main(int argc, char** argv)
{
    std::string name = "";
    std::string rotation_axis = "x"; //default rotation axis
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
                name = argv[ i + 1];
            }
            if(std::string (argv[i]) == "-a")
            {
                rotation_axis = argv[ i + 1];
            }

        }
    }
    else
    {
        std::cout << "Not enough Arguments..." << std::endl;
        printHelp();
        return -1;
    }


    std::string input = name + ".ply";

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();

    vtkSmartPointer<vtkTransform> worldT = vtkSmartPointer<vtkTransform>::New();
    worldT->Identity();

    if(boost::filesystem::exists(input))
    {
        //! Read ply model
        vtkSmartPointer<vtkPLYReader> c_reader =  vtkSmartPointer<vtkPLYReader>::New();
        c_reader->SetFileName ( input.c_str() );
        c_reader->Update();

        vtkSmartPointer<vtkPolyData> model_polyData = vtkSmartPointer<vtkPolyData>::New();
        model_polyData = c_reader->GetOutput();

        vtkSmartPointer<vtkPolyDataMapper> model_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        model_mapper->SetInput(model_polyData);

        //! Compute center of mass of object
        vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter = vtkSmartPointer<vtkCenterOfMass>::New();

        #if VTK_MAJOR_VERSION <= 5
          centerOfMassFilter->SetInput(model_polyData);
        #else
          centerOfMassFilter->SetInputData(model_polyData);
        #endif
          centerOfMassFilter->SetUseScalarsAsWeights(false);
          centerOfMassFilter->Update();

        double center[3];
        centerOfMassFilter->GetCenter(center);

        Eigen::Matrix4d T_CoM;
        T_CoM << 1, 0, 0, center[0],
                0, 1, 0, center[1],
                0, 0, 1, center[2],
                0, 0, 0, 1;

        T_CoM = T_CoM.inverse().eval();

        //! Visualization
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        renderWindow->AddRenderer(renderer);
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        renderWindowInteractor->SetRenderWindow(renderWindow);

        renderer->SetBackground(0, 1, 1); // white

        actor->SetMapper(model_mapper);
        renderer->AddActor(actor);

        int counter = 0;
        //! Rotate object continuously around center of mass
        for(int i = -NB_ROTATIONS; i < NB_ROTATIONS; i++)
        {
            Eigen::Matrix4d rotation_i = computeRotationMatrix(rotation_axis, i, T_CoM); // axis x, y, or z

            rotation_i = T_CoM * rotation_i;

            vtkSmartPointer<vtkTransform> rotation = vtkSmartPointer<vtkTransform>::New();
            rotation = eigenTransformToVTK(&rotation_i);

            actor->SetUserTransform(rotation);
            renderWindow->Render();

            renderWindow->Render();
            counter ++;
        }


        renderWindow->Render();
        renderWindowInteractor->Start();
    }
    else
        std::cout << "Could not find file " << input << std::endl;


    return 0;
 }


vtkSmartPointer<vtkTransform> eigenTransformToVTK(Matrix4d* eigenMat)
{
    vtkSmartPointer<vtkTransform> vtkPose = vtkSmartPointer<vtkTransform>::New();

    double PoseMatrix[] = {eigenMat->coeffRef(0,0), eigenMat->coeffRef(0,1), eigenMat->coeffRef(0,2), eigenMat->coeffRef(0,3),
                        eigenMat->coeffRef(1,0), eigenMat->coeffRef(1,1), eigenMat->coeffRef(1,2), eigenMat->coeffRef(1,3),
                        eigenMat->coeffRef(2,0), eigenMat->coeffRef(2,1), eigenMat->coeffRef(2,2), eigenMat->coeffRef(2,3),
                        eigenMat->coeffRef(3,0), eigenMat->coeffRef(3,1), eigenMat->coeffRef(3,2), eigenMat->coeffRef(3,3)};
    vtkPose->SetMatrix(PoseMatrix);

    return vtkPose;
}

Eigen::Matrix4d computeRotationMatrix(std::string axis, double angleInDegs, Eigen::Matrix4d transformationToCenterOfMass)
{
    Eigen::Matrix4d mat;
    double ang = angleInDegs * 3.14159265 / 180;

    if(axis == "x")
    {
        mat <<  1,0,0,0,
               0, cos(ang), -sin(ang), 0,
                0, sin(ang), cos(ang), 0,
                0, 0, 0, 1;
    }
    else if(axis == "y")
    {
        mat << cos(ang), 0, sin(ang), 0,
                0, 1, 0, 0,
               -sin(ang), 0, cos(ang), 0,
                0, 0, 0, 1;
    }
    else if(axis == "z")
    {
        mat << cos(ang), -sin(ang), 0, 0,
              sin(ang), cos(ang), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
    }
    else
    {
        mat <<  1,0,0,0,
               0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
    }

    //! Rotate around the center of mass of the model
    mat = mat * transformationToCenterOfMass;//* transformationToCenterOfMass.inverse().eval();

    return mat;
}
