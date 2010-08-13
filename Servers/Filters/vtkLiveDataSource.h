/*=========================================================================

  Program:   ParaView
  Module:    vtkLiveDataSource.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLiveDataSource - 
//
// .SECTION Description:
//

//

#ifndef __vtkLiveDataSource_h
#define __vtkLiveDataSource_h

#include "vtkDataObjectAlgorithm.h"

class vtkStringArray;

class VTK_EXPORT vtkLiveDataSource : public vtkDataObjectAlgorithm
{
public:
  static vtkLiveDataSource* New();
  vtkTypeMacro(vtkLiveDataSource, vtkDataObjectAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(CoProcessorConnectionId, vtkIdType);
  vtkGetMacro(CoProcessorConnectionId, vtkIdType);

  vtkSetStringMacro(CPState);
  vtkGetStringMacro(CPState);

  vtkSetStringMacro(CPStateSend);
  vtkGetStringMacro(CPStateSend);

  void SetSinkStatus(int sinkTag, int status);
  int GetSinkStatus(int sinkTag);

  // Description:
  // All pipeline passes are forwarded to the internal reader. The
  // vtkLiveDataSource reports time steps in RequestInformation. It
  // updated the file name of the internal in RequestUpdateExtent based
  // on the time step request.
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

  // Description:
  // Return the MTime also considering the internal vtkFileSet.
  virtual unsigned long GetMTime();

  vtkSetMacro(Port, int);
  vtkGetMacro(Port, int);

  vtkGetMacro(CacheSize, int);
  void SetCacheSize(int cacheSize);

  void ChopVectors(int newSize);

  virtual void ListenForCoProcessorConnection();

  virtual int PollForNewDataObjects();

  virtual void SendCoProcessorConnectionInfo();
  virtual void SetupCoProcessorConnections();

  virtual void ReceiveExtract();

  virtual void ReceiveState();
  virtual void SendState();

protected:
  vtkLiveDataSource();
  ~vtkLiveDataSource();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);
  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Description:
  // Returns the index of the timestep closest to the given time, could be
  // more or less.
  virtual int GetIndexForTime(double time);

  vtkIdType   CoProcessorConnectionId;
  char*       CPState;
  char*       CPStateSend;
  int         Port;
  int         CacheSize;

private:
  vtkLiveDataSource(const vtkLiveDataSource&); // Not implemented.
  void operator=(const vtkLiveDataSource&); // Not implemented.

  //BTX
  class vtkConnectionObserver;
  class vtkInternal;
  vtkInternal* Internal;
  //ETX
};

#endif

