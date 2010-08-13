/*=========================================================================

  Program:   ParaView
  Module:    vtkCoProcessorTransfer.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkCoProcessorTransfer_h
#define __vtkCoProcessorTransfer_h

#include "vtkObject.h"

class vtkStringArray;
class vtkDataObject;
class vtkIntArray;
class vtkDataObjectCollection;
class vtkPVXMLElement;

class VTK_EXPORT vtkCoProcessorTransfer : public vtkObject
{
public:
  static vtkCoProcessorTransfer* New();
  vtkTypeMacro(vtkCoProcessorTransfer, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);


  virtual int CoProcessorConnectToServer(const char* host, int port);

  virtual void CoProcessorMakeConnections(vtkIntArray* procIds);

  virtual void SendExtracts(vtkDataObjectCollection* extracts, vtkIntArray* extractTags,
                            vtkIdType timestep, double time);

  virtual void SendState(vtkPVXMLElement* elem);

  virtual void Disconnect();

  virtual vtkPVXMLElement* ReceiveState();

  vtkSetStringMacro(Host);
  vtkGetStringMacro(Host);

  vtkSetMacro(Port, int);
  vtkGetMacro(Port, int);

protected:
  vtkCoProcessorTransfer();
  ~vtkCoProcessorTransfer();

  int Port;
  char* Host;

private:
  vtkCoProcessorTransfer(const vtkCoProcessorTransfer&); // Not implemented.
  void operator=(const vtkCoProcessorTransfer&); // Not implemented.

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX
};

#endif
