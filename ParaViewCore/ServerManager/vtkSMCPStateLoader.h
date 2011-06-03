/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCPStateLoader.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMCPStateLoader - state loader for coprocessor
// .SECTION Description
//
#ifndef __vtkSMCPStateLoader_h
#define __vtkSMCPStateLoader_h

#include "vtkSMStateLoader.h"

class vtkPVXMLElement;
class vtkSMSourceProxy;

class VTK_EXPORT vtkSMCPStateLoader : public vtkSMStateLoader
{
public:
  static vtkSMCPStateLoader* New();
  vtkTypeMacro(vtkSMCPStateLoader, vtkSMStateLoader);
  void PrintSelf(ostream& os, vtkIndent indent);


  void Go(const char* str);
  void Go(vtkPVXMLElement* elem);

  int GetNumberOfSources();
  vtkSMSourceProxy* GetSource(int index);
  const char* GetSourceName(int index);

  // Description:
  // If given proxy is a pipeline sink, return the tag, which is an int >= 0.
  // If the proxy is not a pipeline sink, returns -1.
  int GetSinkTag(vtkSMProxy* proxy);

  virtual vtkPVXMLElement* LocateProxyElement(vtkPVXMLElement* root, int id);


protected:
  vtkSMCPStateLoader();
  ~vtkSMCPStateLoader();

  virtual void CreatedNewProxy(int id, vtkSMProxy* proxy);

  virtual bool ShouldSkipProxy(const char* xmlgroup, const char* xmlname);

  // Description:
  virtual vtkSMProxy* CreateProxy(const char* xmlgroup, const char* xmlname);

  // Overridden to avoid registering the reused rendermodules twice.
  virtual void RegisterProxyInternal(const char* group,
    const char* name, vtkSMProxy* proxy);

private:

  //BTX
  class vtkInternal;
  vtkInternal *Internal;
  //ETX

  vtkSMCPStateLoader(const vtkSMCPStateLoader&); // Not implemented.
  void operator=(const vtkSMCPStateLoader&); // Not implemented.
};

#endif
