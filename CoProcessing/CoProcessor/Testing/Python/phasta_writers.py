
try: paraview.simple
except: from paraview.simple import *

cp_writers = []

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    timestep = datadescription.GetTimeStep()


    input_name = 'input'
    if (timestep % 1 == 0) :
        datadescription.GetInputDescriptionByName(input_name).AllFieldsOn()
        datadescription.GetInputDescriptionByName(input_name).GenerateMeshOn()
    else:
        datadescription.GetInputDescriptionByName(input_name).AllFieldsOff()
        datadescription.GetInputDescriptionByName(input_name).GenerateMeshOff()


def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"
    global cp_writers
    cp_writers = []
    timestep = datadescription.GetTimeStep()

    input_mesh = CreateProducer( datadescription, "input" )
    
    Slice1 = Slice( guiName="Slice1", SliceOffsetValues=[-12.961600000000001, -11.59722105263158, -10.23284210526316, -8.8684631578947375, -7.5040842105263152, -6.1397052631578966, -4.7753263157894743, -3.4109473684210529, -2.0465684210526316, -0.68218947368421201, 0.68218947368420935, 2.0465684210526316, 3.4109473684210521, 4.7753263157894743, 6.139705263157893, 7.5040842105263152, 8.8684631578947375, 10.23284210526316, 11.597221052631578, 12.961600000000001], SliceType="Plane" )
    Slice1.SliceType.Offset = 0.0
    Slice1.SliceType.Origin = [-2.8049160242080688, 2.1192346811294556, 1.7417440414428711]
    Slice1.SliceType.Normal = [0.0, 0.0, 1.0]
    
    SetActiveSource(input_mesh)
    Contour1 = Contour( guiName="Contour1", Isosurfaces=[-79351.699999999997, -77634.482051282044, -75917.264102564091, -74200.046153846153, -72482.828205128215, -70765.610256410248, -69048.392307692309, -67331.174358974356, -65613.956410256418, -63896.738461538451, -62179.520512820513, -60462.302564102567, -58745.084615384614, -57027.866666666669, -55310.648717948709, -53593.43076923077, -51876.212820512817, -50158.994871794872, -48441.776923076919, -46724.558974358974, -45007.341025641028, -43290.123076923075, -41572.905128205122, -39855.687179487177, -38138.469230769224, -36421.251282051278, -34704.03333333334, -32986.815384615387, -31269.597435897434, -29552.379487179489, -27835.161538461536, -26117.943589743591, -24400.725641025645, -22683.507692307692, -20966.289743589743, -19249.071794871794, -17531.853846153845, -15814.635897435901, -14097.41794871795, -12380.200000000001], ComputeNormals=1, ComputeGradients=0, ComputeScalars=0, ContourBy=['POINTS', 'pressure'], PointMergeMethod="Uniform Binning" )
    Contour1.PointMergeMethod.Numberofpointsperbucket = 8
    Contour1.PointMergeMethod.Divisions = [50, 50, 50]
    
    SetActiveSource(input_mesh)
    Calculator1 = Calculator( guiName="Calculator1", Function='mag(velocity)', ReplacementValue=0.0, ResultArrayName='vel_mag', ReplaceInvalidResults=1, AttributeMode='point_data', CoordinateResults=0 )
    
    SetActiveSource(input_mesh)
    StreamTracer1 = StreamTracer( guiName="StreamTracer1", SeedType="Point Source", IntegrationStepUnit='Cell Length', MaximumError=9.9999999999999995e-07, IntegratorType='Runge-Kutta 4-5', MaximumStepLength=0.5, InitialStepLength=0.20000000000000001, Vectors=['POINTS', 'velocity'], TerminalSpeed=9.9999999999999998e-13, IntegrationDirection='BOTH', MaximumSteps=2000, InterpolatorType='Interpolator with Point Locator', MinimumStepLength=0.01, MaximumStreamlineLength=23.351495742797901 )
    StreamTracer1.SeedType.Radius = 2.0
    StreamTracer1.SeedType.Center = [-4.692198397391973, 2.823816142866542, 3.4959603428217347]
    StreamTracer1.SeedType.NumberOfPoints = 50
    
    SetActiveSource(Slice1)
    ParallelPolyDataWriter1 = CreateWriter( XMLPPolyDataWriter, "cpout_slice_%t.pvtp", 1 )
    
    SetActiveSource(Contour1)
    ParallelPolyDataWriter2 = CreateWriter( XMLPPolyDataWriter, "cpout_contour_%t.pvtp", 1 )
    
    SetActiveSource(Calculator1)
    ParallelUnstructuredGridWriter1 = CreateWriter( XMLPUnstructuredGridWriter, "cpout_calculator_%t.pvtu", 1 )
    
    SetActiveSource(StreamTracer1)
    ParallelPolyDataWriter3 = CreateWriter( XMLPPolyDataWriter, "cpout_stream_%t.pvtp", 1 )
    


    for writer in cp_writers:
        if timestep % writer.cpFrequency == 0:
            writer.FileName = writer.cpFileName.replace("%t", str(timestep))
            writer.UpdatePipeline()

    if timestep % 1 == 0:
        renderviews = servermanager.GetRenderViews()
        imagefilename = ""
        for view in range(len(renderviews)):
            fname = imagefilename.replace("%v", str(view))
            fname = fname.replace("%t", str(timestep))
            WriteImage(fname, renderviews[view])

    # explicitly delete the proxies -- we do it this way to avoid problems with prototypes
    tobedeleted = GetProxiesToDelete()
    while len(tobedeleted) > 0:
        Delete(tobedeleted[0])
        tobedeleted = GetProxiesToDelete()

def GetProxiesToDelete():
    iter = servermanager.vtkSMProxyIterator()
    iter.Begin()
    tobedeleted = []
    while not iter.IsAtEnd():
      if iter.GetGroup().find("prototypes") != -1:
         iter.Next()
         continue
      proxy = servermanager._getPyProxy(iter.GetProxy())
      proxygroup = iter.GetGroup()
      iter.Next()
      if proxygroup != 'timekeeper' and proxy != None and proxygroup.find("pq_helper_proxies") == -1 :
          tobedeleted.append(proxy)

    return tobedeleted

def CreateProducer(datadescription, gridname):
  "Creates a producer proxy for the grid"
  if not datadescription.GetInputDescriptionByName(gridname):
    raise RuntimeError, "Simulation input name '%s' does not exist" % gridname
  grid = datadescription.GetInputDescriptionByName(gridname).GetGrid()
  producer = TrivialProducer()
  producer.GetClientSideObject().SetOutput(grid)
  producer.UpdatePipeline()
  return producer


def CreateWriter(proxy_ctor, filename, freq):
    global cp_writers
    writer = proxy_ctor()
    writer.FileName = filename
    writer.add_attribute("cpFrequency", freq)
    writer.add_attribute("cpFileName", filename)
    cp_writers.append(writer)
    return writer
