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
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkSmartPointer.h"

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
bool vtkCPPartitionHelper::AggregateDataObject(vtkDataObject* in, vtkDataObject* out,
                                  vtkMultiProcessController* controller, int receiveProcessId)
{
  if (!vtkUnstructuredGrid::SafeDownCast(in) && !vtkPolyData::SafeDownCast(in))
    {
    vtkGenericWarningMacro("Cannot aggregate data object type " << in->GetClassName())
    return false;
    }

  vtkPointSet* grid = vtkPointSet::SafeDownCast(in);
  vtkPointSet* outGrid = vtkPointSet::SafeDownCast(out);

  vtkIdType pid = controller->GetLocalProcessId();
  vtkIdType numberOfProcesses = controller->GetNumberOfProcesses();

  vtkIdType numberOfPoints = grid->GetNumberOfPoints();
  vtkIdType numberOfCells = grid->GetNumberOfCells();

  myprint("local number of points/cells " << numberOfPoints << " / " << numberOfCells);


  // Get the pointdata as a double array.  If the safe down cast returns null then create
  // a new double array and deepcopy the point data to convert it to double
  vtkSmartPointer<vtkDoubleArray> pointsArray = vtkDoubleArray::SafeDownCast(grid->GetPoints()->GetData());
  if (!pointsArray)
    {
    pointsArray = vtkSmartPointer<vtkDoubleArray>::New();
    pointsArray->SetNumberOfComponents(3);
    pointsArray->DeepCopy(grid->GetPoints()->GetData());
    }

  vtkIdTypeArray* cellsArray;
  vtkIdType cellElementSize = 0;
  if (vtkUnstructuredGrid::SafeDownCast(grid))
    {
    cellsArray = vtkUnstructuredGrid::SafeDownCast(grid)->GetCells()->GetData();
    cellElementSize = 5;
    }
  else if (vtkPolyData::SafeDownCast(grid))
    {
    cellsArray = vtkPolyData::SafeDownCast(grid)->GetPolys()->GetData();
    cellElementSize = 4;
    }
  vtkDoubleArray* scalarArray = vtkDoubleArray::SafeDownCast(grid->GetPointData()->GetArray("pressure"));
  vtkDoubleArray* vectorArray = vtkDoubleArray::SafeDownCast(grid->GetPointData()->GetArray("velocity"));

  if (!pointsArray)
    {
    myprint("points array is actually " << grid->GetPoints()->GetData()->GetClassName());
    vtkGenericWarningMacro("Failed to get points array.");
    return false;
    }
  if (!cellsArray)
    {
    vtkGenericWarningMacro("Failed to get cells array.");
    return false;
    }
  if (!scalarArray)
    {
    vtkGenericWarningMacro("Failed to get scalar array.");
    return false;
    }
  if (!vectorArray)
    {
    vtkGenericWarningMacro("Failed to get vector array.");
    return false;
    }


  // Gather array containing the point counts for all processes
  vtkSmartPointer<vtkIdTypeArray> localPointCount = vtkSmartPointer<vtkIdTypeArray>::New();
  localPointCount->SetNumberOfTuples(1);
  localPointCount->SetValue(0, numberOfPoints);
  vtkSmartPointer<vtkIdTypeArray> pointCounts = vtkSmartPointer<vtkIdTypeArray>::New();
  pointCounts->SetNumberOfTuples(numberOfProcesses);
  controller->AllGather(localPointCount, pointCounts);

  // Gather array containing the cell counts for all processes
  vtkSmartPointer<vtkIdTypeArray> localCellCount = vtkSmartPointer<vtkIdTypeArray>::New();
  localCellCount->SetNumberOfTuples(1);
  localCellCount->SetValue(0, numberOfCells);
  vtkSmartPointer<vtkIdTypeArray> cellCounts = vtkSmartPointer<vtkIdTypeArray>::New();
  cellCounts->SetNumberOfTuples(numberOfProcesses);
  controller->AllGather(localCellCount, cellCounts);


  if (pid == receiveProcessId)
    {

    vtkIdType totalPointCount = 0;
    vtkIdType totalCellCount = 0;
    for (vtkIdType i = 0; i < numberOfProcesses; ++i)
      {
      totalPointCount += pointCounts->GetValue(i);
      totalCellCount += cellCounts->GetValue(i);
      }

    myprint("total number of points/cells " << totalPointCount << " / " << totalCellCount);

    // receive arrays
    double* pointsArrayData = new double[totalPointCount*3];
    vtkIdType* cellsArrayData = new vtkIdType[totalCellCount*cellElementSize];
    double* scalarArrayData = new double[totalPointCount];
    double* vectorArrayData = new double[totalPointCount*3];

    vtkIdType pointsReceiveOffset = 0;
    vtkIdType cellsReceiveOffset = 0;
    for (vtkIdType i = 0; i < numberOfProcesses; ++i)
      {
      double* pointsArrayReceive = pointsArrayData + pointsReceiveOffset*3;
      vtkIdType* cellsArrayReceive = cellsArrayData + cellsReceiveOffset*cellElementSize;
      double* scalarArrayReceive = scalarArrayData + pointsReceiveOffset;
      double* vectorArrayReceive = vectorArrayData + pointsReceiveOffset*3;

      myprint("receiving from " << i << " offsets are " << pointsReceiveOffset << " / " << cellsReceiveOffset);

      pointsReceiveOffset += pointCounts->GetValue(i);
      cellsReceiveOffset += cellCounts->GetValue(i);

      if (i == receiveProcessId)
        {
        memcpy(pointsArrayReceive, pointsArray->GetPointer(0), numberOfPoints*3*sizeof(double));
        memcpy(cellsArrayReceive, cellsArray->GetPointer(0), numberOfCells*cellElementSize*sizeof(vtkIdType));
        memcpy(scalarArrayReceive, scalarArray->GetPointer(0), numberOfPoints*sizeof(double));
        memcpy(vectorArrayReceive, vectorArray->GetPointer(0), numberOfPoints*3*sizeof(double));
        }
      else
        {
        controller->Receive(pointsArrayReceive, pointCounts->GetValue(i)*3, i, 45670);
        controller->Receive(cellsArrayReceive, cellCounts->GetValue(i)*cellElementSize, i, 45671);
        controller->Receive(scalarArrayReceive, pointCounts->GetValue(i), i, 45672);
        controller->Receive(vectorArrayReceive, pointCounts->GetValue(i)*3, i, 45673);
        }
      }


    myprint("reindexing cell array");

    // Re-index the point ids in the cell array
    vtkIdType cellIndex = 0;
    vtkIdType pointIdOffset = 0;
    for (vtkIdType i = 0; i < cellCounts->GetNumberOfTuples(); ++i)
      {
      vtkIdType processCellCount = cellCounts->GetValue(i);
      myprint("looping over process " << i << " cell count " << processCellCount);
      for (vtkIdType j = 0; j < processCellCount; ++j)
        {
        cellIndex++;
        for (vtkIdType k = 1; k < cellElementSize; ++k)
          {
          cellsArrayData[cellIndex++] += pointIdOffset;
          }
        }
      pointIdOffset += pointCounts->GetValue(i);
      }


    pointsArray = vtkDoubleArray::New();
    pointsArray->SetNumberOfComponents(3);
    pointsArray->SetArray(pointsArrayData, totalPointCount*3, 0);
    vtkPoints* points = vtkPoints::New();
    points->SetData(pointsArray);

    cellsArray = vtkIdTypeArray::New();
    cellsArray->SetArray(cellsArrayData, totalCellCount*cellElementSize, 0);
    vtkCellArray* cells = vtkCellArray::New();
    cells->SetCells(totalCellCount, cellsArray);

    scalarArray = vtkDoubleArray::New();
    scalarArray->SetArray(scalarArrayData, totalPointCount, 0);
    scalarArray->SetName("pressure");

    vectorArray = vtkDoubleArray::New();
    vectorArray->SetNumberOfComponents(3);
    vectorArray->SetArray(vectorArrayData, totalPointCount*3, 0);
    vectorArray->SetName("velocity");
    
    outGrid->SetPoints(points);
    if (vtkUnstructuredGrid::SafeDownCast(outGrid))
      {
      vtkUnstructuredGrid::SafeDownCast(outGrid)->SetCells(VTK_TETRA, cells);
      }
    else if (vtkPolyData::SafeDownCast(outGrid))
      {
      vtkPolyData::SafeDownCast(outGrid)->SetPolys(cells);
      }

    outGrid->GetPointData()->AddArray(scalarArray);
    outGrid->GetPointData()->AddArray(vectorArray);

    pointsArray->Delete();
    points->Delete();
    cellsArray->Delete();
    cells->Delete();
    scalarArray->Delete();
    vectorArray->Delete();

    }
  else
    {

    myprint("sending to " << receiveProcessId);
    // send arrays
    controller->Send(pointsArray->GetPointer(0), numberOfPoints*3, receiveProcessId, 45670);
    controller->Send(cellsArray->GetPointer(0), numberOfCells*cellElementSize, receiveProcessId, 45671);
    controller->Send(scalarArray->GetPointer(0), numberOfPoints, receiveProcessId, 45672);
    controller->Send(vectorArray->GetPointer(0), numberOfPoints*3, receiveProcessId, 45673);
    }

  return true;
}

//----------------------------------------------------------------------------
void vtkCPPartitionHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
