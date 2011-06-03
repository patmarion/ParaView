/*=========================================================================

  Program:   ParaView
  Module:    vtkCoProcessorTransfer.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkCoProcessorTransfer.h"

#include "vtkClientServerInterpreter.h"
#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkIntArray.h"

#include "vtkSmartPointer.h"

#include "vtkCollection.h"
#include "vtkCollectionIterator.h"
#include "vtkNetworkAccessManager.h"
#include "vtkPVFileInformation.h"
#include "vtkPVFileInformationHelper.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLParser.h"
#include "vtkProcessModule.h"
#include "vtkMultiProcessController.h"
//#include "vtkCoProcessorConnection.h"
//#include "vtkRemoteConnection.h"
#include "vtkMultiProcessStream.h"
#include "vtkDataObject.h"
#include "vtkDataObjectCollection.h"
#include "vtkCommand.h"
#include "vtkSocketCommunicator.h"
#include "vtkClientSocket.h"

#include <vtkstd/string>
#include <vtkstd/vector>
#include <vtkstd/set>
#include <vtkstd/map>
#include <vtkstd/algorithm>
#include <vtksys/ios/sstream>

#if 0
#define myprint(msg)
#else
#define myprint(msg) \
    std::cout << "proc(" \
              << vtkProcessModule::GetProcessModule()->GetPartitionId() \
              << ") " << msg << endl;
#endif

namespace {

const int RMITag = 54647;
const int CommTag = 45546;

}

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCoProcessorTransfer);

//-----------------------------------------------------------------------------
class vtkCoProcessorTransfer::vtkInternal
{
  public:
  vtkInternal()
    {
    this->ConnectionId = 0;
    }

  ~vtkInternal()
    {
    }

  vtkIdType ConnectionId;
  vtkSmartPointer<vtkSocketCommunicator> SocketCommunicator;

  vtkSmartPointer<vtkMultiProcessController> CommandController;
};

//-----------------------------------------------------------------------------
vtkCoProcessorTransfer::vtkCoProcessorTransfer()
{
  this->Internal = new vtkInternal;
  this->Host = 0;
  this->Port = 0;
}

//-----------------------------------------------------------------------------
vtkCoProcessorTransfer::~vtkCoProcessorTransfer()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
int vtkCoProcessorTransfer::CoProcessorConnectToServer(const char* host, int port)
{
  this->SetPort(port);
  this->SetHost(host);


  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = vtkProcessModule::GetProcessModule()->GetPartitionId();
  vtkMultiProcessController* sateliteController = pm->GetGlobalController();

  vtkMultiProcessController* socketController = 0;

  if (pid == 0)
    {
    myprint("connecting to pvserver at " << host << ":" << port);


    std::stringstream urlStream;
    urlStream << "tcp://" << host << ":" << port;
    this->Internal->CommandController = pm->GetNetworkAccessManager()->NewConnection(urlStream.str().c_str());

    myprint("command controller is " << &this->Internal->CommandController);
    if (this->Internal->CommandController)
      {
      this->Internal->ConnectionId = 1;
      }
    }

  sateliteController->Broadcast(&this->Internal->ConnectionId, 1, 0);
  if (!this->Internal->ConnectionId)
    {
    return 0;
    }


  vtkMultiProcessStream infoStream;
  if (pid == 0)
    {
    myprint("sending command: send_info");
    std::string command = "send_info";
    this->Internal->CommandController->TriggerRMI(1, const_cast<char*>(command.c_str()), command.size()+1, RMITag);
    this->Internal->CommandController->Receive(infoStream, 1, CommTag);
    }

  sateliteController->Broadcast(infoStream, 0);

  int numberOfPartitions = 0;
  infoStream >> numberOfPartitions;

  myprint("received broadcasted number of partitions: " << numberOfPartitions);
  return numberOfPartitions;
}


//----------------------------------------------------------------------------
void vtkCoProcessorTransfer::CoProcessorMakeConnections(vtkIntArray* procIds)
{

  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = vtkProcessModule::GetProcessModule()->GetPartitionId();


  vtkMultiProcessStream connectionInfo;
  if (pid == 0)
    {
    std::string command = "connect_multi";
    this->Internal->CommandController->TriggerRMI(1, const_cast<char*>(command.c_str()), command.size()+1, RMITag);
    this->Internal->CommandController->Receive(connectionInfo, 1, CommTag);
    }

  pm->GetGlobalController()->Broadcast(connectionInfo, 0);

  int indexInPartition = 0;
  for ( ; indexInPartition < procIds->GetNumberOfTuples(); ++indexInPartition)
    {
    if (procIds->GetValue(indexInPartition) == pid)
      {
      break;
      }
    }

  // This process is not participating
  if (indexInPartition >= procIds->GetNumberOfTuples())
    {
    return;
    }

  vtkstd::string host = this->Host;
  int numberOfMachines;
  connectionInfo >> numberOfMachines;
  for (int i = 0; i < numberOfMachines; ++i)
    {
    vtkstd::string hostFromStream;
    connectionInfo >> hostFromStream;
    myprint("received_host " << i << " " << hostFromStream);
    if (i == indexInPartition)
      {
      myprint("using_host: " << hostFromStream);
      host = hostFromStream;
      break;
      }
    }


  int port = this->Port + indexInPartition + 1;
  myprint("connect_to_server_node: " << host << " " << port);

  this->Internal->SocketCommunicator = vtkSmartPointer<vtkSocketCommunicator>::New();
  this->Internal->SocketCommunicator->ConnectTo(const_cast<char*>(host.c_str()), port);
}

//----------------------------------------------------------------------------
void vtkCoProcessorTransfer::SendState(vtkPVXMLElement* elem)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = pm->GetPartitionId();
  if (pid != 0)
    {
    return;
    }

  if (!this->Internal->ConnectionId)
    {
    vtkErrorMacro("Connection ID is null");
    return;
    }

  myprint("send_state");

  std::string command = "statercv";
  this->Internal->CommandController->TriggerRMI(1, const_cast<char*>(command.c_str()), command.size()+1, RMITag);

  vtksys_ios::ostringstream ostr;
  elem->PrintXML(ostr, vtkIndent());

  vtkMultiProcessStream stream;
  stream << ostr.str();
  this->Internal->CommandController->Send(stream, 1, CommTag);
}

//----------------------------------------------------------------------------
void vtkCoProcessorTransfer::Disconnect()
{
  myprint("disconnect");
  // todo - tell process module to drop the connection
  // for now we don't need to because we only call Disconnect after
  // the connection has already been dropped.
  this->Internal->ConnectionId = 0;
  this->Internal->SocketCommunicator = 0;
}

//----------------------------------------------------------------------------
vtkPVXMLElement* vtkCoProcessorTransfer::ReceiveState()
{
  myprint("receive_state");

  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = pm->GetPartitionId();


  vtkMultiProcessStream stream;
  if (pid == 0)
    {
    if (!this->Internal->ConnectionId)
      {
      vtkErrorMacro("Connection ID is null");
      return 0;
      }

    std::string command = "statesend";
    this->Internal->CommandController->TriggerRMI(1, const_cast<char*>(command.c_str()), command.size()+1, RMITag);

    int result = this->Internal->CommandController->Receive(stream, 1, CommTag);
    if (!result)
      {
      myprint("receive_state, connection is dead");
      stream << "<msg><disconnect/></msg>";
      }
    }

  vtkMultiProcessController* sateliteController = pm->GetGlobalController();
  sateliteController->Broadcast(stream, 0);

  vtkstd::string xmlStr;
  stream >> xmlStr;
  if (!xmlStr.length())
    {
    return 0;
    }

  //myprint("  received:\n" << xmlStr.c_str());

  vtkSmartPointer<vtkPVXMLParser> parser = vtkSmartPointer<vtkPVXMLParser>::New();
  parser->Parse(xmlStr.c_str());
  vtkPVXMLElement* elem = parser->GetRootElement();

  if (!elem)
    {
    vtkErrorMacro("Error parsing received xml string");
    return 0;
    }

  elem->Register(0);
  return elem;
}

//----------------------------------------------------------------------------
void vtkCoProcessorTransfer::SendExtractsCommand()
{
  myprint("send_extracts_command");
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = pm->GetPartitionId();

  if (pid == 0)
    {
    if (!this->Internal->ConnectionId)
      {
      vtkErrorMacro("Null connection id");
      return;
      }

    std::string command = "rcv";
    this->Internal->CommandController->TriggerRMI(1, const_cast<char*>(command.c_str()), command.size()+1, RMITag);
    }
}

//----------------------------------------------------------------------------
void vtkCoProcessorTransfer::SendExtracts(vtkDataObjectCollection* extracts,
                                          vtkIntArray* extractTags,
                                          vtkIdType timestep, double time)
{
  myprint("send_extracts");

  if (!this->Internal->SocketCommunicator)
    {
    vtkErrorMacro("Socket communicator is null");
    return;
    }

  if (!extracts || !extractTags)
    {
    vtkErrorMacro("Given null arguments");
    return;
    }

  int numberOfExtracts = extracts->GetNumberOfItems();
  if (numberOfExtracts != extractTags->GetNumberOfTuples())
    {
    vtkErrorMacro("Mismatch number of extracts and tags.");
    return;
    }

  this->Internal->SocketCommunicator->Send(&timestep, 1, 1, 9998);
  this->Internal->SocketCommunicator->Send(&time, 1, 1, 9998);
  this->Internal->SocketCommunicator->Send(&numberOfExtracts, 1, 1, 9998);

  for (int i = 0; i < numberOfExtracts; ++i)
    {
    int extractTag = extractTags->GetValue(i);
    vtkDataObject* dataObject = extracts->GetItem(i);
    this->Internal->SocketCommunicator->Send(&extractTag, 1, 1, 9998);
    this->Internal->SocketCommunicator->Send(dataObject, 1, 9999);
    }

  if (!this->Internal->SocketCommunicator->GetSocket()->GetConnected())
    {
    myprint("send_extract, connection is dead");
    this->Internal->SocketCommunicator = 0;
    }
}

//-----------------------------------------------------------------------------
void vtkCoProcessorTransfer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
