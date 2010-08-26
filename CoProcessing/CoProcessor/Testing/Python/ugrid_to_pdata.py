#import sys
#sys.path.append("/home/pat")
#import ugrid_to_pdata
#u = GetActiveSource().GetClientSideObject().GetOutput()
#p = paraview.vtk.vtkPolyData()
#ugrid_to_pdata.convert(u,p)
#t= TrivialProducer()
#t.GetClientSideObject().SetOutput(p)
#t.UpdatePipeline()


import paraview
vtk = paraview.vtk

def convert(u, p):

    p.SetPoints(u.GetPoints())
    p.GetPointData().AddArray(u.GetPointData().GetArray(0))
    p.GetPointData().AddArray(u.GetPointData().GetArray(1))
    p.Allocate(1, 1)

    nCells = u.GetNumberOfCells()
    triIdType = vtk.vtkTriangle().GetCellType()
    vertexIdType = vtk.vtkVertex().GetCellType()

    tetIdList = vtk.vtkIdList()
    triIdList = vtk.vtkIdList()
    triIdList.SetNumberOfIds(3)
    triIdList.SetId(0, 0)
    triIdList.SetId(1, 1)
    triIdList.SetId(2, 2)
    vertexIdList = vtk.vtkIdList()
    vertexIdList.SetNumberOfIds(1)

    for i in xrange(nCells):
        u.GetCellPoints(i, tetIdList)
        triIdList.SetId(0, tetIdList.GetId(0))
        triIdList.SetId(1, tetIdList.GetId(1))
        triIdList.SetId(2, tetIdList.GetId(2))
        p.InsertNextCell(triIdType, triIdList)

        #triIdList.SetId(0, tetIdList.GetId(0))
        #triIdList.SetId(1, tetIdList.GetId(1))
        #triIdList.SetId(2, tetIdList.GetId(3))
        #p.InsertNextCell(triIdType, triIdList)

        #triIdList.SetId(0, tetIdList.GetId(0))
        #triIdList.SetId(1, tetIdList.GetId(3))
        #triIdList.SetId(2, tetIdList.GetId(2))
        #p.InsertNextCell(triIdType, triIdList)

        #triIdList.SetId(0, tetIdList.GetId(2))
        #triIdList.SetId(1, tetIdList.GetId(3))
        #triIdList.SetId(2, tetIdList.GetId(1))
        #p.InsertNextCell(triIdType, triIdList)

        vertexIdList.SetId(0, tetIdList.GetId(3))
        p.InsertNextCell(vertexIdType, vertexIdList)

