/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDebugSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDebugSource - Filter which shallow copies it's input to it's output
// .SECTION Description
//

#ifndef __vtkDebugSource_h
#define __vtkDebugSource_h

#include "vtkDataSetAlgorithm.h"

class vtkFieldData;

class VTK_PARALLEL_EXPORT vtkDebugSource : public vtkAlgorithm
{
public:
  vtkTypeMacro(vtkDebugSource, vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a new vtkDebugSource.
  static vtkDebugSource *New();

  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

  vtkSetMacro(Radius, double);

protected:

  double Radius;

  virtual int RequestDataObject(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  vtkDebugSource();
  virtual ~vtkDebugSource();

private:
  vtkDebugSource(const vtkDebugSource&);  // Not implemented.
  void operator=(const vtkDebugSource&);  // Not implemented.
};

#endif


