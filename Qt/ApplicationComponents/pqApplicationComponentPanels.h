
#ifndef _pqApplicationComponentPanels_h
#define _pqApplicationComponentPanels_h

#include "pqApplicationComponentsExport.h"
#include "pqObjectPanelInterface.h"

#include "pqLiveSourcePanel.h"
#include "pqObjectPanelInterface.h"
#include "pqProxy.h"
#include "vtkSMProxy.h"

class PQAPPLICATIONCOMPONENTS_EXPORT pqApplicationComponentPanels :
  public QObject, public pqObjectPanelInterface
{
  Q_OBJECT
  Q_INTERFACES(pqObjectPanelInterface)
public:
  pqApplicationComponentPanels(QObject* p) : QObject(p) {}

  QString name() const
    {
    return "ApplicationComponentPanels";
    }

  pqObjectPanel* createPanel(pqProxy* proxy, QWidget* p)
    {
    if(QString("sources") == proxy->getProxy()->GetXMLGroup())
      {
      if(QString("LiveDataSource") == proxy->getProxy()->GetXMLName())
        {
        return new pqLiveSourcePanel(proxy, p);
        }
      }
    return NULL;
    }

  bool canCreatePanel(pqProxy* proxy) const
    {
    if(QString("sources") == proxy->getProxy()->GetXMLGroup())
      {
      if (QString("LiveDataSource") == proxy->getProxy()->GetXMLName())
        {
        return true;
        }
      }
    return false;
    }
};



#endif
