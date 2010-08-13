/*=========================================================================

  Program:   ParaView
  Module:    vtkCoProcessorConnection.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCoProcessorConnection - represent a connection with a client.
// .SECTION Description
// This is a remote connection "to" a Client. This class is only instantiated on 
// the server (data/render).

#ifndef __vtkCoProcessorConnection_h
#define __vtkCoProcessorConnection_h

#include "vtkRemoteConnection.h"
class vtkUndoStack;
class vtkMultiProcessStream;

class VTK_EXPORT vtkCoProcessorConnection : public vtkRemoteConnection
{
public:
  static vtkCoProcessorConnection* New();
  vtkTypeMacro(vtkCoProcessorConnection, vtkRemoteConnection);
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Initializes the connection. This is essential to
  // intialize the controller associated with the connection etc etc.
  // This sets up the RMIs and returns. 
  virtual int Initialize(int argc, char** argv, int *partitionId); 
 
  // Description:
  // Finalizes the connection.
  virtual void Finalize();

  // Description:
  // Send the last result over to the client.
  void SendLastResult();

  const vtkClientServerStream& GetLastResult(vtkTypeUInt32 serverFlags);

  static void RegisterHandlerForConnection(vtkIdType connectionId, vtkClientServerID handlerCSId);
  static vtkClientServerID GetHandlerIdForConnectionId(vtkIdType);

  //BTX
  static int SendStream(vtkIdType connectionId, vtkMultiProcessStream* stream);
  static int ReceiveStream(vtkIdType connectionId, vtkMultiProcessStream* stream);
  //ETX

  // Description:
  // Client connection does not support these method. Do nothing.
  virtual void PushUndo(const char*, vtkPVXMLElement*)  { }
  virtual vtkPVXMLElement* NewNextUndo() { return 0; }
  virtual vtkPVXMLElement* NewNextRedo() { return 0; }


  vtkGetMacro(IsClientConnection, int);
  vtkSetMacro(IsClientConnection, int);

//BTX
protected:
  vtkCoProcessorConnection();
  ~vtkCoProcessorConnection();

  int IsClientConnection;
  vtkClientServerStream* LastResultStream;

  // Description:
  // ClientConnection cannot send streams anywhere.
  virtual vtkTypeUInt32 CreateSendFlag(vtkTypeUInt32 servers);


  int SendStreamToDataServerRoot(vtkClientServerStream& stream);

  // Description:
  // Authenticates the connection. Returns 1 on success, 0 on error.
  int AuthenticateConnection();

  // Description:
  // Set up RMI callbacks.
  void SetupRMIs();

private:
  vtkCoProcessorConnection(const vtkCoProcessorConnection&); // Not implemented.
  void operator=(const vtkCoProcessorConnection&); // Not implemented.
//ETX
};
                                       

#endif

