/*=========================================================================

  Program:   ParaView
  Module:    vtkCPPythonScriptPipeline.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCPPythonScriptPipeline.h"

#include "CPSystemInformation.h"
#include "vtkCPDataDescription.h"
#include "vtkCPPythonHelper.h"
#include "vtkDataObject.h"
#include "vtkInitializationHelper.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkPVMain.h"
#include "vtkPVPythonInterpretor.h"
#include "vtkPVPythonOptions.h"
#include "vtkSMProxyManager.h"
#include "vtkSMObject.h"
#include "vtkSMXMLParser.h"

#include <vtkstd/string>
#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>
using vtksys_ios::ostringstream;

vtkCPPythonHelper* vtkCPPythonScriptPipeline::PythonHelper = 0;

vtkStandardNewMacro(vtkCPPythonScriptPipeline);

//----------------------------------------------------------------------------
vtkCPPythonScriptPipeline::vtkCPPythonScriptPipeline()
{
  if(!vtkCPPythonScriptPipeline::PythonHelper)
    {
    this->PythonHelper = vtkCPPythonHelper::New();
    this->PythonHelper->Register(this);
    this->PythonHelper->Delete();
    }
  else
    {
    this->PythonHelper->Register(this);
    }
  this->PythonScriptName = 0;
}

//----------------------------------------------------------------------------
vtkCPPythonScriptPipeline::~vtkCPPythonScriptPipeline()
{
  this->PythonHelper->UnRegister(this);
  this->SetPythonScriptName(0);
}

//----------------------------------------------------------------------------
int vtkCPPythonScriptPipeline::Initialize(const char* fileName)
{
  this->SetPythonScriptName("_cpscriptmodule");

  ostringstream loadPythonModules;
  loadPythonModules
    << "paraview.servermanager._ImportModuleFromProcessZero('"
    << this->PythonScriptName << "', '" << fileName << "')\n"
    << "import " << this->PythonScriptName << "\n";

  this->PythonHelper->GetPythonInterpretor()->RunSimpleString(
    loadPythonModules.str().c_str());
  this->PythonHelper->GetPythonInterpretor()->FlushMessages();
  return 1;
}

//----------------------------------------------------------------------------
int vtkCPPythonScriptPipeline::RequestDataDescription(
  vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("dataDescription is NULL.");
    return 0;
    }
  // check the script to see if it should be run...
  vtkStdString dataDescriptionString = this->GetPythonAddress(dataDescription);

  ostringstream pythonInput;
#ifndef COPROCESSOR_WIN32_BUILD
  // Not on Windows.
  pythonInput << "dataDescription = libvtkCoProcessorPython.vtkCPDataDescription('"
              << dataDescriptionString << "')\n"
              << this->PythonScriptName << ".RequestDataDescription(dataDescription)\n";
#else
  pythonInput << "dataDescription = vtkCoProcessorPython.vtkCPDataDescription('"
              << dataDescriptionString << "')\n"
              << this->PythonScriptName << ".RequestdataDescription(dataDescription)\n";
#endif

  this->PythonHelper->GetPythonInterpretor()->RunSimpleString(pythonInput.str().c_str());
  this->PythonHelper->GetPythonInterpretor()->FlushMessages();
  return dataDescription->GetIfAnyGridNecessary()? 1: 0;
}

//----------------------------------------------------------------------------
int vtkCPPythonScriptPipeline::CoProcess(
  vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("DataDescription is NULL.");
    return 0;
    }
  vtkStdString dataDescriptionString = this->GetPythonAddress(dataDescription);

  ostringstream pythonInput;
  pythonInput 
#ifndef COPROCESSOR_WIN32_BUILD
    // Not on Windows
    << "dataDescription = libvtkCoProcessorPython.vtkCPDataDescription('"
#else
    << "dataDescription = vtkCoProcessorPython.vtkCPDataDescription('"
#endif
    << dataDescriptionString << "')\n"
    << this->PythonScriptName << ".DoCoProcessing(dataDescription)\n";

  this->PythonHelper->GetPythonInterpretor()->RunSimpleString(
    pythonInput.str().c_str());
  this->PythonHelper->GetPythonInterpretor()->FlushMessages();
  return 1;  
}

//----------------------------------------------------------------------------
vtkStdString vtkCPPythonScriptPipeline::GetPythonAddress(void* pointer)
{
  char addressOfPointer[1024];
#ifdef COPROCESSOR_WIN32_BUILD
  sprintf_s(addressOfPointer, "%p", pointer);
#else
  sprintf(addressOfPointer, "%p", pointer);
#endif
  char *aplus = addressOfPointer;
  if ((addressOfPointer[0] == '0') && 
      ((addressOfPointer[1] == 'x') || addressOfPointer[1] == 'X'))
    {
    aplus += 2; //skip over "0x"
    }

  vtkstd::string value = aplus;
  return value;
}

//----------------------------------------------------------------------------
void vtkCPPythonScriptPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PythonHelper: " << this->PythonHelper << "\n";
  os << indent << "PythonScriptName: " << this->PythonScriptName << "\n";
}



