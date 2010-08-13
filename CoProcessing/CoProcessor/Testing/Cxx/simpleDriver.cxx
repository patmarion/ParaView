
#include "vtkCPProcessor.h"
#include "vtkCPPythonScriptPipeline.h"
#include "vtkMultiProcessController.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCPDataDescription.h"
#include "vtkCPInputDataDescription.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"

#include <stdio.h>
#include <string>
#include <sstream>

static unsigned int procId;

void myprint(const std::string& str)
{
  printf("driver (%u): %s\n", procId, str.c_str());
}

class DataGenerator {
public:

  DataGenerator()
    {
    this->Sphere = vtkSmartPointer<vtkSphereSource>::New();
    this->Sphere->SetThetaResolution(30);
    this->Sphere->SetPhiResolution(30);
    this->Sphere->SetCenter(procId*4.0, 0, 0);
    this->Index = 0;
    }

  vtkSmartPointer<vtkPolyData> GetNext()
    {
    double radius = fabs(sin(0.1 * this->Index));
    this->Index++;
    this->Sphere->SetRadius(1.0 + radius);
    this->Sphere->Update();
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    ret->DeepCopy(this->Sphere->GetOutput());
    return ret;
    }

protected:

  int Index;
  vtkSmartPointer<vtkSphereSource> Sphere;
  

};

int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    printf("Usage: %s <cp python file> <number of steps>\n", argv[0]);
    return 1;
    }

  std::string cpPythonFile = argv[1];
  int nSteps = atoi(argv[2]);

  myprint("starting coprocessor");

  vtkCPProcessor* processor = vtkCPProcessor::New();
  processor->Initialize();
  vtkCPPythonScriptPipeline* pipeline = vtkCPPythonScriptPipeline::New();

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
  pipeline->Delete();

  if (nSteps == 0)
    {
    return 0;
    }

  // create a data source
  DataGenerator generator;

  // do coprocessing
  double tStart = 0.0;
  double tEnd = 1.0;
  double stepSize = (tEnd - tStart)/nSteps;

  vtkCPDataDescription* dataDesc = vtkCPDataDescription::New();
  dataDesc->AddInput("input");

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

      vtkSmartPointer<vtkDataObject> dataObject =
        generator.GetNext();

      dataDesc->GetInputDescriptionByName("input")->SetGrid(dataObject);
      processor->CoProcess(dataDesc);
      }
    }


  myprint("finalizing");
  dataDesc->Delete();
  processor->Finalize();
  processor->Delete();

  return 0;
}


