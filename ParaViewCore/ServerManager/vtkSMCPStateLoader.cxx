/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCPStateLoader.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMCPStateLoader.h"

#include "vtkObjectFactory.h"
#include "vtkSMProxyLocator.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLParser.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
//#include "vtkProcessModuleConnectionManager.h"
#include "vtkSmartPointer.h"
#include <vtkstd/vector>
#include <vtkstd/map>
#include <vtkstd/algorithm>
#include <vtksys/ios/sstream>


//-----------------------------------------------------------------------------
class vtkSMCPStateLoader::vtkInternal
{
public:
  vtkstd::vector<vtkSMSourceProxy*> Sources;
  vtkstd::vector<vtkstd::string> SourceNames;

  // Maps pipeline sinks to sink tags
  vtkstd::map<vtkSMProxy*, int> Sinks;
};


//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSMCPStateLoader);

//-----------------------------------------------------------------------------
vtkSMCPStateLoader::vtkSMCPStateLoader()
{
  this->Internal = new vtkInternal;
}

//-----------------------------------------------------------------------------
vtkSMCPStateLoader::~vtkSMCPStateLoader()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
int vtkSMCPStateLoader::GetNumberOfSources()
{
  return static_cast<int>(this->Internal->Sources.size());
}

//-----------------------------------------------------------------------------
bool vtkSMCPStateLoader::ShouldSkipProxy(const char* xmlgroup, const char* xmlname)
{
  if (xmlgroup && xmlname)
    {
    if (!strcmp(xmlgroup, "animation") && !strcmp(xmlname, "AnimationScene"))
      {
      return true;
      }

    if (!strcmp(xmlgroup, "misc") && !strcmp(xmlname, "TimeKeeper"))
      {
      return true;
      }
    }

  return this->Superclass::ShouldSkipProxy(xmlgroup, xmlname);
}

//-----------------------------------------------------------------------------
vtkSMSourceProxy* vtkSMCPStateLoader::GetSource(int index)
{
  if (index >= 0 && index < static_cast<int>(this->Internal->Sources.size()))
    {
    return this->Internal->Sources[index];
    }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkSMCPStateLoader::GetSinkTag(vtkSMProxy* proxy)
{
  vtkstd::map<vtkSMProxy*, int>::const_iterator itr = this->Internal->Sinks.find(proxy);
  if (itr != this->Internal->Sinks.end())
    {
    return itr->second;
    }

  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkSMCPStateLoader::GetSourceName(int index)
{
  if (index >= 0 && index < static_cast<int>(this->Internal->SourceNames.size()))
    {
    return this->Internal->SourceNames[index].c_str();
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkSMCPStateLoader::Go(vtkPVXMLElement* rootElement)
{
  if (!rootElement)
    {
    return;
    }

  //this->GetProxyLocator()->SetConnectionID(vtkProcessModuleConnectionManager::GetNullConnectionID());
  this->LoadState(rootElement);
}

//-----------------------------------------------------------------------------
void vtkSMCPStateLoader::Go(const char* str)
{
  if (!str || !str[0])
    {
    return;
    }

  vtkPVXMLParser* parser = vtkPVXMLParser::New();
  parser->Parse(str);
  this->Go(parser->GetRootElement());
  parser->Delete();
}

//---------------------------------------------------------------------------
vtkPVXMLElement* vtkSMCPStateLoader::LocateProxyElement(vtkPVXMLElement* root, int id)
{
  return this->LocateProxyElementInternal(root, id);
}

//---------------------------------------------------------------------------
void vtkSMCPStateLoader::CreatedNewProxy(int id, vtkSMProxy* proxy)
{
  this->RegisterProxy(id, proxy);
  vtkPVXMLElement* proxyElement = this->Superclass::LocateProxyElement(id);
  if (proxyElement)
    {
    int sinkTag;
    if (proxyElement->GetScalarAttribute("sink_tag", &sinkTag))
      {
      this->Internal->Sinks[proxy] = sinkTag;
      }
    }
  //return this->Superclass::CreatedNewProxy(id, proxy);
}

//-----------------------------------------------------------------------------
vtkSMProxy* vtkSMCPStateLoader::CreateProxy(
  const char* xml_group, const char* xml_name)
{
  return this->Superclass::CreateProxy(xml_group, xml_name);
}


//---------------------------------------------------------------------------
void vtkSMCPStateLoader::RegisterProxyInternal(const char* group,
  const char* name, vtkSMProxy* proxy)
{
  if (!strcmp(group, "sources"))
    {
    vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(proxy);
    if (sourceProxy)
      {
      this->Internal->SourceNames.push_back(name);
      this->Internal->Sources.push_back(sourceProxy);
      }
    }
  //this->Superclass::RegisterProxyInternal(group, name, proxy);
}

//-----------------------------------------------------------------------------
void vtkSMCPStateLoader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
