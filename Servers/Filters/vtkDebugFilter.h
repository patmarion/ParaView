/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDebugFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDebugFilter - Filter which shallow copies it's input to it's output
// .SECTION Description
//

#ifndef __vtkDebugFilter_h
#define __vtkDebugFilter_h

#include "vtkDataSetAlgorithm.h"

class vtkFieldData;

class VTK_PARALLEL_EXPORT vtkDebugFilter : public vtkDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkDebugFilter,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a new vtkDebugFilter.
  static vtkDebugFilter *New();

  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);


protected:

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  vtkDebugFilter();
  virtual ~vtkDebugFilter();

private:
  vtkDebugFilter(const vtkDebugFilter&);  // Not implemented.
  void operator=(const vtkDebugFilter&);  // Not implemented.
};

#endif


