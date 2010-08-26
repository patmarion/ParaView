/*=========================================================================

  Program:   ParaView
  Module:    vtkLiveDataSource.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLiveDataSource.h"

#include "vtkProcessModule.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkCommand.h"
#include "vtkCoProcessorConnection.h"
#include "vtkSocketCommunicator.h"
#include "vtkMultiProcessController.h"
#include "vtkServerSocket.h"
#include "vtkClientSocket.h"
#include "vtkMultiProcessStream.h"
#include "vtkClientServerStream.h"
#include "vtkProcessModuleConnectionManager.h"
#include "vtkStringArray.h"
#include "vtkPVXMLElement.h"

#include <vtkstd/vector>
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

//-----------------------------------------------------------------------------
class vtkLiveDataSource::vtkConnectionObserver : public vtkCommand
{
public:

  static vtkConnectionObserver* New()
    {
    return new vtkConnectionObserver;
    }

  virtual void Execute(vtkObject* obj, unsigned long event, void* calldata)
    {
    if (this->Source && event == vtkCommand::ConnectionCreatedEvent)
      {
      vtkIdType connectionId = *(reinterpret_cast<vtkIdType*>(calldata));
      vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

      if (vtkCoProcessorConnection::SafeDownCast(pm->GetConnectionFromId(connectionId)))
        {
        myprint("cp_connection_created_event");

        vtkClientServerID csId = pm->GetIDFromObject(this->Source);
        if (!csId.ID)
          {
          vtkGenericWarningMacro("Could not get client-server ID for live data source");
          return;
          }

        this->Source->SetCoProcessorConnectionId(connectionId);
        vtkCoProcessorConnection::RegisterHandlerForConnection(connectionId, csId);
        }
      }
    }

  vtkLiveDataSource* Source;

protected:
  vtkConnectionObserver()
    {
    this->Source = 0;
    }
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLiveDataSource);

//-----------------------------------------------------------------------------
class vtkLiveDataSource::vtkInternal
{
public:
  vtkInternal()
    {
    this->ConnectionIsOpen = false;
    this->NewDataAvailable = false;
    this->SendSinkStatusToCP = false;
    this->NumberOfTimeSteps = 0;
    this->Observer = vtkSmartPointer<vtkConnectionObserver>::New();
    }

  bool ConnectionIsOpen;
  bool NewDataAvailable;
  bool SendSinkStatusToCP;
  int NumberOfTimeSteps;
  vtkSmartPointer<vtkConnectionObserver> Observer;
  vtkSmartPointer<vtkSocketCommunicator> SocketCommunicator;
  vtkSmartPointer<vtkServerSocket>       ServerSocket;

  typedef vtkstd::vector<vtkSmartPointer<vtkDataObject> > DataObjectVectorType;
  typedef vtkstd::map<int, DataObjectVectorType> DataObjectCacheType;
  DataObjectCacheType DataObjectCache;

  vtkstd::vector<double> TimeSteps;
  vtkstd::map<int, int>  SinkStatus;
};

//-----------------------------------------------------------------------------
vtkLiveDataSource::vtkLiveDataSource()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->CPState = 0;
  this->CPStateSend = 0;
  this->CoProcessorConnectionId = 0;
  this->Port = 22222;
  this->CacheSize = 0;
  this->Internal = new vtkInternal;
  this->Internal->Observer->Source = this;
  vtkProcessModule::GetProcessModule()->AddObserver(
    vtkCommand::ConnectionCreatedEvent, this->Internal->Observer);

}

//-----------------------------------------------------------------------------
vtkLiveDataSource::~vtkLiveDataSource()
{
  this->Internal->Observer->Source = 0;
  vtkProcessModule::GetProcessModule()->RemoveObserver(this->Internal->Observer);
  delete this->Internal;
}

//----------------------------------------------------------------------------
unsigned long vtkLiveDataSource::GetMTime()
{
  return this->Superclass::GetMTime();
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::SendCoProcessorConnectionInfo()
{
  if (!this->CoProcessorConnectionId)
    {
    vtkErrorMacro("The coprocessor connection id has not been set.");
    return;
    }

  myprint("send_connection_info");

  vtkMultiProcessStream stream;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  stream << pm->GetNumberOfLocalPartitions();
  vtkCoProcessorConnection::SendStream(this->CoProcessorConnectionId, &stream);
}


//----------------------------------------------------------------------------
void vtkLiveDataSource::SetupCoProcessorConnections()
{

  int port = this->Port;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = pm->GetPartitionId();
  port = port + pid + 1;

  myprint("listen on port: " << port);

  this->Internal->ServerSocket = vtkSmartPointer<vtkServerSocket>::New();
  this->Internal->ServerSocket->CreateServer(port);

  pm->GetController()->Barrier();

  if (pid == 0)
    {
    // Todo- gather host:port info for satelites and send
    // For now just send hosts

    int numberOfMachines = pm->GetNumberOfMachines();
    int numberOfPartitions = pm->GetNumberOfLocalPartitions();

    vtkMultiProcessStream stream;
    stream << numberOfPartitions;

    for (int i = 0; i < numberOfPartitions; ++i)
      {
      if (i < numberOfMachines)
        {
        stream << pm->GetMachineName(i);
        }
      else
        {
        stream << "localhost";
        }
      }

    vtkCoProcessorConnection::SendStream(this->CoProcessorConnectionId, &stream);
    }

  vtkClientSocket* socket = this->Internal->ServerSocket->WaitForConnection();
  this->Internal->ServerSocket = 0;

  if (!socket)
    {
    vtkErrorMacro("Failed to get connection!");
    return;
    }

  this->Internal->SocketCommunicator = vtkSmartPointer<vtkSocketCommunicator>::New();
  this->Internal->SocketCommunicator->SetSocket(socket);
  this->Internal->SocketCommunicator->ServerSideHandshake();
  socket->Delete();

  //int remotePid;
  //comm->Receive(&remotePid, 1, 1, 9999);
  //myprint("connected to remote pid: " << remotePid);
}


//----------------------------------------------------------------------------
void vtkLiveDataSource::ListenForCoProcessorConnection()
{
  if (this->Internal->ConnectionIsOpen)
    {
    return;
    }

  this->Internal->ConnectionIsOpen = true;

  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  if (pm->GetPartitionId() == 0)
    {
    myprint("listen_for_cpconnection port: " << this->Port);
    pm->AcceptCoProcessorConnectionOnPort(this->Port);
    }
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::ChopVectors(int newSize)
{
  int currentSize = static_cast<int>(this->Internal->TimeSteps.size());
  if (currentSize <= newSize)
    {
    return;
    }

  int chopSize = currentSize - newSize;
  this->Internal->NumberOfTimeSteps -= chopSize;
  if (this->Internal->NumberOfTimeSteps < 0)
    {
    this->Internal->NumberOfTimeSteps = 0;
    }

  vtkstd::rotate(this->Internal->TimeSteps.begin(),
                 this->Internal->TimeSteps.end() - newSize,
                 this->Internal->TimeSteps.end());
  this->Internal->TimeSteps.resize(newSize);

  // now re-shuffle the data object pointers
  vtkInternal::DataObjectCacheType::iterator itr;
  for (itr = this->Internal->DataObjectCache.begin(); itr != this->Internal->DataObjectCache.end(); ++itr)
    {
    vtkInternal::DataObjectVectorType & dataObjectVector = itr->second;

    // Might need to grow the vector first
    while (dataObjectVector.size() < newSize)
      {
      dataObjectVector.push_back(NULL);
      }

    vtkstd::rotate(dataObjectVector.begin(),
                   dataObjectVector.end() - newSize,
                   dataObjectVector.end());
    dataObjectVector.resize(newSize);
    }
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::SetCacheSize(int cacheSize)
{
  if (cacheSize < 0)
    {
    cacheSize = 0;
    }

  if (cacheSize == this->CacheSize)
    {
    return;
    }


  // rotate and chop the vectors if we need to
  if (cacheSize)
    {
    this->ChopVectors(cacheSize);
    }

  this->CacheSize = cacheSize;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::ReceiveExtract()
{
  myprint("receive_extract");

  if (!this->Internal->SocketCommunicator)
    {
    vtkErrorMacro("Socket communicator is null");
    return;
    }


  int sinkTag;
  int numberOfExtracts;
  vtkIdType timestep;
  double time;

  this->Internal->SocketCommunicator->Receive(&timestep, 1, 1, 9998);
  this->Internal->SocketCommunicator->Receive(&time, 1, 1, 9998);
  this->Internal->SocketCommunicator->Receive(&numberOfExtracts, 1, 1, 9998);

  myprint("receiving " << numberOfExtracts << " extracts at time " << time);

  // Add new time

  // chop vectors if we are at the cache limit
  if (this->CacheSize && this->Internal->TimeSteps.size() >= this->CacheSize)
    {
    this->ChopVectors(this->CacheSize - 1);
    }

  this->Internal->TimeSteps.push_back(time);

  size_t insertIndex = this->Internal->TimeSteps.size() - 1;

  // Receive extracts
  for (int i = 0; i < numberOfExtracts; ++i)
    {
    this->Internal->SocketCommunicator->Receive(&sinkTag, 1, 1, 9998);
    vtkDataObject* dataObject = this->Internal->SocketCommunicator->ReceiveDataObject(1, 9999);
    if (!dataObject)
      {
      vtkErrorMacro("Error receiving data object from controller.");
      continue;
      }

    vtkInternal::DataObjectVectorType & dataObjectVector =
      this->Internal->DataObjectCache[sinkTag];

    while (dataObjectVector.size() < insertIndex + 1)
      {
      dataObjectVector.push_back(NULL);
      }

    dataObjectVector[insertIndex] = dataObject;
    dataObject->Delete();
    }

  this->Internal->NewDataAvailable = true;

  // Don't call modified, we will call modified from Poll().
  //this->Modified();
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::ReceiveState()
{
  myprint("receive_state");

  if (!this->CoProcessorConnectionId)
    {
    vtkErrorMacro("Connection ID is null");
    return;
    }

  vtkMultiProcessStream stream;
  vtkCoProcessorConnection::ReceiveStream(this->CoProcessorConnectionId, &stream);
  vtkstd::string xmlStr;
  stream >> xmlStr;
  this->SetCPState(xmlStr.c_str());
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::SendState()
{
  myprint("send_state");

  if (!this->CoProcessorConnectionId)
    {
    vtkErrorMacro("Connection ID is null");
    return;
    }

  vtkMultiProcessStream stream;
  vtkstd::string messages;

  if (this->CPStateSend)
    {
    messages += this->CPStateSend;
    }

  if (this->Internal->SendSinkStatusToCP)
    {
    vtksys_ios::ostringstream ostr;
    ostr << "<sink_status status=\"";
    vtkstd::map<int, int>::const_iterator itr;
    for (itr = this->Internal->SinkStatus.begin(); itr != this->Internal->SinkStatus.end(); ++itr)
      {
      ostr << itr->first << " " << itr->second << " ";
      }
    ostr << "\"/>";
    messages += ostr.str().c_str();
    this->Internal->SendSinkStatusToCP = false;    
    }
  
  if (messages.size())
    {
    messages = "<msg>\n" + messages + "</msg>\n";
    }

  stream << messages.c_str();

  vtkCoProcessorConnection::SendStream(this->CoProcessorConnectionId, &stream);
  this->SetCPStateSend(0);
}


//-----------------------------------------------------------------------------
int vtkLiveDataSource::PollForNewDataObjects()
{
  myprint("poll");

  if (this->Internal->NewDataAvailable)
    {
    myprint("new_data_available");
    this->Internal->NewDataAvailable = false;
    this->Internal->NumberOfTimeSteps = static_cast<int>(this->Internal->TimeSteps.size());
    this->Modified();

    // 3rd parties do not know about the new timesteps until
    // UpdateInformation is called.  Call it here, or call
    // it from the client.
    //this->UpdateInformation();
    return 1;
    }

  return 0;
}

//-----------------------------------------------------------------------------
int vtkLiveDataSource::GetIndexForTime(double time)
{
  int index = 0;
  double minDifference = VTK_DOUBLE_MAX;
  for (size_t i = 0; i < this->Internal->TimeSteps.size(); ++i)
    {
    double difference = fabs(this->Internal->TimeSteps[i] - time);
      if (difference < minDifference)
        {
        minDifference = difference;
        index = static_cast<int>(i);
        }
    }
  return index;
}

//----------------------------------------------------------------------------
int vtkLiveDataSource::ProcessRequest(vtkInformation* request,
                                        vtkInformationVector** inputVector,
                                        vtkInformationVector* outputVector)
{
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(request, inputVector, outputVector);
    }
  else if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->RequestInformation(request, inputVector, outputVector);
    }
  else if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }
 else if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
    }
  else
    {
    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
    }
}

//----------------------------------------------------------------------------
int vtkLiveDataSource::RequestInformation(
                                 vtkInformation* request,
                                 vtkInformationVector** vtkNotUsed(inputVector),
                                 vtkInformationVector* outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  double timeRange[2] = {0, 0};
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);

  vtkstd::vector<double> timeSteps;
  timeSteps.resize(this->Internal->NumberOfTimeSteps);
  for (int i = 0; i < this->Internal->NumberOfTimeSteps; ++i)
    {
    timeSteps[i] = this->Internal->TimeSteps[i];
    }
  if (timeSteps.size() > 0)
    {
    myprint("request_information, last time: " << timeSteps.back());
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
               &timeSteps[0], static_cast<int>(timeSteps.size()));
    }
  else
    {
    myprint("request_information, no available timesteps");
    outInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkLiveDataSource::RequestUpdateExtent(
                                 vtkInformation* vtkNotUsed(request),
                                 vtkInformationVector** vtkNotUsed(inputVector),
                                 vtkInformationVector* outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  return 1;
}

//-----------------------------------------------------------------------------
int vtkLiveDataSource::RequestData(vtkInformation *request,
                                     vtkInformationVector **inputVector,
                                     vtkInformationVector *outputVector)
{

  // This is a no-op after it is called the first time
  this->ListenForCoProcessorConnection();

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int piece, numPieces;
  piece = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  numPieces = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  vtkMultiBlockDataSet *output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("Incorrect output type.");
    return 0;
    }

  double time = 0;

  // todo - this algorithm should be able to supply multiple times pretty easily
  // but for now just print a warning and only return only the first requested time
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    time = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0];
    if (outInfo->Length(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()) > 1)
      {
      vtkWarningMacro("More than one timestep requested.");
      }
    }

  // Lookup index for requested time, possibly clamping the index
  int stepIndex = this->GetIndexForTime(time);
  if (stepIndex >= this->Internal->NumberOfTimeSteps)
    {
    stepIndex = this->Internal->NumberOfTimeSteps - 1;
    }

  time = 0;
  if (stepIndex >= 0 && stepIndex < this->Internal->TimeSteps.size())
    {
    time = this->Internal->TimeSteps[stepIndex];
    }

  output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &time, 1);


  vtkInternal::DataObjectCacheType::const_iterator itr;
  for (itr = this->Internal->DataObjectCache.begin(); itr != this->Internal->DataObjectCache.end(); ++itr)
    {
    int sinkTag = itr->first;
    if (this->GetSinkStatus(sinkTag) == 0)
      {
      continue;
      }

    if (stepIndex >= 0 && stepIndex < static_cast<int>(itr->second.size()))
      {
      output->SetBlock(output->GetNumberOfBlocks(), itr->second[stepIndex]);
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkLiveDataSource::FillOutputPortInformation(int port, 
                                                   vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  return 1;
}

//-----------------------------------------------------------------------------
void vtkLiveDataSource::SetSinkStatus(int sinkTag, int status)
{
  if (status != this->GetSinkStatus(sinkTag))
    {
    this->Internal->SinkStatus[sinkTag] = status;
    this->Internal->SendSinkStatusToCP = true;
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
int vtkLiveDataSource::GetSinkStatus(int sinkTag)
{
  vtkstd::map<int,int>::const_iterator itr = this->Internal->SinkStatus.find(sinkTag);
  if (itr == this->Internal->SinkStatus.end())
    {
    return 1;
    }
  return itr->second;
}

//-----------------------------------------------------------------------------
void vtkLiveDataSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  //os << indent << "MetaFileName: " << this->MetaFileName << endl;
}
