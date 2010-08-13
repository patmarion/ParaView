/*=========================================================================

  Program:   ParaView
  Module:    vtkFileSet.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __vtkFileSet_h
#define __vtkFileSet_h

#include "vtkObject.h"

class vtkStringArray;
class vtkDataObject;
class vtkIntArray;
class vtkPVXMLElement;

class VTK_EXPORT vtkFileSet : public vtkObject
{
public:
  static vtkFileSet* New();
  vtkTypeMacro(vtkFileSet, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Adds names of files to be read. The files are read in the order
  // they are added.
  virtual void AddFileName(const char* fname);

  virtual void RemoveAllFileNames();

  virtual vtkStringArray* GetFileNames();

  // Description:
  // Returns the number of file names added by AddFileName.
  virtual unsigned int GetNumberOfFileNames();

  // Description:
  // Returns the name of a file with index idx.
  virtual const char* GetFileName(unsigned int idx);

  virtual int PollForNewFiles();

  vtkSetStringMacro(BaseDirectory);
  vtkGetStringMacro(BaseDirectory);

protected:
  vtkFileSet();
  ~vtkFileSet();

  char* BaseDirectory;

private:
  vtkFileSet(const vtkFileSet&); // Not implemented.
  void operator=(const vtkFileSet&); // Not implemented.

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX
};

#endif
