/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkPrismFilter.cxx


=========================================================================*/
#include "vtkPrismFilter.h"

#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h" 
#include "vtkCellData.h"
#include "vtkPrismSurfaceReader.h"  
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkCompositeDataIterator.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"
#include <math.h>

vtkStandardNewMacro(vtkPrismFilter);

class vtkPrismFilter::MyInternal
{
public:

    vtkSmartPointer<vtkTransformFilter> TransformFilter;
    vtkPrismSurfaceReader *Reader;
    vtkSmartPointer<vtkDoubleArray> RangeArray;
    vtkstd::string AxisVarName[3];
    MyInternal()
    {
        this->RangeArray = vtkSmartPointer<vtkDoubleArray>::New();
        this->RangeArray->Initialize();
        this->RangeArray->SetNumberOfComponents(1);


        this->Reader = vtkPrismSurfaceReader::New();
        this->AxisVarName[0]      = "none";
        this->AxisVarName[1]      = "none";
        this->AxisVarName[2]      = "none";

        this->TransformFilter=vtkSmartPointer<vtkTransformFilter>::New(); 

    }
    ~MyInternal()
    {
        if(this->Reader)
        {
            this->Reader->Delete();
        }
    } 
};

//----------------------------------------------------------------------------
vtkPrismFilter::vtkPrismFilter()
{

    this->Internal = new MyInternal();

    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(3);

}

unsigned long vtkPrismFilter::GetMTime()
{
    unsigned long time = this->Superclass::GetMTime();
    unsigned long readertime = this->Internal->Reader->GetMTime();
    return time > readertime ? time : readertime;
}

int vtkPrismFilter::IsValidFile()
{
    if(!this->Internal->Reader)
    {
        return 0;
    }

    return this->Internal->Reader->IsValidFile();

}

void vtkPrismFilter::SetFileName(const char* file)
{
    if(!this->Internal->Reader)
    {
        return;
    }

    this->Internal->Reader->SetFileName(file);
}

const char* vtkPrismFilter::GetFileName()
{
    if(!this->Internal->Reader)
    {
        return NULL;
    }
    return this->Internal->Reader->GetFileName();
}

int vtkPrismFilter::GetNumberOfTableIds()
{
    if(!this->Internal->Reader)
    {
        return 0;
    }

    return this->Internal->Reader->GetNumberOfTableIds();
}

int* vtkPrismFilter::GetTableIds()
{
    if(!this->Internal->Reader)
    {
        return NULL;
    }

    return this->Internal->Reader->GetTableIds();
}

vtkIntArray* vtkPrismFilter::GetTableIdsAsArray()
{
    if(!this->Internal->Reader)
    {
        return NULL;
    }

    return this->Internal->Reader->GetTableIdsAsArray();
}

void vtkPrismFilter::SetTable(int tableId)
{
    if(!this->Internal->Reader)
    {
        return ;
    }

    this->Internal->Reader->SetTable(tableId);
}

int vtkPrismFilter::GetTable()
{
    if(!this->Internal->Reader)
    {
        return 0;
    }

    return this->Internal->Reader->GetTable();
}

int vtkPrismFilter::GetNumberOfTableArrayNames()
{
    if(!this->Internal->Reader)
    {
        return 0;
    }

    return this->Internal->Reader->GetNumberOfTableArrayNames();
}

const char* vtkPrismFilter::GetTableArrayName(int index)
{
    if(!this->Internal->Reader)
    {
        return NULL;
    }

    return this->Internal->Reader->GetTableArrayName(index);

}

void vtkPrismFilter::SetTableArrayToProcess(const char* name)
{
    if(!this->Internal->Reader)
    {
        return ;
    }


    int numberOfArrays=this->Internal->Reader->GetNumberOfTableArrayNames();
    for(int i=0;i<numberOfArrays;i++)
    {
        this->Internal->Reader->SetTableArrayStatus(this->Internal->Reader->GetTableArrayName(i), 0);
    }
    this->Internal->Reader->SetTableArrayStatus(name, 1);

    this->SetInputArrayToProcess(
        0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
        name );
}

const char* vtkPrismFilter::GetTableArrayNameToProcess()
{
    int numberOfArrays;
    int i;

    numberOfArrays=this->Internal->Reader->GetNumberOfTableArrayNames();
    for(i=0;i<numberOfArrays;i++)
    {
        if(this->Internal->Reader->GetTableArrayStatus(this->Internal->Reader->GetTableArrayName(i)))
        {
            return this->Internal->Reader->GetTableArrayName(i);
        }
    }

    return NULL;
}

void vtkPrismFilter::SetTableArrayStatus(const char* name, int flag)
{
    if(!this->Internal->Reader)
    {
        return ;
    }

    return this->Internal->Reader->SetTableArrayStatus(name , flag);
}

int vtkPrismFilter::GetTableArrayStatus(const char* name)
{
    if(!this->Internal->Reader)
    {
        return 0 ;
    }
    return this->Internal->Reader->GetTableArrayStatus(name);
}



//----------------------------------------------------------------------------
int vtkPrismFilter::RequestData(
                                vtkInformation *request,
                                vtkInformationVector **inputVector,
                                vtkInformationVector *outputVector)
{
    this->RequestSESAMEData(request, inputVector,outputVector);
    this->RequestGeometryData(request, inputVector,outputVector);

    return 1;
}

//----------------------------------------------------------------------------
int vtkPrismFilter::RequestSESAMEData(
                                      vtkInformation *vtkNotUsed(request),
                                      vtkInformationVector **vtkNotUsed(inputVector),
                                      vtkInformationVector *outputVector)
{
    vtkstd::string filename=this->Internal->Reader->GetFileName();
    if(filename.empty())
    {
        return 1;
    }

    this->Internal->Reader->Update();


    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkPointSet *output = vtkPointSet::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkPointSet *input= this->Internal->Reader->GetOutput(0);
    output->ShallowCopy(input);



    vtkInformation *contourOutInfo = outputVector->GetInformationObject(1);
    vtkPointSet *contourOutput = vtkPointSet::SafeDownCast(
        contourOutInfo->Get(vtkDataObject::DATA_OBJECT()));


    contourOutput->ShallowCopy(this->Internal->Reader->GetOutput(1));



    return 1;
}
int vtkPrismFilter::RequestGeometryData(
                                        vtkInformation *vtkNotUsed(request),
                                        vtkInformationVector **inputVector,
                                        vtkInformationVector *outputVector)
{

    if( strcmp(this->GetXAxisVarName(), "none") == 0)
    {
        return 1;
    }



    vtkInformation *info = outputVector->GetInformationObject(2);
    vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
        info->Get(vtkDataObject::DATA_OBJECT()));
    if ( !output ) 
    {
        vtkDebugMacro( << "No output found." );
        return 0;
    }

    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    if ( !input ) 
    {
        vtkDebugMacro( << "No input found." );
        return 0;
    }


    double weight       = 0.0;
    double *weights     = NULL;
    vtkIdType cellId, ptId;
    vtkIdType numCells, numPts;
    vtkIdList *cellPts  = NULL;
    vtkDataArray *inputScalars[3];

    unsigned int j=0;
    vtkCompositeDataIterator* iter= input->NewIterator();
    iter->SkipEmptyNodesOn();
    iter->TraverseSubTreeOn();
    iter->VisitOnlyLeavesOn();
    iter->GoToFirstItem();
    while(!iter->IsDoneWithTraversal())
    {
        vtkDataSet *inputData=NULL;
        inputData=vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
        iter->GoToNextItem();
        if(inputData)
        {
            vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New(); 

            vtkPointData  *outPD = polydata->GetPointData();
            vtkCellData  *outCD = polydata->GetCellData();
            vtkPointData *inPD  = inputData->GetPointData();
            vtkCellData  *inCD  = inputData->GetCellData();
            int maxCellSize     = inputData->GetMaxCellSize();

            vtkDebugMacro( << "Mapping point data to new cell center point..." );

            // construct new points at the centers of the cells 
            vtkPoints *newPoints = vtkPoints::New();

            inputScalars[0] = inCD->GetScalars( this->GetXAxisVarName() );
            inputScalars[1] = inCD->GetScalars( this->GetYAxisVarName() );
            inputScalars[2] = inCD->GetScalars( this->GetZAxisVarName() );

            vtkIdType newIDs[1] = {0};
            if ( (numCells=inputData->GetNumberOfCells()) < 1 )
            {
                vtkDebugMacro(<< "No input cells, nothing to do." );
                return 0;
            }

            weights = new double[maxCellSize];
            cellPts = vtkIdList::New();
            cellPts->Allocate( maxCellSize );

            // Pass cell data (note that this passes current cell data through to the
            // new points that will be created at the cell centers)
            outCD->PassData( inCD );

            // create space for the newly interpolated values
            outPD->CopyAllocate( inPD,numCells );

            int abort=0;
            //double funcArgs[3]  = { 0.0, 0.0, 0.0 };
            double newPt[3] = {0.0, 0.0, 0.0};
            vtkIdType progressInterval=numCells/20 + 1;
            polydata->Allocate( numCells ); 
            for ( cellId=0; cellId < numCells && !abort; cellId++ )
            {
                if ( !(cellId % progressInterval) )
                {
                    this->UpdateProgress( (double)cellId/numCells );
                    abort = GetAbortExecute();
                }

                inputData->GetCellPoints( cellId, cellPts );
                numPts = cellPts->GetNumberOfIds();
                if ( numPts > 0 )
                {
                    weight = 1.0 / numPts;
                    for (ptId=0; ptId < numPts; ptId++)
                    {
                        weights[ptId] = weight;
                    }
                    outPD->InterpolatePoint(inPD, cellId, cellPts, weights);
                }


                newPt[0] = inputScalars[0]->GetTuple1( cellId );
                newPt[1] = inputScalars[1]->GetTuple1( cellId );
                newPt[2] = inputScalars[2]->GetTuple1( cellId );
                newIDs[0] = newPoints->InsertNextPoint( newPt );


                polydata->InsertNextCell( VTK_VERTEX, 1, newIDs );
            }

            polydata->SetPoints( newPoints );
            newPoints->Delete();
            polydata->Squeeze();
            cellPts->Delete();
            delete [] weights;


            double scale[3];
            this->Internal->Reader->GetAspectScale(scale);
            vtkSmartPointer<vtkTransform> transform= vtkSmartPointer<vtkTransform>::New();
            transform->Scale(scale);
            this->Internal->TransformFilter->SetTransform(transform);
            this->Internal->TransformFilter->SetInput(polydata);
            this->Internal->TransformFilter->Update();

            polydata->ShallowCopy(this->Internal->TransformFilter->GetOutput());

            output->SetBlock(j,polydata);
            j++;



        }
    }

    iter->Delete();

    return 1;
}



vtkDoubleArray* vtkPrismFilter::GetRanges()
{
    this->Internal->Reader->GetRanges(this->Internal->RangeArray);

    return this->Internal->RangeArray;
}



void vtkPrismFilter::SetXAxisVarName( const char *name )
{
    this->Internal->AxisVarName[0]=name;
    this->Modified();
}
void vtkPrismFilter::SetYAxisVarName( const char *name )
{
    this->Internal->AxisVarName[1]=name;
    this->Modified();
}
void vtkPrismFilter::SetZAxisVarName( const char *name )
{
    this->Internal->AxisVarName[2]=name;
    this->Modified();
}
const char * vtkPrismFilter::GetXAxisVarName()
{
    return this->Internal->AxisVarName[0].c_str();
}
const char * vtkPrismFilter::GetYAxisVarName()
{
    return this->Internal->AxisVarName[1].c_str();
}
const char * vtkPrismFilter::GetZAxisVarName()
{
    return this->Internal->AxisVarName[2].c_str();
}


//----------------------------------------------------------------------------
int vtkPrismFilter::FillOutputPortInformation(
    int port, vtkInformation* info)
{
    if(port==0)
    {
        // now add our info
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    }
       if(port==1)
    {
        // now add our info
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    }
    if(port==2)
    {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
    }

    return 1;
}


//----------------------------------------------------------------------------
void vtkPrismFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Not Implemented: " << "\n";
}



void vtkPrismFilter::SetSESAMEXAxisVarName( const char *name )
{
    this->Internal->Reader->SetXAxisVarName(name);
}

void vtkPrismFilter::SetSESAMEYAxisVarName( const char *name )
{
    this->Internal->Reader->SetYAxisVarName(name);
}

void vtkPrismFilter::SetSESAMEZAxisVarName( const char *name )
{
    this->Internal->Reader->SetZAxisVarName(name);
}

const char* vtkPrismFilter::GetSESAMEXAxisVarName()
{
    return this->Internal->Reader->GetXAxisVarName();
}

const char* vtkPrismFilter::GetSESAMEYAxisVarName()
{
    return this->Internal->Reader->GetYAxisVarName();
}

const char* vtkPrismFilter::GetSESAMEZAxisVarName()
{
    return this->Internal->Reader->GetZAxisVarName();
}


void vtkPrismFilter::SetSESAMEXLogScaling(bool b)
{
    this->Internal->Reader->SetXLogScaling(b);
}

void vtkPrismFilter::SetSESAMEYLogScaling(bool b)
{
    this->Internal->Reader->SetYLogScaling(b);
}

void vtkPrismFilter::SetSESAMEZLogScaling(bool b)
{
    this->Internal->Reader->SetZLogScaling(b);
}

bool vtkPrismFilter::GetSESAMEXLogScaling()
{
    return this->Internal->Reader->GetZLogScaling();
}

bool vtkPrismFilter::GetSESAMEYLogScaling()
{
    return this->Internal->Reader->GetYLogScaling();
}

bool vtkPrismFilter::GetSESAMEZLogScaling()
{
    return this->Internal->Reader->GetZLogScaling();
}


void vtkPrismFilter::SetSESAMEConversions(double d,double t,double p,double e)
{
    this->Internal->Reader->SetConversions(d,t,p,e);
}

double* vtkPrismFilter::GetSESAMEConversions()
{
    return this->Internal->Reader->GetConversions();
}

void vtkPrismFilter::GetSESAMEConversions (double &_arg1, double &_arg2,double &_arg3,double &_arg4)
{
    return this->Internal->Reader->GetConversions(_arg1,_arg2,_arg3,_arg4);
}

void vtkPrismFilter::GetSESAMEConversions (double _arg[4])
{
    this->Internal->Reader->GetConversions(_arg);
}

vtkDoubleArray* vtkPrismFilter:: GetSESAMEXRange()
{
    return this->Internal->Reader->GetXRange();
}

vtkDoubleArray* vtkPrismFilter:: GetSESAMEYRange()
{
    return this->Internal->Reader->GetYRange();
}


void vtkPrismFilter::SetThresholdSESAMEXBetween(double lower, double upper)
{
    this->Internal->Reader->SetThresholdXBetween(lower,upper);
}

void vtkPrismFilter::SetThresholdSESAMEYBetween(double lower, double upper)
{
    this->Internal->Reader->SetThresholdYBetween(lower,upper);
}


double* vtkPrismFilter::GetSESAMEXThresholdBetween()
{
    return this->Internal->Reader->GetXThresholdBetween();
}

void vtkPrismFilter::GetSESAMEXThresholdBetween (double &_arg1, double &_arg2)
{
    return this->Internal->Reader->GetXThresholdBetween(_arg1,_arg2);
}

void vtkPrismFilter::GetSESAMEXThresholdBetween (double _arg[2])
{
    this->Internal->Reader->GetXThresholdBetween(_arg);
}


double* vtkPrismFilter::GetSESAMEYThresholdBetween()
{
    return this->Internal->Reader->GetYThresholdBetween();
}

void vtkPrismFilter::GetSESAMEYThresholdBetween (double &_arg1, double &_arg2)
{
    return this->Internal->Reader->GetYThresholdBetween(_arg1,_arg2);
}

void vtkPrismFilter::GetSESAMEYThresholdBetween (double _arg[2])
{
    this->Internal->Reader->GetYThresholdBetween(_arg);
}



void vtkPrismFilter::SetWarpSESAMESurface(bool b)
{
    this->Internal->Reader->SetWarpSurface(b);
}

void vtkPrismFilter::SetDisplaySESAMEContours(bool b)
{
    this->Internal->Reader->SetDisplayContours(b);
}

void vtkPrismFilter::SetSESAMEContourVarName( const char *name )
{
    this->Internal->Reader->SetContourVarName(name);
}

const char* vtkPrismFilter::GetSESAMEContourVarName()
{
    return this->Internal->Reader->GetContourVarName();
}

vtkDoubleArray* vtkPrismFilter:: GetSESAMEContourVarRange()
{
    return this->Internal->Reader->GetContourVarRange();
}



void vtkPrismFilter::SetSESAMEContourValue(int i, double value)
{
    this->Internal->Reader->SetContourValue(i,value);
}

double vtkPrismFilter::GetSESAMEContourValue(int i)
{
    return this->Internal->Reader->GetContourValue(i);
}


double* vtkPrismFilter::GetSESAMEContourValues()
{
    return this->Internal->Reader->GetContourValues();
}

void vtkPrismFilter::GetSESAMEContourValues(double *contourValues)
{
    this->Internal->Reader->GetContourValues(contourValues);
}


void vtkPrismFilter::SetNumberOfSESAMEContours(int i)
{
    this->Internal->Reader->SetNumberOfContours(i);
}



vtkStringArray* vtkPrismFilter:: GetSESAMEAxisVarNames()
{
    return this->Internal->Reader->GetAxisVarNames();
}



