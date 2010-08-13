/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDebugFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDebugFilter.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

//#define myprint(x) printf(x)
#define myprint(x)

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDebugFilter);

//----------------------------------------------------------------------------
vtkDebugFilter::vtkDebugFilter()
{
}

//----------------------------------------------------------------------------
vtkDebugFilter::~vtkDebugFilter()
{
}

//----------------------------------------------------------------------------
int vtkDebugFilter::ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector)
{
  myprint("vtkDebugFilter::ProcessRequest\n");
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkDebugFilter::RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector)
{
  myprint("vtkDebugFilter::RequestInformation\n");



  int res = this->Superclass::RequestInformation(request, inputVector, outputVector);


  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  outInfo->CopyEntry(inInfo, vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  return res;
}

//----------------------------------------------------------------------------
int vtkDebugFilter::RequestUpdateExtent(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector)
{
  myprint("vtkDebugFilter::RequestUpdateExtent\n");
  int res = this->Superclass::RequestUpdateExtent(request, inputVector, outputVector);




  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  //inInfo->CopyEntry(outInfo, vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  //inInfo->CopyEntry(outInfo, vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  return res;
}

//----------------------------------------------------------------------------
int vtkDebugFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  return this->Superclass::FillOutputPortInformation(port, info);
}

//----------------------------------------------------------------------------
int vtkDebugFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  myprint("vtkDebugFilter::RequestData\n");
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);


  //inInfo->Print(cout);
  //myprint("------------------\n");
  //outInfo->Print(cout);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //output->ShallowCopy(input);

  // This has to be here because it initialized all field datas.
  output->CopyStructure( input );
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  return 1;
}

//----------------------------------------------------------------------------
void vtkDebugFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
