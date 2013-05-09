/*=========================================================================

   Program: ParaView
   Module:    pqPythonQtPlugin.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef __pqPluginDecorators_h
#define __pqPluginDecorators_h

#include <QObject>
#include "PythonQt.h"

#include "pqApplicationCore.h"
#include "pqLoadDataReaction.h"
#include "pqPVApplicationCore.h"
#include "pqPipelineSource.h"
#include "pqProxy.h"
#include "pqPythonDialog.h"
#include "pqPythonEventFilter.h"
#include "pqPythonManager.h"
#include "pqPythonQtMethodHelpers.h"
#include "pqRenderView.h"
#include "pqRenderViewBase.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqSettings.h"
#include "pqTimeKeeper.h"

class  pqPluginDecorators : public QObject
{
  Q_OBJECT

public:

  pqPluginDecorators(QObject* parent=0) : QObject(parent)
    {
    this->registerClassForPythonQt(&pqApplicationCore::staticMetaObject);
    this->registerClassForPythonQt(&pqLoadDataReaction::staticMetaObject);
    this->registerClassForPythonQt(&pqPVApplicationCore::staticMetaObject);
    this->registerClassForPythonQt(&pqPipelineSource::staticMetaObject);
    this->registerClassForPythonQt(&pqProxy::staticMetaObject);
    this->registerClassForPythonQt(&pqPythonDialog::staticMetaObject);
    this->registerClassForPythonQt(&pqPythonEventFilter::staticMetaObject);
    this->registerClassForPythonQt(&pqPythonManager::staticMetaObject);
    this->registerClassForPythonQt(&pqPythonQtMethodHelpers::staticMetaObject);
    this->registerClassForPythonQt(&pqRenderView::staticMetaObject);
    this->registerClassForPythonQt(&pqRenderViewBase::staticMetaObject);
    this->registerClassForPythonQt(&pqServer::staticMetaObject);
    this->registerClassForPythonQt(&pqServerManagerModel::staticMetaObject);
    this->registerClassForPythonQt(&pqSettings::staticMetaObject);
    this->registerClassForPythonQt(&pqTimeKeeper::staticMetaObject);
    }

  inline void registerClassForPythonQt(const QMetaObject* metaobject)
    {
    PythonQt::self()->registerClass(metaobject, "paraview");
    }

public slots:


  QList<pqPipelineSource*> static_pqLoadDataReaction_loadData()
    {
    return pqLoadDataReaction::loadData();
    }


  pqApplicationCore* static_pqApplicationCore_instance()
    {
    return pqApplicationCore::instance();
    }


  pqPVApplicationCore* static_pqPVApplicationCore_instance()
    {
    return pqPVApplicationCore::instance();
    }


  pqSettings* settings(pqApplicationCore* inst)
    {
    return inst->settings();
    }


  pqPythonManager* pythonManager(pqPVApplicationCore* inst)
    {
    return inst->pythonManager();
    }


  pqPythonDialog* pythonShellDialog(pqPythonManager* inst)
    {
    return inst->pythonShellDialog();
    }


  pqServer* getActiveServer(pqApplicationCore* inst)
    {
    return inst->getActiveServer();
    }


  pqTimeKeeper* getTimeKeeper(pqServer* inst)
    {
    return inst->getTimeKeeper();
    }

  double getTime(pqTimeKeeper* inst)
    {
    return inst->getTime();
    }

  void setTime(pqTimeKeeper* inst, double arg0)
    {
    inst->setTime(arg0);
    }

  int getNumberOfTimeStepValues(pqTimeKeeper* inst)
    {
    return inst->getNumberOfTimeStepValues();
    }


  pqServerManagerModel* getServerManagerModel(pqApplicationCore* inst)
    {
    return inst->getServerManagerModel();
    }


  QWidget* getWidget(pqRenderViewBase* inst)
    {
    return inst->getWidget();
    }


  void resetCamera(pqRenderView* inst)
    {
    inst->resetCamera();
    }


  pqPythonEventFilter* new_pqPythonEventFilter()
    {
    return new pqPythonEventFilter();
    }

  void delete_pqPythonEventFilter(pqPythonEventFilter* inst)
    {
    delete inst;
    }



  pqProxy* static_pqPythonQtMethodHelpers_findProxyItem(pqServerManagerModel* arg0, vtkSMProxy* arg1)
    {
    return pqPythonQtMethodHelpers::findProxyItem(arg0, arg1);
    }


};

#endif
