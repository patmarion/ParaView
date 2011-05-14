/*=========================================================================

  Program:   ParaView
  Module:    vtkCPPartitionHelper.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCPPartitionHelper.h"

#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"
#include "vtkSortDataArray.h"
#include "vtkProcessModule.h"
#include "vtkMultiProcessController.h"
#include "vtkMultiProcessStream.h"


#ifdef __bgp__
#include <spi/kernel_interface.h>
#include <common/bgp_personality.h>
#include <common/bgp_personality_inlines.h>
#endif

#if 1
#define myprint(msg)
#else
#define myprint(msg) \
    std::cout << "proc(" \
              << vtkProcessModule::GetProcessModule()->GetPartitionId() \
              << ") " << msg << endl;
#endif

//----------------------------------------------------------------------------
class vtkCPPartitionHelper::vtkInternals
{
public:

};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCPPartitionHelper);

//----------------------------------------------------------------------------
vtkCPPartitionHelper::vtkCPPartitionHelper()
{
  this->Internals = new vtkInternals();
}

//----------------------------------------------------------------------------
vtkCPPartitionHelper::~vtkCPPartitionHelper()
{
  delete this->Internals;
  this->Internals = 0;
}

//----------------------------------------------------------------------------
void vtkCPPartitionHelper::ComputePSetRanks(vtkIntArray* ranks)
{

#ifdef __bgp__

  int rankInPset;
  int psetId;

  _BGP_Personality_t personality;
  Kernel_GetPersonality(&personality, sizeof(personality));

  rankInPset = personality.Network_Config.RankInPSet;
  psetId = personality.Network_Config.PSetNum;
  //psetSize = personality.Network_Config.PSetSize;

  //MPI_Comm psetComm;
  //MPIX_Pset_same_comm_create(&psetComm);
  //MPI_Comm_rank(psetComm, &rankInPset);
  //MPI_Comm_size(psetComm, &psetSize);

  vtkIntArray* tmp = vtkIntArray::New();
  tmp->SetNumberOfTuples(2);
  tmp->SetValue(0, rankInPset);
  tmp->SetValue(1, psetId);

  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  ranks->SetNumberOfTuples(pm->GetNumberOfLocalPartitions()*2);
  pm->GetController()->AllGather(tmp, ranks);
  tmp->Delete();

#else
  vtkGenericWarningMacro("vtkCPPartitionHelper::ComputePSetRanks(vtkIntArray*) not implemented.");
#endif

}


//----------------------------------------------------------------------------
void vtkCPPartitionHelper::SendMessages(vtkStringArray* sendArray, vtkStringArray* receiveArray,
            vtkMultiProcessController* controller, int receivePid)
{

    const int commTag = 34567;
    const int localPid = controller->GetLocalProcessId();

    if (localPid == receivePid)
      {

      for (int i = 0; i < sendArray->GetNumberOfTuples(); ++i)
        {
        receiveArray->InsertNextValue(sendArray->GetValue(i));
        }

      const int numberOfProcs = controller->GetNumberOfProcesses();
      for (int i = 0; i < numberOfProcs; ++i)
        {

        if (i == receivePid)
          {
          continue;
          }

        myprint("receiving from " << i);

        vtkMultiProcessStream receiveStream;
        controller->Receive(receiveStream, i, commTag);

        int numberOfValues = 0;
        receiveStream >> numberOfValues;

        for (int i = 0; i < numberOfValues;  ++i)
          {
          vtkstd::string tmp;
          receiveStream >> tmp;
          myprint("  received str: " << tmp);
          receiveArray->InsertNextValue(tmp);
          }
        }

      }
    else
      {

      vtkMultiProcessStream sendStream;
      int numberOfTuples = sendArray->GetNumberOfTuples();
      sendStream << numberOfTuples;
      for (int i = 0; i < sendArray->GetNumberOfTuples(); ++i)
        {
        sendStream << vtkstd::string(sendArray->GetValue(i));
        }

      myprint("sending stream to proc 0");
      controller->Send(sendStream, receivePid, commTag);
      }
}

//----------------------------------------------------------------------------
void vtkCPPartitionHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
