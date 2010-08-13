/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDebugSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDebugSource.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSphereSource.h"
#include "vtkSmartPointer.h"

//#define myprint(x) printf(x)
#define myprint(x)

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDebugSource);

//----------------------------------------------------------------------------
vtkDebugSource::vtkDebugSource()
{
  this->Radius = 1;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkDebugSource::~vtkDebugSource()
{
}

//----------------------------------------------------------------------------
int vtkDebugSource::ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector)
{
  myprint("vtkDebugSource::ProcessRequest\n");


  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(request, inputVector, outputVector);
    }
  else if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->RequestInformation(request, inputVector, outputVector);
    }
  else if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }
 else if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
    }
  else
    {
    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
    }
}

//----------------------------------------------------------------------------
int vtkDebugSource::RequestDataObject(vtkInformation*, 
                                      vtkInformationVector** inputVector , 
                                      vtkInformationVector* outputVector)
{

  /*
  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(info->Get(vtkDataObject::DATA_OBJECT()));
    
  if (!output) 
    {
    vtkPolyData* newOutput = vtkPolyData::New();
    newOutput->SetPipelineInformation(info);
    newOutput->Delete();
    }
  */
  return 1;
}

//----------------------------------------------------------------------------
int vtkDebugSource::RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector)
{
  myprint("vtkDebugSource::RequestInformation\n");

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  double timeSteps[5] = {0, 1, 2, 3, 4};
  double timeRange[2] = {0, 4};
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), timeSteps, 5);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);


  //vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  //outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  //outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  return 1;
}

//----------------------------------------------------------------------------
int vtkDebugSource::RequestUpdateExtent(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector)
{
  myprint("vtkDebugSource::RequestUpdateExtent\n");

  //vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  //inInfo->CopyEntry(outInfo, vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  //inInfo->CopyEntry(outInfo, vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  return 1;
}

//----------------------------------------------------------------------------
int vtkDebugSource::FillInputPortInformation(int port, vtkInformation* info)
{
  //info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  //return 1;
}

//----------------------------------------------------------------------------
int vtkDebugSource::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkDebugSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  myprint("vtkDebugSource::RequestData\n");
  // get the info objects

  //vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  //vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  double timeStep = 0;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    timeStep = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0];

    if (outInfo->Length(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()) > 1)
      {
      printf("More than one timestep requested!\n");
      }
    }
    
  output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &timeStep, 1);


  vtkSphereSource* sphere = vtkSphereSource::New();
  sphere->SetRadius(this->Radius*(timeStep+1));
  sphere->Update();
  output->ShallowCopy(sphere->GetOutput());
  sphere->Delete();

  // This has to be here because it initialized all field datas.
  //output->CopyStructure( input );
  //output->GetPointData()->PassData(input->GetPointData());
  //output->GetCellData()->PassData(input->GetCellData());

  return 1;
}

//----------------------------------------------------------------------------
void vtkDebugSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
