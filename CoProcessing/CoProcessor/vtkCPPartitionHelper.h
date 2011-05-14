/*=========================================================================

  Program:   ParaView
  Module:    vtkCPPartitionHelper.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef vtkCPPartitionHelper_h
#define vtkCPPartitionHelper_h

#include "vtkObject.h"
#include "CPWin32Header.h" // For windows import/export of shared libraries

class vtkIntArray;
class vtkStringArray;
class vtkMultiProcessController;


/// @ingroup CoProcessing
/// This class provides utility methods for computing process partitions
class COPROCESSING_EXPORT vtkCPPartitionHelper : public vtkObject
{
public:
  static vtkCPPartitionHelper* New();
  vtkTypeMacro(vtkCPPartitionHelper,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Description:
  static void ComputePSetRanks(vtkIntArray* ranks);


  static void SendMessages(vtkStringArray* sendArray, vtkStringArray* receiveArray,
                              vtkMultiProcessController* controller, int receivePid);


//BTX
protected:
  vtkCPPartitionHelper();
  virtual ~vtkCPPartitionHelper();

private:
  vtkCPPartitionHelper(const vtkCPPartitionHelper&); // Not implemented
  void operator=(const vtkCPPartitionHelper&); // Not implemented

  class vtkInternals;
  vtkInternals* Internals;
//ETX
};

#endif
