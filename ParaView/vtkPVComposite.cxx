/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPVComposite.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1998-1999 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkPVComposite.h"
#include "vtkKWLabel.h"
#include "vtkPVApplication.h"
#include "vtkKWView.h"
#include "vtkPVWindow.h"

//----------------------------------------------------------------------------
vtkPVComposite::vtkPVComposite()
{
  this->Notebook = vtkKWNotebook::New();
  this->Source = NULL;
  this->Window = NULL;
  this->CompositeName = NULL;
  
  this->NotebookCreated = 0;
  this->Name = NULL;
}

//----------------------------------------------------------------------------
vtkPVComposite* vtkPVComposite::New()
{
  return new vtkPVComposite();
}

//----------------------------------------------------------------------------
vtkPVComposite::~vtkPVComposite()
{
  this->Notebook->SetParent(NULL);
  this->Notebook->Delete();
  this->Notebook = NULL;
  
  this->SetSource(NULL);
<<<<<<< vtkPVComposite.cxx

  this->SetName(NULL);
=======
  this->SetCompositeName(NULL);
>>>>>>> 1.7
}


//----------------------------------------------------------------------------
void vtkPVComposite::Clone(vtkPVApplication *pvApp)
{
  if (this->Application)
    {
    vtkErrorMacro("Application has already been set.");
    }
  this->SetApplication(pvApp);
  
  // Clone this object on every other process.
  pvApp->BroadcastScript("%s %s", this->GetClassName(), this->GetTclName());
}


//----------------------------------------------------------------------------
void vtkPVComposite::SetWindow(vtkPVWindow *window)
{
  if (this->Window == window)
    {
    return;
    }
  this->Modified();

  if (this->Window)
    {
    vtkPVWindow *tmp = this->Window;
    this->Window = NULL;
<<<<<<< vtkPVComposite.cxx
    //tmp->UnRegister(this);
=======
    //Register and UnRegister are commented out because including
    //these lines causes ParaView to crash when you try to exit.
    //We think it is probably some weirdness with reference counting.
//    tmp->UnRegister(this);
>>>>>>> 1.6
    }
  if (window)
    {
    this->Window = window;
<<<<<<< vtkPVComposite.cxx
    //window->Register(this);
=======
//    window->Register(this);
>>>>>>> 1.6
    }
}

//----------------------------------------------------------------------------
vtkPVWindow *vtkPVComposite::GetWindow()
{
  return this->Window;
}

//----------------------------------------------------------------------------
void vtkPVComposite::Select(vtkKWView *view)
{
}

//----------------------------------------------------------------------------
void vtkPVComposite::Deselect(vtkKWView *view)
{
}

//----------------------------------------------------------------------------
vtkProp* vtkPVComposite::GetProp()
{
  vtkPVData *data = this->GetPVData();

  if (data == NULL)
    {
    return NULL;
    }
  return data->GetProp();
}

//----------------------------------------------------------------------------
void vtkPVComposite::CreateProperties(char *args)
{ 
  const char *dataPage, *sourcePage;
  vtkPVData *data = this->GetPVData();
  vtkPVApplication *app = this->GetPVApplication();

  if (data == NULL || this->Source == NULL)
    {
    vtkErrorMacro("You need to set the data and source before you create a composite");
    return;
    }

  // Create widgets.
  if (this->NotebookCreated)
    {
    return;
    }
  this->NotebookCreated = 1;
  
  this->Notebook->Create(app, args);
  sourcePage = this->Source->GetClassName();
  this->Notebook->AddPage(sourcePage);
  dataPage = data->GetClassName();
  this->Notebook->AddPage(dataPage);
  
  this->Script("pack %s -pady 2 -padx 2 -fill both -expand yes -anchor n",
               this->Notebook->GetWidgetName());
    
  data->SetParent(this->Notebook->GetFrame(dataPage));
  data->Create("");
  this->Script("pack %s", data->GetWidgetName());

  this->Source->SetParent(this->Notebook->GetFrame(sourcePage));
  this->Source->Create("");
  this->Script("pack %s", this->Source->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkPVComposite::SetPropertiesParent(vtkKWWidget *parent)
{
  this->Notebook->SetParent(parent);
}

//----------------------------------------------------------------------------
vtkKWWidget *vtkPVComposite::GetPropertiesParent()
{
  return this->Notebook->GetParent();
}

//----------------------------------------------------------------------------
vtkKWWidget *vtkPVComposite::GetProperties()
{
  return this->Notebook;
}

//----------------------------------------------------------------------------
void vtkPVComposite::SetSource(vtkPVSource *source)
{
  if (this->Source == source)
    {
    return;
    }
  this->Modified();

  if (this->Source)
    {
    vtkPVSource *tmp = this->Source;
    this->Source = NULL;
    tmp->SetComposite(NULL);
    tmp->UnRegister(this);
    }
  if (source)
    {
    this->Source = source;
    source->Register(this);
    source->SetComposite(this);
    
    // Make the assignment in all of the processes.
    vtkPVApplication *pvApp = this->GetPVApplication();
    if (pvApp && pvApp->GetController()->GetLocalProcessId() == 0)
      {
      pvApp->BroadcastScript("%s SetSource %s", this->GetTclName(),
			     source->GetTclName());
      }
    }
}
<<<<<<< vtkPVComposite.cxx

void vtkPVComposite::SetData(vtkPVData *data)
{
  if (this->Source == NULL)
    {
    vtkErrorMacro("Source is not set yet");
    return;
    }
  this->Modified();

  if (this->Data)
    {
    vtkPVPolyData *tmp = this->Data;
    this->Data = NULL;
    tmp->SetComposite(NULL);
    tmp->UnRegister(this);
    }
<<<<<<< vtkPVComposite.cxx
  if (data)
=======
  this->Source->SetDataWidget(data);
}
=======
>>>>>>> 1.9

//----------------------------------------------------------------------------
char* vtkPVComposite::GetCompositeName()
{
  return this->CompositeName;
}

//----------------------------------------------------------------------------
void vtkPVComposite::SetCompositeName (const char* arg) 
{ 
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting " 
                << this->CompositeName << " to " << arg ); 
  if ( this->CompositeName && arg && (!strcmp(this->CompositeName,arg))) 
    { 
    return;
    } 
  if (this->CompositeName) 
    { 
    delete [] this->CompositeName; 
    } 
  if (arg) 
    { 
    this->CompositeName = new char[strlen(arg)+1]; 
    strcpy(this->CompositeName,arg); 
    } 
  else 
    { 
    this->CompositeName = NULL;
    }
  this->Modified(); 
} 
 
//----------------------------------------------------------------------------
vtkPVData *vtkPVComposite::GetPVData()
{
  if (this->Source == NULL)
>>>>>>> 1.6
    {
    return NULL;
    }
<<<<<<< vtkPVComposite.cxx
<<<<<<< vtkPVComposite.cxx
=======
  return this->Source->GetPVData();
>>>>>>> 1.9
}
<<<<<<< vtkPVComposite.cxx
=======
  return this->Source->GetDataWidget();
}
>>>>>>> 1.6
=======


//----------------------------------------------------------------------------
void vtkPVComposite::SetVisibility(int v)
{
  vtkProp * p = this->GetProp();
  vtkPVApplication *pvApp;
  
  if (p)
    {
    p->SetVisibility(v);
    }
  
  pvApp = (vtkPVApplication*)(this->Application);
  
  // Make the assignment in all of the processes.
  if (pvApp && pvApp->GetController()->GetLocalProcessId() == 0)
    {
    pvApp->BroadcastScript("%s SetVisibility %d", this->GetTclName(), v);
    }
}

  
//----------------------------------------------------------------------------
int vtkPVComposite::GetVisibility()
{
  vtkProp *p = this->GetProp();
  
  if (p == NULL)
    {
    return 0;
    }
  
  return p->GetVisibility();
}


//----------------------------------------------------------------------------
vtkPVApplication* vtkPVComposite::GetPVApplication()
{
  if (this->Application == NULL)
    {
    return NULL;
    }
  
  if (this->Application->IsA("vtkPVApplication"))
    {  
    return (vtkPVApplication*)(this->Application);
    }
  else
    {
    vtkErrorMacro("Bad typecast");
    return NULL;
    } 
}

>>>>>>> 1.9
