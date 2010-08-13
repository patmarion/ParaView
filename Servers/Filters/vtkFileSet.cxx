/*=========================================================================

  Program:   ParaView
  Module:    vtkFileSet.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkFileSet.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkSmartPointer.h"

#include "vtkCollection.h"
#include "vtkCollectionIterator.h"
#include "vtkPVFileInformation.h"
#include "vtkPVFileInformationHelper.h"

#include <vtkstd/set>
#include <vtkstd/vector>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkFileSet);

//-----------------------------------------------------------------------------
class vtkFileSet::vtkInternal
{
  public:
  vtkInternal()
    {
    this->FileNamesArray = vtkSmartPointer<vtkStringArray>::New();
    }

  ~vtkInternal()
    {
    }

  vtkstd::set<vtkstd::string> CurrentFileSet;
  vtkstd::vector<vtkstd::string> FileNames;
  vtkSmartPointer<vtkStringArray> FileNamesArray;
};

//-----------------------------------------------------------------------------
vtkFileSet::vtkFileSet()
{
  this->Internal = new vtkInternal;
  this->BaseDirectory = 0;
}

//-----------------------------------------------------------------------------
vtkFileSet::~vtkFileSet()
{
  delete this->Internal;
  this->SetBaseDirectory(0);
}

//----------------------------------------------------------------------------
vtkStringArray* vtkFileSet::GetFileNames()
{
  this->Internal->FileNamesArray->SetNumberOfValues(this->GetNumberOfFileNames());
  for (unsigned int i = 0; i < this->GetNumberOfFileNames(); ++i)
    {
    this->Internal->FileNamesArray->SetValue(i, this->GetFileName(i));
    }
  return this->Internal->FileNamesArray;
}

//----------------------------------------------------------------------------
void vtkFileSet::AddFileName(const char* name)
{
  this->Internal->FileNames.push_back(name);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkFileSet::RemoveAllFileNames()
{
  this->Internal->FileNames.clear();
}

//----------------------------------------------------------------------------
unsigned int vtkFileSet::GetNumberOfFileNames()
{
  return static_cast<unsigned int>(this->Internal->FileNames.size());
}

//----------------------------------------------------------------------------
const char* vtkFileSet::GetFileName(unsigned int idx)
{
  if (idx >= this->Internal->FileNames.size())
    {
    return 0;
    }
  return this->Internal->FileNames[idx].c_str();
}

//-----------------------------------------------------------------------------
int vtkFileSet::PollForNewFiles()
{
  if (!this->BaseDirectory || !this->BaseDirectory[0])
    {
    printf("poll for new files, no base directory\n");
    return 0;
    }

  vtkPVFileInformationHelper* helper = vtkPVFileInformationHelper::New();
  vtkPVFileInformation* dirInfo = vtkPVFileInformation::New();

  helper->SetPath(this->BaseDirectory);
  helper->SetDirectoryListing(1);
  //helper->SetOrganizeGroups(0);
  dirInfo->CopyFromObject(helper);

  vtkstd::set<vtkstd::string>::const_iterator findItr;

  vtkSmartPointer<vtkCollectionIterator> iter;
  iter.TakeReference(dirInfo->GetContents()->NewIterator());

  bool sameFiles = true;
  this->Internal->FileNames.clear();

  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    vtkPVFileInformation* info = vtkPVFileInformation::SafeDownCast(
      iter->GetCurrentObject());
    if (!info)
      {
      continue;
      }
    if (vtkPVFileInformation::IsDirectory(info->GetType()))
      {
      continue;
      }
    else if (info->GetType() != vtkPVFileInformation::FILE_GROUP)
      {
      this->Internal->FileNames.push_back(info->GetFullPath());

      if (sameFiles && this->Internal->CurrentFileSet.find(info->GetFullPath())
                        == this->Internal->CurrentFileSet.end())
        {
        sameFiles = false;
        }
      }
    else if (info->GetType() == vtkPVFileInformation::FILE_GROUP)
      {
      vtkSmartPointer<vtkCollectionIterator> childIter;
      childIter.TakeReference(info->GetContents()->NewIterator());
      for (childIter->InitTraversal(); !childIter->IsDoneWithTraversal();
                                                  childIter->GoToNextItem())
        {
        vtkPVFileInformation* child = vtkPVFileInformation::SafeDownCast(
          childIter->GetCurrentObject());


        this->Internal->FileNames.push_back(child->GetFullPath());
        if (sameFiles && this->Internal->CurrentFileSet.find(child->GetFullPath())
                          == this->Internal->CurrentFileSet.end())
          {
          sameFiles = false;
          }

        }
      }
    }

  helper->Delete();
  dirInfo->Delete();

  if (!sameFiles)
    {
    this->Modified();
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkFileSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  for (unsigned int i = 0; i < this->GetNumberOfFileNames(); ++i)
    {
    os << indent << "File " << i << ": " << this->GetFileName(i) << endl;
    }
}
