
#include "vtkCPProcessor.h"
#include "vtkCPPythonScriptPipeline.h"
#include "vtkMultiProcessController.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCPDataDescription.h"
#include "vtkCPInputDataDescription.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkOBBTree.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"
#include "vtkXMLUnstructuredGridWriter.h"
#include "vtkDistributedDataFilter.h"

#include <stdio.h>
#include <string>
#include <sstream>


static unsigned int procId;

void myprint(const std::string& str)
{
  printf("driver (%u): %s\n", procId, str.c_str());
}

int main(int argc, char* argv[])
{
  if (argc < 4)
    {
    printf("Usage: %s <data file> <cp python file> <number of steps>\n", argv[0]);
    return 1;
    }

  std::string dataFile = argv[1];
  std::string cpPythonFile = argv[2];
  int nSteps = atoi(argv[3]);



  ////////////////////////////
  // initialize coprocessor
  myprint("starting coprocessor");

  vtkSmartPointer<vtkCPProcessor> processor = vtkSmartPointer<vtkCPProcessor>::New();
  processor->Initialize();
  vtkSmartPointer<vtkCPPythonScriptPipeline> pipeline =
    vtkSmartPointer<vtkCPPythonScriptPipeline>::New();

  // mpi was initialized when we called vtkCPPythonScriptPipeline::New()
  procId = vtkMultiProcessController::GetGlobalController()->GetLocalProcessId();

  // read the coprocessing python file
  myprint("loading pipeline python file: " + cpPythonFile);
  int success = pipeline->Initialize(cpPythonFile.c_str());
  if (!success)
    {
    myprint("aborting");
    return 1;
    }

  processor->AddPipeline(pipeline);
  ///////////////////////////////////


  ////////////////////////////////////
  // Load and redistribute data
  vtkSmartPointer<vtkDataObject> d3Input;
  if (procId == 0)
    {
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
      vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(dataFile.c_str());
    reader->Update();
    d3Input = reader->GetOutput();

    if (!reader->GetOutput()->GetNumberOfPoints())
      {
      printf("File does not appear to have any points: %s\n", reader->GetFileName());
      return 1;
      }
    }
  else
    {
    d3Input = vtkSmartPointer<vtkUnstructuredGrid>::New();
    }

  vtkSmartPointer<vtkDistributedDataFilter> d3 =
    vtkSmartPointer<vtkDistributedDataFilter>::New();
  d3->SetInput(d3Input);
  d3->SetController(vtkMultiProcessController::GetGlobalController());
  d3->Update();

  vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();

  transformFilter->SetInputConnection(d3->GetOutputPort());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  // define a point and axis that we'll rotate around
  double center[3] = { -1.34365, -0.899783, 13.0487 };
  double axis[3] = { -3.95467, 6.051383, -14.898 };


  if (nSteps == 0)
    {
    return 0;
    }

  // do coprocessing
  double tStart = 0.0;
  double tEnd = 1.0;
  double stepSize = (tEnd - tStart)/nSteps;

  vtkSmartPointer<vtkCPDataDescription> dataDesc =
    vtkSmartPointer<vtkCPDataDescription>::New();
  dataDesc->AddInput("input");

  int steps_per_loop = 10;
  double two_pi = 6.2831853071795862;

  for (int i = 0; i < nSteps; ++i)
    {

    double currentTime = tStart + stepSize*i;
    std::stringstream timeStr;
    timeStr << "time(" << i << ", " << currentTime << ")";


    dataDesc->SetTimeData(currentTime, i);

    myprint("call RequestDataDescription, " + timeStr.str());
    int do_coprocessing = processor->RequestDataDescription(dataDesc);

    if (do_coprocessing)
      {
      myprint("calling CoProcess, " + timeStr.str());

      transform->Identity();


      //double scale = 1.0 + 0.5*sin(two_pi/steps_per_loop * i);
      //transform->Scale(scale, scale, scale);

      double angle = 360 * (static_cast<double>(i) / steps_per_loop);
      transform->PostMultiply();
      transform->Translate(-center[0], -center[1], -center[2]);
      transform->RotateWXYZ(angle, axis[0], axis[1], axis[2]);
      transform->Translate(center[0], center[1], center[2]);


      transformFilter->Update();

      dataDesc->GetInputDescriptionByName("input")->SetGrid(transformFilter->GetOutput());
      processor->CoProcess(dataDesc);
      }
    }


  myprint("finalizing");
  processor->Finalize();

  return 0;
}


