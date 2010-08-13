/*=========================================================================

  Program:   ParaView
  Module:    vtkCoProcessorConnection.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCoProcessorConnection.h"

#include "vtkClientServerInterpreter.h"
#include "vtkClientServerStream.h"
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkProcessModuleConnectionManager.h"
#include "vtkCommand.h"
#include "vtkPVConfig.h" // for PARAVIEW_VERSION_MAJOR etc.
#include "vtkPVInformation.h"
#include "vtkPVOptions.h"
#include "vtkSocketCommunicator.h"
#include "vtkSocketController.h"
#include "vtkMultiProcessStream.h"
#include "vtkUndoSet.h"
#include "vtkUndoStack.h"

#include <vtkstd/new>
#include <vtkstd/map>
#include <vtkstd/string>

#if 0
#define myprint(msg)
#else
#define myprint(msg) \
    std::cout << "proc(" \
              << vtkProcessModule::GetProcessModule()->GetPartitionId() \
              << ") " << msg << endl;
#endif


//-----------------------------------------------------------------------------
// Maps a connection id to a handler object's client-server id
static vtkstd::map<vtkIdType, vtkClientServerID> HandlerMap;

//-----------------------------------------------------------------------------
// RMI Callbacks.

// Called when requesting the last result.
void vtkCoProcessorConnectionLastResultRMI(void* localArg, void* , int, int)
{
  vtkCoProcessorConnection* self = (vtkCoProcessorConnection*)localArg;
  self->SendLastResult();
}

//-----------------------------------------------------------------------------
// Called when requesting to process the stream on Root Node only.
void vtkCoProcessorConnectionRootRMI(void *localArg, void *remoteArg,
  int remoteArgLength, int vtkNotUsed(remoteProcessId))
{
  try
    {
    
    vtkClientServerStream stream;
    stream.SetData(reinterpret_cast<unsigned char*>(remoteArg), remoteArgLength);

    vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

    //printf("received stream:\n");
    //stream.Print(cout);

    // Tell process module to send it to SelfConnection Root.
    //vtkProcessModule::GetProcessModule()->SendStream(
    //  vtkProcessModuleConnectionManager::GetSelfConnectionID(),
    //  vtkProcessModule::DATA_SERVER_ROOT, stream);

    myprint("process_stream");

    //printf("  nmsgs: %d\n", stream.GetNumberOfMessages());
    //printf("  nargs: %d\n", stream.GetNumberOfArguments(0));

    vtkCoProcessorConnection* self = static_cast<vtkCoProcessorConnection*>(localArg);
    vtkIdType connId = pm->GetConnectionID(self);
    vtkClientServerID handlerId = vtkCoProcessorConnection::GetHandlerIdForConnectionId(connId);
    //printf("  got connid(%u) handlerid(%u)\n", connId, handlerId.ID);

    const char* arg = 0;
    stream.GetArgument(0, 0, &arg);

    vtkClientServerStream stored = pm->GetInterpreter()->GetLastResult();

    if (!strcmp(arg, "rcv"))
      {
      // Invoke handler method on all satelites
      vtkClientServerStream stream;
      stream << vtkClientServerStream::Invoke
             << handlerId << "ReceiveExtract"
             << vtkClientServerStream::End;
      pm->SendStream(vtkProcessModuleConnectionManager::GetSelfConnectionID(),
                                         vtkProcessModule::DATA_SERVER, stream);
      }
    else if (!strcmp(arg, "statercv"))
      {
      vtkClientServerStream stream;
      stream << vtkClientServerStream::Invoke
             << handlerId << "ReceiveState"
             << vtkClientServerStream::End;
      pm->SendStream(vtkProcessModuleConnectionManager::GetSelfConnectionID(),
                                         vtkProcessModule::DATA_SERVER_ROOT, stream);

      }
    else if (!strcmp(arg, "statesend"))
      {
      vtkClientServerStream stream;
      stream << vtkClientServerStream::Invoke
             << handlerId << "SendState"
             << vtkClientServerStream::End;
      pm->SendStream(vtkProcessModuleConnectionManager::GetSelfConnectionID(),
                                         vtkProcessModule::DATA_SERVER_ROOT, stream);
      }
    else if (!strcmp(arg, "send_info"))
      {
      vtkClientServerStream stream;
      stream << vtkClientServerStream::Invoke
             << handlerId << "SendCoProcessorConnectionInfo"
             << vtkClientServerStream::End;
      pm->SendStream(vtkProcessModuleConnectionManager::GetSelfConnectionID(),
                                         vtkProcessModule::DATA_SERVER_ROOT, stream);
      }
    else if (!strcmp(arg, "connect_multi"))
      {
      // Invoke handler method on all satelites
      vtkClientServerStream stream;
      stream << vtkClientServerStream::Invoke
             << handlerId << "SetupCoProcessorConnections"
             << vtkClientServerStream::End;
      pm->SendStream(vtkProcessModuleConnectionManager::GetSelfConnectionID(),
                                         vtkProcessModule::DATA_SERVER, stream);
      }
    else
      {
      myprint("command was: " << arg);
      }
    

    }
  catch (vtkstd::bad_alloc)
    {
    vtkProcessModule::GetProcessModule()->ExceptionEvent(
      vtkProcessModule::EXCEPTION_BAD_ALLOC);
    }
  catch (...)
    {
    vtkProcessModule::GetProcessModule()->ExceptionEvent(
      vtkProcessModule::EXCEPTION_UNKNOWN);
    }
}

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCoProcessorConnection);

//-----------------------------------------------------------------------------
vtkCoProcessorConnection::vtkCoProcessorConnection()
{
  this->IsClientConnection = 0;
  this->LastResultStream = new vtkClientServerStream;
}

//-----------------------------------------------------------------------------
vtkCoProcessorConnection::~vtkCoProcessorConnection()
{
  delete this->LastResultStream;
}

//-----------------------------------------------------------------------------
int vtkCoProcessorConnection::Initialize(int argc, char** argv, int *partitionId)
{
  this->Superclass::Initialize(argc, argv, partitionId);

  // Ensure that we are indeed the root node.
  if (vtkMultiProcessController::GetGlobalController()->
    GetLocalProcessId() != 0)
    {
    vtkErrorMacro("vtkCoProcessorConnection can only be initialized on the Root node.");
    return 1;
    }

  if (!this->AuthenticateConnection())
    {
    vtkErrorMacro("Failed to authenticate with client.");
    return 1;
    }
  
  // TODO- uncomment this if condition
  // Setup RMIs on server side
  //if (!this->GetIsClientConnection())
  //  {
    this->SetupRMIs();
  //  }

  /*
  if (!this->GetIsClientConnection())
    {
    vtkClientServerStream str;
    str << vtkClientServerStream::Invoke << "foo" << vtkClientServerStream::End;
    this->SendStreamToDataServerRoot(str);
    }
  else
    {
    this->ProcessCommunication();
    }
  */

  
  return 0;
}

//-----------------------------------------------------------------------------
void vtkCoProcessorConnection::RegisterHandlerForConnection(vtkIdType connectionId,
                                                              vtkClientServerID handlerCSId)
{
  HandlerMap[connectionId] = handlerCSId;
}

//-----------------------------------------------------------------------------
vtkClientServerID vtkCoProcessorConnection::GetHandlerIdForConnectionId(vtkIdType connectionId)
{
  return HandlerMap[connectionId];
}

//-----------------------------------------------------------------------------
void vtkCoProcessorConnection::Finalize()
{
  this->GetSocketController()->CloseConnection();
  this->Superclass::Finalize();
}

//-----------------------------------------------------------------------------
int vtkCoProcessorConnection::AuthenticateConnection()
{

  if (this->GetIsClientConnection())
    {
    //printf("CP connection authenticating in client mode\n");
    // Send version
    int match = 0;
    int version = PARAVIEW_VERSION_MAJOR;
    if (!this->Controller->Send(&version, 1, 1, 
        vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG))
      {
      return 0;
      }
    version = PARAVIEW_VERSION_MINOR;
    if (!this->Controller->Send(&version, 1, 1, 
        vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG))
      {
      return 0;
      }
    version = PARAVIEW_VERSION_PATCH;
    if (!this->Controller->Send(&version, 1, 1,
        vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG))
      {
      return 0;
      }
    // Receive the result of the version check
    this->Controller->Receive(&match, 1, 1, 
      vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG);
    if (!match)
      {
      vtkErrorMacro("Version mismatch.");
      return 0;
      }
    }
  else
    {
    //printf("CP connection authenticating in server mode\n");
    // Receive version
    int match = 0;
    int majorVersion =0, minorVersion =0, patchVersion =0;
    this->Controller->Receive(&majorVersion, 1, 1, 
      vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG);
    this->Controller->Receive(&minorVersion, 1, 1, 
      vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG);
    this->Controller->Receive(&patchVersion, 1, 1, 
      vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG);
    match = ( (majorVersion == PARAVIEW_VERSION_MAJOR) ||
      (minorVersion == PARAVIEW_VERSION_MINOR) );
    // Tell the client the result of version check
    this->Controller->Send(&match, 1, 1, 
      vtkRemoteConnection::CLIENT_SERVER_COMMUNICATION_TAG); 
    if (!match)
      {
      vtkErrorMacro("Client-Server Version mismatch. "
        << "Connection will be aborted.");
      return 0;
      }
    }

  return 1; //SUCCESS.
}

//-----------------------------------------------------------------------------
// send a stream to the data server root mpi process
int vtkCoProcessorConnection::SendStreamToDataServerRoot(vtkClientServerStream& stream)
{
  const unsigned char* data;
  size_t len;
  stream.GetData(&data, &len);
  this->GetSocketController()->TriggerRMI(1, (void*)data, static_cast<int>(len), 
    vtkRemoteConnection::CLIENT_SERVER_ROOT_RMI_TAG);
  return 0;
}

//-----------------------------------------------------------------------------
void vtkCoProcessorConnection::SetupRMIs()
{
  // We have succesfully authenticated with the client. The connection
  // is deemed valid. Now set up RMIs so that we can communicate.

  this->Controller->AddRMI(vtkCoProcessorConnectionLastResultRMI,
    (void *)(this),
    vtkRemoteConnection::CLIENT_SERVER_LAST_RESULT_TAG);
  
  
  this->Controller->AddRMI(vtkCoProcessorConnectionRootRMI, 
    (void *)(this),
    vtkRemoteConnection::CLIENT_SERVER_ROOT_RMI_TAG);
  
  vtkSocketCommunicator* comm = vtkSocketCommunicator::SafeDownCast(
    this->Controller->GetCommunicator());
  if (comm)
    {
    comm->SetReportErrors(0);
    }
}

//-----------------------------------------------------------------------------
vtkTypeUInt32 vtkCoProcessorConnection::CreateSendFlag(vtkTypeUInt32 servers)
{
  if (servers != 0)
    {
    return vtkProcessModule::DATA_SERVER_ROOT;
    }
  return 0;
}


//-----------------------------------------------------------------------------
int vtkCoProcessorConnection::SendStream(vtkIdType connectionId,
                                          vtkMultiProcessStream* stream)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  vtkCoProcessorConnection* connection = vtkCoProcessorConnection::SafeDownCast(
                               pm->GetConnectionFromId(connectionId));

  if (!connection)
    {
    vtkGenericWarningMacro("Couldn't get a connection for given connection id");
    return 0;
    }

  return connection->GetSocketController()->Send(*stream, 1, 999798);
}

//-----------------------------------------------------------------------------
int vtkCoProcessorConnection::ReceiveStream(vtkIdType connectionId,
                                             vtkMultiProcessStream* stream)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  vtkCoProcessorConnection* connection = vtkCoProcessorConnection::SafeDownCast(
                               pm->GetConnectionFromId(connectionId));

  if (!connection)
    {
    vtkGenericWarningMacro("Couldn't get a connection for given connection id");
    return 0;
    }

  stream->Reset();
  return connection->GetSocketController()->Receive(*stream, 1, 999798);
}


//-----------------------------------------------------------------------------
void vtkCoProcessorConnection::SendLastResult()
{
  const unsigned char* data;
  size_t length = 0;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  vtkClientServerInterpreter* interpreter = pm->GetInterpreter();
  interpreter->GetLastResult().GetData(&data, &length);

  interpreter->GetLastResult().Print(cout);
  int len = static_cast<int>(length);
  
  this->GetSocketController()->Send(&len, 1, 1,
    vtkRemoteConnection::ROOT_RESULT_LENGTH_TAG);
  if(length > 0)
    {
    this->GetSocketController()->Send((char*)(data), length, 1,
      vtkRemoteConnection::ROOT_RESULT_TAG);
    }
}


//-----------------------------------------------------------------------------
const vtkClientServerStream& vtkCoProcessorConnection::GetLastResult(vtkTypeUInt32 
  vtkNotUsed(serverFlags))
{
  if (this->AbortConnection)
    {
    // Don't get last restult on an aborted connection.
    this->LastResultStream->Reset();
    return *this->LastResultStream;
    }

  int length =0;
  this->GetSocketController()->TriggerRMI(1, "", 
    vtkRemoteConnection::CLIENT_SERVER_LAST_RESULT_TAG);
  this->GetSocketController()->Receive(&length, 1, 1, 
    vtkRemoteConnection::ROOT_RESULT_LENGTH_TAG);
  if (length <= 0)
    {
    this->LastResultStream->Reset();
    *this->LastResultStream
      << vtkClientServerStream::Error
      << "vtkCoProcessorConnection::GetLastResultInternal() received no data from the"
      << " server." << vtkClientServerStream::End;
    }
  else
    {
    unsigned char* result = new unsigned char[length];
    this->GetSocketController()->Receive((char*)result, length, 1, 
      vtkRemoteConnection::ROOT_RESULT_TAG);
    this->LastResultStream->SetData(result, length);
    delete[] result;
    }
  return *this->LastResultStream;
}


//-----------------------------------------------------------------------------
void vtkCoProcessorConnection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
