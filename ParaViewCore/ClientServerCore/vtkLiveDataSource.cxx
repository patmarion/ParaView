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
//#include "vtkCoProcessorConnection.h"
#include "vtkSocketCommunicator.h"
#include "vtkMultiProcessController.h"
#include "vtkNetworkAccessManager.h"
#include "vtkServerSocket.h"
#include "vtkClientSocket.h"
#include "vtkMultiProcessStream.h"
#include "vtkClientServerStream.h"
//#include "vtkProcessModuleConnectionManager.h"
#include "vtkStringArray.h"
#include "vtkPVXMLElement.h"
#include "vtkPVServerOptions.h"
#include "vtkTimerLog.h"

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


namespace {

const int RMITag = 54647;
const int CommTag = 45546;

//-----------------------------------------------------------------------------
// Called when requesting to process the stream on Root Node only.
void SateliteRMI(void *localArg, void *remoteArg, int remoteArgLength, int vtkNotUsed(remoteProcessId))
{
  vtkLiveDataSource* self = static_cast<vtkLiveDataSource*>(localArg);
  self->HandleCommand(reinterpret_cast<char*>(remoteArg));
}

void RootRMI(void *localArg, void *remoteArg, int remoteArgLength, int vtkNotUsed(remoteProcessId))
{
  vtkLiveDataSource* self = static_cast<vtkLiveDataSource*>(localArg);
  self->HandleCommand(reinterpret_cast<char*>(remoteArg));
}

}

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
    if (event == vtkCommand::ConnectionCreatedEvent)
      {
      int port = *(reinterpret_cast<int*>(calldata));
      if (this->Source && port == this->Source->GetPort())
        {
        this->Source->OnCoProcessorConnected();
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

  vtkSmartPointer<vtkMultiProcessController> CommandController;

  std::vector<vtkSmartPointer<vtkSocketCommunicator> > SocketCommunicators;

  vtkSmartPointer<vtkClientSocket>       NotifySocket;

  typedef std::vector<vtkSmartPointer<vtkDataObject> > DataPiecesType;
  typedef std::vector<DataPiecesType> DataPiecesVectorType;
  typedef std::map<int, DataPiecesVectorType> DataObjectCacheType;
  DataObjectCacheType DataObjectCache;

  std::vector<double> TimeSteps;
  std::map<int, int>  SinkStatus;
};

//-----------------------------------------------------------------------------
vtkLiveDataSource::vtkLiveDataSource()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->CPState = 0;
  this->CPStateSend = 0;
  this->Port = 22222;
  this->CacheSize = 0;
  this->SocketsPerProcess = 0;
  this->Internal = new vtkInternal;
  this->Internal->Observer->Source = this;
  vtkProcessModule::GetProcessModule()->GetNetworkAccessManager()->AddObserver(
    vtkCommand::ConnectionCreatedEvent, this->Internal->Observer);
}

//-----------------------------------------------------------------------------
vtkLiveDataSource::~vtkLiveDataSource()
{
  this->Internal->Observer->Source = 0;
  vtkProcessModule::GetProcessModule()->GetNetworkAccessManager()->RemoveObserver(this->Internal->Observer);
  delete this->Internal;
}

//----------------------------------------------------------------------------
unsigned long vtkLiveDataSource::GetMTime()
{
  return this->Superclass::GetMTime();
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::HandleCommand(const char* command)
{
  myprint("handling command: " << command);


  std::string commandStr = command;

  if (commandStr == "send_info") // proc 0 only
    {
    this->SendCoProcessorConnectionInfo();
    }
  else if (commandStr == "connect_multi") // proc 0 only
    {
    std::string newCommand = "connect_multi_all";
    vtkMultiProcessController::GetGlobalController()->TriggerRMIOnAllChildren(
      const_cast<char*>(newCommand.c_str()), newCommand.size()+1, RMITag);
    this->HandleCommand(newCommand.c_str());
    }
  else if (commandStr == "connect_multi_all") // all procs
    {
    this->SetupCoProcessorConnections();
    }
  else if (commandStr == "rcv") // proc 0 only
    {
    std::string newCommand = "rcv_all";
    vtkMultiProcessController::GetGlobalController()->TriggerRMIOnAllChildren(
      const_cast<char*>(newCommand.c_str()), newCommand.size()+1, RMITag);
    this->HandleCommand(newCommand.c_str());
    }
  else if (commandStr == "rcv_all") // proc 0 only
    {
    this->ReceiveExtract();
    }
  else if (commandStr == "statercv") // proc 0 only
    {
    this->ReceiveState();
    }
  else if (commandStr == "statesend")
    {
    this->SendState();
    }
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::OnCoProcessorConnected()
{
  myprint("receiving incoming coprocessor connection");

  std::stringstream urlStream;
  urlStream << "tcp://localhost:" << this->Port << "?listen=true";

  this->Internal->CommandController =
    vtkProcessModule::GetProcessModule()->GetNetworkAccessManager()->NewConnection(urlStream.str().c_str());

  myprint("got CommandController " << &this->Internal->CommandController);

  if (this->Internal->CommandController)
    {
    myprint("attaching root rmi to socket controller");
    this->Internal->CommandController->AddRMI(&RootRMI, this, RMITag);
    }
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::SendCoProcessorConnectionInfo()
{
  if (!this->Internal->CommandController)
    {
    vtkErrorMacro("The connection has not been established.");
    return;
    }

  myprint("send_connection_info");

  vtkMultiProcessStream stream;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  stream << pm->GetNumberOfLocalPartitions() * this->SocketsPerProcess;
  this->Internal->CommandController->Send(stream, 1, CommTag);
}


//----------------------------------------------------------------------------
void vtkLiveDataSource::SetupCoProcessorConnections()
{
  const int dataPortBase = this->Port + 1;

  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  int pid = pm->GetPartitionId();

  std::vector<int> ports;
  std::vector<vtkSmartPointer<vtkServerSocket> > serverSockets;
  for (int i = 0; i < this->SocketsPerProcess; ++i)
    {
    ports.push_back(dataPortBase + pid*this->SocketsPerProcess + i);
    serverSockets.push_back(vtkSmartPointer<vtkServerSocket>::New());

    myprint("listening on port: " << ports.back());

    serverSockets.back()->CreateServer(ports.back());
    }

  // debugging barrier
  pm->GetGlobalController()->Barrier();

  if (pid == 0)
    {
    // Todo- gather host:port info for satelites and send
    // For now just send hosts

    int numberOfMachineNames = 0;
    int numberOfProcesses = pm->GetNumberOfLocalPartitions();

    vtkPVServerOptions* options = vtkPVServerOptions::SafeDownCast(pm->GetOptions());
    if (options)
      {
      numberOfMachineNames = options->GetNumberOfMachines();
      }

    vtkMultiProcessStream stream;
    stream << numberOfProcesses * this->SocketsPerProcess;

    for (int i = 0; i < numberOfProcesses; ++i)
      {
      for (int k = 0; k < this->SocketsPerProcess; ++k)
        {
        if (i < numberOfMachineNames)
          {
          stream << options->GetMachineName(i);
          }
        else
          {
          stream << "localhost";
          }
        }
      }

    this->Internal->CommandController->Send(stream, 1, CommTag);
    }

  // Wait for connections
  this->Internal->SocketCommunicators.clear();
  for (size_t i = 0; i < serverSockets.size(); ++i)
    {
    vtkClientSocket* socket = serverSockets[i]->WaitForConnection();
    serverSockets[i] = 0;
    if (!socket)
      {
      vtkErrorMacro("Data connection failed for server socket on port " << ports[i]);
      continue;
      }

    this->Internal->SocketCommunicators.push_back(vtkSmartPointer<vtkSocketCommunicator>::New());
    this->Internal->SocketCommunicators.back()->SetSocket(socket);
    this->Internal->SocketCommunicators.back()->ServerSideHandshake();
    socket->Delete();
    }
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
  vtkMultiProcessController* globalController = pm->GetGlobalController();

  if (pm->GetPartitionId() == 0)
    {
    myprint("listen_for_cpconnection port: " << this->Port);

    std::stringstream urlStream;
    urlStream << "tcp://localhost:" << this->Port << "?listen=true&nonblocking=true";
    pm->GetNetworkAccessManager()->NewConnection(urlStream.str().c_str());

    vtkPVOptions* options = pm->GetOptions();
    const char* hostName = options->GetClientHostName();
    const int port = 12345;
    if (hostName && hostName[0])
      {
      myprint("creating client notify connection on " << hostName << ":" << port);

      this->Internal->NotifySocket = vtkSmartPointer<vtkClientSocket>::New();
      int result = this->Internal->NotifySocket->ConnectToServer(hostName, 12345);
      if (result != 0)
        {
        vtkErrorMacro("Failed to connect notify socket with client "
                      << hostName << ":" << port);
        this->Internal->NotifySocket = NULL;
        }
      }
    }
  else
    {
    myprint("adding RMIs to global controller on process " << pm->GetPartitionId());
    // connect callbacks to global controller
    vtkMultiProcessController::GetGlobalController()->AddRMI(&SateliteRMI, this, RMITag);
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
    vtkInternal::DataPiecesVectorType & dataPiecesVector = itr->second;

    // Might need to grow the vector first
    while (dataPiecesVector.size() < newSize)
      {
      dataPiecesVector.push_back(vtkInternal::DataPiecesType());
      }

    vtkstd::rotate(dataPiecesVector.begin(),
                   dataPiecesVector.end() - newSize,
                   dataPiecesVector.end());
    dataPiecesVector.resize(newSize);
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
  vtkTimerLog::MarkStartEvent("vtkLiveDataSource::ReceiveExtract");

  // chop storage vectors if we are at the cache limit
  if (this->CacheSize && this->Internal->TimeSteps.size() >= this->CacheSize)
    {
    this->ChopVectors(this->CacheSize - 1);
    }

  // Receive extract data on each socket communicator
  const size_t numberOfSocketCommunicators = this->Internal->SocketCommunicators.size();
  for (size_t commIndex = 0; commIndex < numberOfSocketCommunicators; ++commIndex)
    {

    vtkSocketCommunicator* communicator = this->Internal->SocketCommunicators[commIndex];
    if (!communicator)
      {
      vtkErrorMacro("Socket communicator " << commIndex << " is null");
      continue;
      }


    int sinkTag;
    int numberOfExtracts;
    vtkIdType timestep;
    double time;

    communicator->Receive(&timestep, 1, 1, 9998);
    communicator->Receive(&time, 1, 1, 9998);
    communicator->Receive(&numberOfExtracts, 1, 1, 9998);

    myprint("receiving " << numberOfExtracts << " extracts at time " << time);

    // Add new time
    if (commIndex == 0)
      {
      this->Internal->TimeSteps.push_back(time);
      }

    size_t numberOfTimeSteps = this->Internal->TimeSteps.size();

    // Receive extracts
    for (int i = 0; i < numberOfExtracts; ++i)
      {
      communicator->Receive(&sinkTag, 1, 1, 9998);
      vtkDataObject* dataObject = communicator->ReceiveDataObject(1, 9999);
      if (!dataObject)
        {
        vtkErrorMacro("Error receiving data object from controller.");
        continue;
        }

      vtkInternal::DataPiecesVectorType& dataPiecesVector =
        this->Internal->DataObjectCache[sinkTag];

      if (dataPiecesVector.size() < numberOfTimeSteps)
        {
        dataPiecesVector.resize(numberOfTimeSteps);
        }

      vtkInternal::DataPiecesType& dataPieces = dataPiecesVector[numberOfTimeSteps-1];

      dataPieces.resize(numberOfSocketCommunicators);
      dataPieces[commIndex] = dataObject;
      dataObject->Delete();
      }
    }

  this->Internal->NewDataAvailable = true;

  vtkTimerLog::MarkEndEvent("vtkLiveDataSource::ReceiveExtract");

  // Notify client
  /*
  if (this->Internal->NotifySocket)
    {
    this->Internal->NotifySocket->Send("N", 1);
    }
  */
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::ReceiveState()
{
  myprint("receive_state");

  if (!this->Internal->CommandController)
    {
    vtkErrorMacro("Command controller is null");
    return;
    }

  vtkMultiProcessStream stream;
  this->Internal->CommandController->Receive(stream, 1, CommTag);
  vtkstd::string xmlStr;
  stream >> xmlStr;
  this->SetCPState(xmlStr.c_str());

  std::cout << this->GetCPState() << std::endl;
}

//----------------------------------------------------------------------------
void vtkLiveDataSource::SendState()
{
  myprint("send_state");

  if (!this->Internal->CommandController)
    {
    vtkErrorMacro("Command controller is null");
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

  this->Internal->CommandController->Send(stream, 1, CommTag);
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
    return 0;
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

    const vtkInternal::DataPiecesVectorType& dataPiecesVector = itr->second;
    
    vtkSmartPointer<vtkMultiBlockDataSet> dataPiecesOutput = vtkSmartPointer<vtkMultiBlockDataSet>::New();
    output->SetBlock(output->GetNumberOfBlocks(), dataPiecesOutput);

    if (stepIndex >= 0 && stepIndex < static_cast<int>(dataPiecesVector.size()))
      {
      const vtkInternal::DataPiecesType& dataPieces = dataPiecesVector[stepIndex];
      for (size_t pieceIndex = 0; pieceIndex < dataPieces.size(); ++pieceIndex)
        {
        dataPiecesOutput->SetBlock(dataPiecesOutput->GetNumberOfBlocks(), dataPieces[pieceIndex]);
        }
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
