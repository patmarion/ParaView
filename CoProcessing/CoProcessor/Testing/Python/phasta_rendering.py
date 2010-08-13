
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

    RenderView2 = CreateRenderView()
    RenderView2.LightSpecularColor = [1.0, 1.0, 1.0]
    RenderView2.UseLight = 1
    RenderView2.CameraPosition = [26.745499260625301, 2.0325416829163081, 1.8983660180214521]
    RenderView2.FillLightKFRatio = 3.0
    RenderView2.Background2 = [0.0, 0.0, 0.16500000000000001]
    RenderView2.FillLightAzimuth = -10.0
    RenderView2.LODResolution = 50
    RenderView2.BackgroundTexture = []
    RenderView2.KeyLightAzimuth = 10.0
    RenderView2.LightIntensity = 1.0
    RenderView2.CameraFocalPoint = [-3.2533699999999999, 2.2128000000000001, 1.71035]
    RenderView2.RenderInterruptsEnabled = 0
    RenderView2.CameraParallelScale = 1.0
    RenderView2.EyeAngle = 2.0
    RenderView2.HeadLightKHRatio = 3.0
    RenderView2.UseTriangleStrips = 0
    RenderView2.StereoRender = 0
    RenderView2.CameraViewAngle = 30.0
    RenderView2.KeyLightIntensity = 0.75
    RenderView2.BackLightAzimuth = 110.0
    RenderView2.OrientationAxesInteractivity = 0
    RenderView2.UseTexturedBackground = 0
    RenderView2.Background = [0.31999694819562063, 0.34000152590218968, 0.42999923704890519]
    RenderView2.UseOffscreenRenderingForScreenshots = 1
    RenderView2.CenterOfRotation = [-3.4324660450220108, 2.271648108959198, 1.7417440414428711]
    RenderView2.CameraParallelProjection = 0
    RenderView2.CameraViewUp = [-0.0062672009005881515, -0.99909408344726436, 0.042091978018888504]
    RenderView2.HeadLightWarmth = 0.5
    RenderView2.MaximumNumberOfPeels = 4
    RenderView2.LightDiffuseColor = [1.0, 1.0, 1.0]
    RenderView2.StereoType = 'Red-Blue'
    RenderView2.DepthPeeling = 1
    RenderView2.BackLightKBRatio = 3.5
    RenderView2.UseImmediateMode = 1
    RenderView2.LightAmbientColor = [1.0, 1.0, 1.0]
    RenderView2.KeyLightElevation = 50.0
    RenderView2.CenterAxesVisibility = 0
    RenderView2.MaintainLuminance = 0
    RenderView2.BackLightWarmth = 0.5
    RenderView2.FillLightElevation = -75.0
    RenderView2.FillLightWarmth = 0.40000000000000002
    RenderView2.LightSwitch = 0
    RenderView2.OrientationAxesVisibility = 1
    RenderView2.CameraClippingRange = [18.021983990872851, 71.836946970533589]
    RenderView2.BackLightElevation = 0.0
    RenderView2.ViewTime = 1.0
    RenderView2.OrientationAxesOutlineColor = [1.0, 1.0, 1.0]
    RenderView2.LODThreshold = 5.0
    RenderView2.UseGradientBackground = 0
    RenderView2.KeyLightWarmth = 0.59999999999999998
    RenderView2.OrientationAxesLabelColor = [1.0, 1.0, 1.0]
    RenderView2.ViewSize = [800, 400]
    
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
    
    a3_velocity_PiecewiseFunction = CreatePiecewiseFunction( Points=[0.0, 0.0, 1.0, 1.0] )
    
    a1_pressure_PiecewiseFunction = CreatePiecewiseFunction( Points=[0.0, 0.0, 1.0, 1.0] )
    
    a1_vel_mag_PiecewiseFunction = CreatePiecewiseFunction( Points=[0.0, 0.0, 1.0, 1.0] )
    
    a1_Rotation_PiecewiseFunction = CreatePiecewiseFunction( Points=[0.0, 0.0, 1.0, 1.0] )
    
    a3_Vorticity_PiecewiseFunction = CreatePiecewiseFunction( Points=[0.0, 0.0, 1.0, 1.0] )
    
    a3_velocity_PVLookupTable = GetLookupTableForArray( "velocity", 3, Discretize=1, RGBPoints=[0.0, 0.23000000000000001, 0.29899999999999999, 0.754, 190.88937819361666, 0.70599999999999996, 0.016, 0.14999999999999999], UseLogScale=0, VectorComponent=0, NumberOfTableValues=256, ColorSpace='Diverging', VectorMode='Magnitude', HSVWrap=0, ScalarRangeInitialized=1.0, LockScalarRange=0 )
    
    a1_pressure_PVLookupTable = GetLookupTableForArray( "pressure", 1, Discretize=1, RGBPoints=[-79351.700000000012, 0.23000000000000001, 0.29899999999999999, 0.754, -12680.127144435206, 0.70599999999999996, 0.016, 0.14999999999999999], UseLogScale=0, VectorComponent=0, NumberOfTableValues=256, ColorSpace='Diverging', VectorMode='Magnitude', HSVWrap=0, ScalarRangeInitialized=1.0, LockScalarRange=0 )
    
    a1_vel_mag_PVLookupTable = GetLookupTableForArray( "vel_mag", 1, Discretize=1, RGBPoints=[0.0, 0.23000000000000001, 0.29899999999999999, 0.754, 191.99556483266474, 0.70599999999999996, 0.016, 0.14999999999999999], UseLogScale=0, VectorComponent=0, NumberOfTableValues=256, ColorSpace='Diverging', VectorMode='Magnitude', HSVWrap=0, ScalarRangeInitialized=1.0, LockScalarRange=0 )
    
    a1_Rotation_PVLookupTable = GetLookupTableForArray( "Rotation", 1, Discretize=1, RGBPoints=[-195.09524824489776, 0.23000000000000001, 0.29899999999999999, 0.754, 366.07845627765187, 0.70599999999999996, 0.016, 0.14999999999999999], UseLogScale=0, VectorComponent=0, NumberOfTableValues=256, ColorSpace='Diverging', VectorMode='Magnitude', HSVWrap=0, ScalarRangeInitialized=1.0, LockScalarRange=0 )
    
    a3_Vorticity_PVLookupTable = GetLookupTableForArray( "Vorticity", 3, Discretize=1, RGBPoints=[1.0466654487584488, 0.23000000000000001, 0.29899999999999999, 0.754, 4719.8843019691176, 0.70599999999999996, 0.016, 0.14999999999999999], UseLogScale=0, VectorComponent=0, NumberOfTableValues=256, ColorSpace='Diverging', VectorMode='Magnitude', HSVWrap=0, ScalarRangeInitialized=1.0, LockScalarRange=0 )
    
    ScalarBarWidgetRepresentation1 = CreateScalarBar( Title='vel_mag', Position2=[0.12999999999999998, 0.5], TitleOpacity=1.0, TitleShadow=0, AutomaticLabelFormat=1, TitleFontSize=12, TitleColor=[1.0, 1.0, 1.0], AspectRatio=20.0, NumberOfLabels=5, ComponentTitle='', Resizable=1, TitleFontFamily='Arial', Visibility=0, LabelFontSize=12, LabelFontFamily='Arial', TitleItalic=0, Selectable=0, LabelItalic=0, Enabled=0, LabelColor=[1.0, 1.0, 1.0], Position=[0.12107604017216647, 0.18899204244031836], LabelBold=0, LabelOpacity=1.0, TitleBold=0, LabelFormat='%-#6.3g', Orientation='Vertical', LabelShadow=0, LookupTable=a1_vel_mag_PVLookupTable, Repositionable=1 )
    GetRenderView().Representations.append(ScalarBarWidgetRepresentation1)
    
    SetActiveSource(input_mesh)
    DataRepresentation7 = Show()
    DataRepresentation7.CubeAxesZAxisVisibility = 1
    DataRepresentation7.SelectionPointLabelColor = [1.0, 1.0, 1.0]
    DataRepresentation7.SelectionPointFieldDataArrayName = ''
    DataRepresentation7.SuppressLOD = 0
    DataRepresentation7.CubeAxesXGridLines = 0
    DataRepresentation7.CubeAxesYAxisTickVisibility = 1
    DataRepresentation7.Position = [0.0, 0.0, 0.0]
    DataRepresentation7.BackfaceRepresentation = 'Follow Frontface'
    DataRepresentation7.SelectionOpacity = 1.0
    DataRepresentation7.SelectionPointLabelShadow = 0
    DataRepresentation7.CubeAxesYGridLines = 0
    DataRepresentation7.Shading = 0
    DataRepresentation7.Diffuse = 1.0
    DataRepresentation7.Origin = [0.0, 0.0, 0.0]
    DataRepresentation7.CubeAxesZTitle = 'Z-Axis'
    DataRepresentation7.Specular = 0.10000000000000001
    DataRepresentation7.SelectionVisibility = 1
    DataRepresentation7.InterpolateScalarsBeforeMapping = 1
    DataRepresentation7.CubeAxesZAxisTickVisibility = 1
    DataRepresentation7.SelectionUseOutline = 0
    DataRepresentation7.SelectionCellFieldDataArrayIndex = 0
    DataRepresentation7.CubeAxesVisibility = 0
    DataRepresentation7.Scale = [1.0, 1.0, 1.0]
    DataRepresentation7.SelectionCellLabelJustification = 'Center'
    DataRepresentation7.DiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation7.SelectionCellLabelOpacity = 1.0
    DataRepresentation7.CubeAxesInertia = 1
    DataRepresentation7.Opacity = 1.0
    DataRepresentation7.LineWidth = 1.0
    DataRepresentation7.SelectionPointSize = 5.0
    DataRepresentation7.Material = ''
    DataRepresentation7.Visibility = 0
    DataRepresentation7.SelectionCellLabelFontSize = 24
    DataRepresentation7.CubeAxesCornerOffset = 0.0
    DataRepresentation7.SelectionPointLabelJustification = 'Center'
    DataRepresentation7.Ambient = 0.0
    DataRepresentation7.SelectedMapperIndex = 'Projected tetra'
    DataRepresentation7.CubeAxesTickLocation = 'Inside'
    DataRepresentation7.BackfaceDiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation7.CubeAxesYAxisVisibility = 1
    DataRepresentation7.SelectionPointLabelFontFamily = 'Arial'
    DataRepresentation7.CubeAxesFlyMode = 'Closest Triad'
    DataRepresentation7.CubeAxesYTitle = 'Y-Axis'
    DataRepresentation7.SelectionPointFieldDataArrayIndex = 0
    DataRepresentation7.ColorAttributeType = 'CELL_DATA'
    DataRepresentation7.SpecularPower = 100.0
    DataRepresentation7.Texture = []
    DataRepresentation7.SelectionCellLabelShadow = 0
    DataRepresentation7.AmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation7.MapScalars = 1
    DataRepresentation7.PointSize = 2.0
    DataRepresentation7.StaticMode = 0
    DataRepresentation7.SelectionCellLabelColor = [0.0, 1.0, 0.0]
    DataRepresentation7.EdgeColor = [0.0, 0.0, 0.50000762951094835]
    DataRepresentation7.CubeAxesXAxisTickVisibility = 1
    DataRepresentation7.SelectionCellLabelVisibility = 0
    DataRepresentation7.NonlinearSubdivisionLevel = 1
    DataRepresentation7.CubeAxesColor = [1.0, 1.0, 1.0]
    DataRepresentation7.Representation = 'Surface'
    DataRepresentation7.CustomBounds = [0.0, 1.0, 0.0, 1.0, 0.0, 1.0]
    DataRepresentation7.CubeAxesXAxisMinorTickVisibility = 1
    DataRepresentation7.Orientation = [0.0, 0.0, 0.0]
    DataRepresentation7.UseLookupTableScalarRange = 0
    DataRepresentation7.CubeAxesXTitle = 'X-Axis'
    DataRepresentation7.ScalarOpacityUnitDistance = 0.41991230410220659
    DataRepresentation7.BackfaceOpacity = 1.0
    DataRepresentation7.SelectionCellFieldDataArrayName = ''
    DataRepresentation7.SelectionColor = [1.0, 0.0, 1.0]
    DataRepresentation7.SelectionPointLabelVisibility = 0
    DataRepresentation7.SelectionPointLabelFontSize = 18
    DataRepresentation7.BackfaceAmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation7.ScalarOpacityFunction = []
    DataRepresentation7.SelectionLineWidth = 2.0
    DataRepresentation7.CubeAxesZAxisMinorTickVisibility = 1
    DataRepresentation7.CubeAxesXAxisVisibility = 1
    DataRepresentation7.Interpolation = 'Gouraud'
    DataRepresentation7.SelectionCellLabelFontFamily = 'Arial'
    DataRepresentation7.SelectionCellLabelItalic = 0
    DataRepresentation7.CubeAxesYAxisMinorTickVisibility = 1
    DataRepresentation7.CubeAxesZGridLines = 0
    DataRepresentation7.ExtractedBlockIndex = 0
    DataRepresentation7.SelectionPointLabelOpacity = 1.0
    DataRepresentation7.Pickable = 1
    DataRepresentation7.CustomBoundsActive = [0, 0, 0]
    DataRepresentation7.SelectionRepresentation = 'Wireframe'
    DataRepresentation7.ClippingPlanes = []
    DataRepresentation7.SelectionPointLabelBold = 0
    DataRepresentation7.NumberOfSubPieces = 1
    DataRepresentation7.ColorArrayName = ''
    DataRepresentation7.SelectionPointLabelItalic = 0
    DataRepresentation7.SpecularColor = [1.0, 1.0, 1.0]
    DataRepresentation7.LookupTable = []
    DataRepresentation7.SelectionCellLabelBold = 0
    
    SetActiveSource(Slice1)
    DataRepresentation10 = Show()
    DataRepresentation10.CubeAxesZAxisVisibility = 1
    DataRepresentation10.SelectionPointLabelColor = [1.0, 1.0, 1.0]
    DataRepresentation10.SelectionPointFieldDataArrayName = ''
    DataRepresentation10.SuppressLOD = 0
    DataRepresentation10.CubeAxesXGridLines = 0
    DataRepresentation10.CubeAxesYAxisTickVisibility = 1
    DataRepresentation10.Position = [0.0, 0.0, 0.0]
    DataRepresentation10.BackfaceRepresentation = 'Follow Frontface'
    DataRepresentation10.SelectionOpacity = 1.0
    DataRepresentation10.SelectionPointLabelShadow = 0
    DataRepresentation10.CubeAxesYGridLines = 0
    DataRepresentation10.Shading = 0
    DataRepresentation10.Diffuse = 1.0
    DataRepresentation10.Origin = [0.0, 0.0, 0.0]
    DataRepresentation10.CubeAxesZTitle = 'Z-Axis'
    DataRepresentation10.Specular = 0.10000000000000001
    DataRepresentation10.SelectionVisibility = 1
    DataRepresentation10.InterpolateScalarsBeforeMapping = 1
    DataRepresentation10.CubeAxesZAxisTickVisibility = 1
    DataRepresentation10.SelectionUseOutline = 0
    DataRepresentation10.SelectionCellFieldDataArrayIndex = 0
    DataRepresentation10.CubeAxesVisibility = 0
    DataRepresentation10.Scale = [1.0, 1.0, 1.0]
    DataRepresentation10.SelectionCellLabelJustification = 'Center'
    DataRepresentation10.DiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation10.SelectionCellLabelOpacity = 1.0
    DataRepresentation10.Opacity = 1.0
    DataRepresentation10.LineWidth = 1.0
    DataRepresentation10.SelectionPointSize = 5.0
    DataRepresentation10.Material = ''
    DataRepresentation10.Visibility = 1
    DataRepresentation10.SelectionCellLabelFontSize = 24
    DataRepresentation10.CubeAxesCornerOffset = 0.0
    DataRepresentation10.SelectionPointLabelJustification = 'Center'
    DataRepresentation10.Ambient = 0.0
    DataRepresentation10.CubeAxesTickLocation = 'Inside'
    DataRepresentation10.BackfaceDiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation10.CubeAxesYAxisVisibility = 1
    DataRepresentation10.SelectionPointLabelFontFamily = 'Arial'
    DataRepresentation10.CubeAxesFlyMode = 'Closest Triad'
    DataRepresentation10.CubeAxesYTitle = 'Y-Axis'
    DataRepresentation10.SelectionPointFieldDataArrayIndex = 0
    DataRepresentation10.ColorAttributeType = 'POINT_DATA'
    DataRepresentation10.SpecularPower = 100.0
    DataRepresentation10.Texture = []
    DataRepresentation10.SelectionCellLabelShadow = 0
    DataRepresentation10.AmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation10.MapScalars = 1
    DataRepresentation10.PointSize = 2.0
    DataRepresentation10.StaticMode = 0
    DataRepresentation10.SelectionCellLabelColor = [0.0, 1.0, 0.0]
    DataRepresentation10.EdgeColor = [0.0, 0.0, 0.50000762951094835]
    DataRepresentation10.CubeAxesXAxisTickVisibility = 1
    DataRepresentation10.SelectionCellLabelVisibility = 0
    DataRepresentation10.NonlinearSubdivisionLevel = 1
    DataRepresentation10.CubeAxesColor = [1.0, 1.0, 1.0]
    DataRepresentation10.Representation = 'Surface'
    DataRepresentation10.CustomBounds = [0.0, 1.0, 0.0, 1.0, 0.0, 1.0]
    DataRepresentation10.CubeAxesXAxisMinorTickVisibility = 1
    DataRepresentation10.Orientation = [0.0, 0.0, 0.0]
    DataRepresentation10.UseLookupTableScalarRange = 0
    DataRepresentation10.CubeAxesXTitle = 'X-Axis'
    DataRepresentation10.CubeAxesInertia = 1
    DataRepresentation10.BackfaceOpacity = 1.0
    DataRepresentation10.SelectionCellFieldDataArrayName = ''
    DataRepresentation10.SelectionColor = [1.0, 0.0, 1.0]
    DataRepresentation10.SelectionPointLabelVisibility = 0
    DataRepresentation10.SelectionPointLabelFontSize = 18
    DataRepresentation10.BackfaceAmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation10.SelectionLineWidth = 2.0
    DataRepresentation10.CubeAxesZAxisMinorTickVisibility = 1
    DataRepresentation10.CubeAxesXAxisVisibility = 1
    DataRepresentation10.Interpolation = 'Gouraud'
    DataRepresentation10.SelectionCellLabelFontFamily = 'Arial'
    DataRepresentation10.SelectionCellLabelItalic = 0
    DataRepresentation10.CubeAxesYAxisMinorTickVisibility = 1
    DataRepresentation10.CubeAxesZGridLines = 0
    DataRepresentation10.SelectionPointLabelOpacity = 1.0
    DataRepresentation10.Pickable = 1
    DataRepresentation10.CustomBoundsActive = [0, 0, 0]
    DataRepresentation10.SelectionRepresentation = 'Wireframe'
    DataRepresentation10.ClippingPlanes = []
    DataRepresentation10.SelectionPointLabelBold = 0
    DataRepresentation10.NumberOfSubPieces = 1
    DataRepresentation10.ColorArrayName = 'velocity'
    DataRepresentation10.SelectionPointLabelItalic = 0
    DataRepresentation10.SpecularColor = [1.0, 1.0, 1.0]
    DataRepresentation10.LookupTable = a3_velocity_PVLookupTable
    DataRepresentation10.SelectionCellLabelBold = 0
    
    SetActiveSource(Contour1)
    DataRepresentation11 = Show()
    DataRepresentation11.CubeAxesZAxisVisibility = 1
    DataRepresentation11.SelectionPointLabelColor = [1.0, 1.0, 1.0]
    DataRepresentation11.SelectionPointFieldDataArrayName = ''
    DataRepresentation11.SuppressLOD = 0
    DataRepresentation11.CubeAxesXGridLines = 0
    DataRepresentation11.CubeAxesYAxisTickVisibility = 1
    DataRepresentation11.Position = [0.0, 0.0, 0.0]
    DataRepresentation11.BackfaceRepresentation = 'Follow Frontface'
    DataRepresentation11.SelectionOpacity = 1.0
    DataRepresentation11.SelectionPointLabelShadow = 0
    DataRepresentation11.CubeAxesYGridLines = 0
    DataRepresentation11.Shading = 0
    DataRepresentation11.Diffuse = 1.0
    DataRepresentation11.Origin = [0.0, 0.0, 0.0]
    DataRepresentation11.CubeAxesZTitle = 'Z-Axis'
    DataRepresentation11.Specular = 0.10000000000000001
    DataRepresentation11.SelectionVisibility = 1
    DataRepresentation11.InterpolateScalarsBeforeMapping = 1
    DataRepresentation11.CubeAxesZAxisTickVisibility = 1
    DataRepresentation11.SelectionUseOutline = 0
    DataRepresentation11.SelectionCellFieldDataArrayIndex = 0
    DataRepresentation11.CubeAxesVisibility = 0
    DataRepresentation11.Scale = [1.0, 1.0, 1.0]
    DataRepresentation11.SelectionCellLabelJustification = 'Center'
    DataRepresentation11.DiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation11.SelectionCellLabelOpacity = 1.0
    DataRepresentation11.Opacity = 1.0
    DataRepresentation11.LineWidth = 1.0
    DataRepresentation11.SelectionPointSize = 5.0
    DataRepresentation11.Material = ''
    DataRepresentation11.Visibility = 1
    DataRepresentation11.SelectionCellLabelFontSize = 24
    DataRepresentation11.CubeAxesCornerOffset = 0.0
    DataRepresentation11.SelectionPointLabelJustification = 'Center'
    DataRepresentation11.Ambient = 0.0
    DataRepresentation11.CubeAxesTickLocation = 'Inside'
    DataRepresentation11.BackfaceDiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation11.CubeAxesYAxisVisibility = 1
    DataRepresentation11.SelectionPointLabelFontFamily = 'Arial'
    DataRepresentation11.CubeAxesFlyMode = 'Closest Triad'
    DataRepresentation11.CubeAxesYTitle = 'Y-Axis'
    DataRepresentation11.SelectionPointFieldDataArrayIndex = 0
    DataRepresentation11.ColorAttributeType = 'POINT_DATA'
    DataRepresentation11.SpecularPower = 100.0
    DataRepresentation11.Texture = []
    DataRepresentation11.SelectionCellLabelShadow = 0
    DataRepresentation11.AmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation11.MapScalars = 1
    DataRepresentation11.PointSize = 2.0
    DataRepresentation11.StaticMode = 0
    DataRepresentation11.SelectionCellLabelColor = [0.0, 1.0, 0.0]
    DataRepresentation11.EdgeColor = [0.0, 0.0, 0.50000762951094835]
    DataRepresentation11.CubeAxesXAxisTickVisibility = 1
    DataRepresentation11.SelectionCellLabelVisibility = 0
    DataRepresentation11.NonlinearSubdivisionLevel = 1
    DataRepresentation11.CubeAxesColor = [1.0, 1.0, 1.0]
    DataRepresentation11.Representation = 'Surface'
    DataRepresentation11.CustomBounds = [0.0, 1.0, 0.0, 1.0, 0.0, 1.0]
    DataRepresentation11.CubeAxesXAxisMinorTickVisibility = 1
    DataRepresentation11.Orientation = [0.0, 0.0, 0.0]
    DataRepresentation11.UseLookupTableScalarRange = 0
    DataRepresentation11.CubeAxesXTitle = 'X-Axis'
    DataRepresentation11.CubeAxesInertia = 1
    DataRepresentation11.BackfaceOpacity = 1.0
    DataRepresentation11.SelectionCellFieldDataArrayName = ''
    DataRepresentation11.SelectionColor = [1.0, 0.0, 1.0]
    DataRepresentation11.SelectionPointLabelVisibility = 0
    DataRepresentation11.SelectionPointLabelFontSize = 18
    DataRepresentation11.BackfaceAmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation11.SelectionLineWidth = 2.0
    DataRepresentation11.CubeAxesZAxisMinorTickVisibility = 1
    DataRepresentation11.CubeAxesXAxisVisibility = 1
    DataRepresentation11.Interpolation = 'Gouraud'
    DataRepresentation11.SelectionCellLabelFontFamily = 'Arial'
    DataRepresentation11.SelectionCellLabelItalic = 0
    DataRepresentation11.CubeAxesYAxisMinorTickVisibility = 1
    DataRepresentation11.CubeAxesZGridLines = 0
    DataRepresentation11.SelectionPointLabelOpacity = 1.0
    DataRepresentation11.Pickable = 1
    DataRepresentation11.CustomBoundsActive = [0, 0, 0]
    DataRepresentation11.SelectionRepresentation = 'Wireframe'
    DataRepresentation11.ClippingPlanes = []
    DataRepresentation11.SelectionPointLabelBold = 0
    DataRepresentation11.NumberOfSubPieces = 1
    DataRepresentation11.ColorArrayName = 'pressure'
    DataRepresentation11.SelectionPointLabelItalic = 0
    DataRepresentation11.SpecularColor = [1.0, 1.0, 1.0]
    DataRepresentation11.LookupTable = a1_pressure_PVLookupTable
    DataRepresentation11.SelectionCellLabelBold = 0
    
    SetActiveSource(Calculator1)
    DataRepresentation12 = Show()
    DataRepresentation12.CubeAxesZAxisVisibility = 1
    DataRepresentation12.SelectionPointLabelColor = [1.0, 1.0, 1.0]
    DataRepresentation12.SelectionPointFieldDataArrayName = ''
    DataRepresentation12.SuppressLOD = 0
    DataRepresentation12.CubeAxesXGridLines = 0
    DataRepresentation12.CubeAxesYAxisTickVisibility = 1
    DataRepresentation12.Position = [0.0, 0.0, 0.0]
    DataRepresentation12.BackfaceRepresentation = 'Follow Frontface'
    DataRepresentation12.SelectionOpacity = 1.0
    DataRepresentation12.SelectionPointLabelShadow = 0
    DataRepresentation12.CubeAxesYGridLines = 0
    DataRepresentation12.Shading = 0
    DataRepresentation12.Diffuse = 1.0
    DataRepresentation12.Origin = [0.0, 0.0, 0.0]
    DataRepresentation12.CubeAxesZTitle = 'Z-Axis'
    DataRepresentation12.Specular = 0.10000000000000001
    DataRepresentation12.SelectionVisibility = 1
    DataRepresentation12.InterpolateScalarsBeforeMapping = 1
    DataRepresentation12.CubeAxesZAxisTickVisibility = 1
    DataRepresentation12.SelectionUseOutline = 0
    DataRepresentation12.SelectionCellFieldDataArrayIndex = 0
    DataRepresentation12.CubeAxesVisibility = 0
    DataRepresentation12.Scale = [1.0, 1.0, 1.0]
    DataRepresentation12.SelectionCellLabelJustification = 'Center'
    DataRepresentation12.DiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation12.SelectionCellLabelOpacity = 1.0
    DataRepresentation12.CubeAxesInertia = 1
    DataRepresentation12.Opacity = 1.0
    DataRepresentation12.LineWidth = 1.0
    DataRepresentation12.SelectionPointSize = 5.0
    DataRepresentation12.Material = ''
    DataRepresentation12.Visibility = 0
    DataRepresentation12.SelectionCellLabelFontSize = 24
    DataRepresentation12.CubeAxesCornerOffset = 0.0
    DataRepresentation12.SelectionPointLabelJustification = 'Center'
    DataRepresentation12.Ambient = 0.0
    DataRepresentation12.SelectedMapperIndex = 'Projected tetra'
    DataRepresentation12.CubeAxesTickLocation = 'Inside'
    DataRepresentation12.BackfaceDiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation12.CubeAxesYAxisVisibility = 1
    DataRepresentation12.SelectionPointLabelFontFamily = 'Arial'
    DataRepresentation12.CubeAxesFlyMode = 'Closest Triad'
    DataRepresentation12.CubeAxesYTitle = 'Y-Axis'
    DataRepresentation12.SelectionPointFieldDataArrayIndex = 0
    DataRepresentation12.ColorAttributeType = 'POINT_DATA'
    DataRepresentation12.SpecularPower = 100.0
    DataRepresentation12.Texture = []
    DataRepresentation12.SelectionCellLabelShadow = 0
    DataRepresentation12.AmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation12.MapScalars = 1
    DataRepresentation12.PointSize = 2.0
    DataRepresentation12.StaticMode = 0
    DataRepresentation12.SelectionCellLabelColor = [0.0, 1.0, 0.0]
    DataRepresentation12.EdgeColor = [0.0, 0.0, 0.50000762951094835]
    DataRepresentation12.CubeAxesXAxisTickVisibility = 1
    DataRepresentation12.SelectionCellLabelVisibility = 0
    DataRepresentation12.NonlinearSubdivisionLevel = 1
    DataRepresentation12.CubeAxesColor = [1.0, 1.0, 1.0]
    DataRepresentation12.Representation = 'Surface'
    DataRepresentation12.CustomBounds = [0.0, 1.0, 0.0, 1.0, 0.0, 1.0]
    DataRepresentation12.CubeAxesXAxisMinorTickVisibility = 1
    DataRepresentation12.Orientation = [0.0, 0.0, 0.0]
    DataRepresentation12.UseLookupTableScalarRange = 0
    DataRepresentation12.CubeAxesXTitle = 'X-Axis'
    DataRepresentation12.ScalarOpacityUnitDistance = 0.41991230410220659
    DataRepresentation12.BackfaceOpacity = 1.0
    DataRepresentation12.SelectionCellFieldDataArrayName = ''
    DataRepresentation12.SelectionColor = [1.0, 0.0, 1.0]
    DataRepresentation12.SelectionPointLabelVisibility = 0
    DataRepresentation12.SelectionPointLabelFontSize = 18
    DataRepresentation12.BackfaceAmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation12.ScalarOpacityFunction = a1_vel_mag_PiecewiseFunction
    DataRepresentation12.SelectionLineWidth = 2.0
    DataRepresentation12.CubeAxesZAxisMinorTickVisibility = 1
    DataRepresentation12.CubeAxesXAxisVisibility = 1
    DataRepresentation12.Interpolation = 'Gouraud'
    DataRepresentation12.SelectionCellLabelFontFamily = 'Arial'
    DataRepresentation12.SelectionCellLabelItalic = 0
    DataRepresentation12.CubeAxesYAxisMinorTickVisibility = 1
    DataRepresentation12.CubeAxesZGridLines = 0
    DataRepresentation12.ExtractedBlockIndex = 0
    DataRepresentation12.SelectionPointLabelOpacity = 1.0
    DataRepresentation12.Pickable = 1
    DataRepresentation12.CustomBoundsActive = [0, 0, 0]
    DataRepresentation12.SelectionRepresentation = 'Wireframe'
    DataRepresentation12.ClippingPlanes = []
    DataRepresentation12.SelectionPointLabelBold = 0
    DataRepresentation12.NumberOfSubPieces = 1
    DataRepresentation12.ColorArrayName = 'vel_mag'
    DataRepresentation12.SelectionPointLabelItalic = 0
    DataRepresentation12.SpecularColor = [1.0, 1.0, 1.0]
    DataRepresentation12.LookupTable = a1_vel_mag_PVLookupTable
    DataRepresentation12.SelectionCellLabelBold = 0
    
    SetActiveSource(StreamTracer1)
    DataRepresentation16 = Show()
    DataRepresentation16.CubeAxesZAxisVisibility = 1
    DataRepresentation16.SelectionPointLabelColor = [1.0, 1.0, 1.0]
    DataRepresentation16.SelectionPointFieldDataArrayName = ''
    DataRepresentation16.SuppressLOD = 0
    DataRepresentation16.CubeAxesXGridLines = 0
    DataRepresentation16.CubeAxesYAxisTickVisibility = 1
    DataRepresentation16.Position = [0.0, 0.0, 0.0]
    DataRepresentation16.BackfaceRepresentation = 'Follow Frontface'
    DataRepresentation16.SelectionOpacity = 1.0
    DataRepresentation16.SelectionPointLabelShadow = 0
    DataRepresentation16.CubeAxesYGridLines = 0
    DataRepresentation16.Shading = 0
    DataRepresentation16.Diffuse = 1.0
    DataRepresentation16.Origin = [0.0, 0.0, 0.0]
    DataRepresentation16.CubeAxesZTitle = ''
    DataRepresentation16.Specular = 0.10000000000000001
    DataRepresentation16.SelectionVisibility = 1
    DataRepresentation16.InterpolateScalarsBeforeMapping = 1
    DataRepresentation16.CubeAxesZAxisTickVisibility = 1
    DataRepresentation16.SelectionUseOutline = 0
    DataRepresentation16.SelectionCellFieldDataArrayIndex = 0
    DataRepresentation16.CubeAxesVisibility = 1
    DataRepresentation16.Scale = [1.0, 1.0, 1.0]
    DataRepresentation16.SelectionCellLabelJustification = 'Center'
    DataRepresentation16.DiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation16.SelectionCellLabelOpacity = 1.0
    DataRepresentation16.Opacity = 1.0
    DataRepresentation16.LineWidth = 1.0
    DataRepresentation16.SelectionPointSize = 5.0
    DataRepresentation16.Material = ''
    DataRepresentation16.Visibility = 1
    DataRepresentation16.SelectionCellLabelFontSize = 24
    DataRepresentation16.CubeAxesCornerOffset = 0.0
    DataRepresentation16.SelectionPointLabelJustification = 'Center'
    DataRepresentation16.Ambient = 0.0
    DataRepresentation16.CubeAxesTickLocation = 'Inside'
    DataRepresentation16.BackfaceDiffuseColor = [1.0, 1.0, 1.0]
    DataRepresentation16.CubeAxesYAxisVisibility = 1
    DataRepresentation16.SelectionPointLabelFontFamily = 'Arial'
    DataRepresentation16.CubeAxesFlyMode = 'Outer Edges'
    DataRepresentation16.CubeAxesYTitle = ''
    DataRepresentation16.SelectionPointFieldDataArrayIndex = 0
    DataRepresentation16.ColorAttributeType = 'POINT_DATA'
    DataRepresentation16.SpecularPower = 100.0
    DataRepresentation16.Texture = []
    DataRepresentation16.SelectionCellLabelShadow = 0
    DataRepresentation16.AmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation16.MapScalars = 1
    DataRepresentation16.PointSize = 2.0
    DataRepresentation16.StaticMode = 0
    DataRepresentation16.SelectionCellLabelColor = [0.0, 1.0, 0.0]
    DataRepresentation16.EdgeColor = [0.0, 0.0, 0.50000762951094835]
    DataRepresentation16.CubeAxesXAxisTickVisibility = 1
    DataRepresentation16.SelectionCellLabelVisibility = 0
    DataRepresentation16.NonlinearSubdivisionLevel = 1
    DataRepresentation16.CubeAxesColor = [1.0, 1.0, 1.0]
    DataRepresentation16.Representation = 'Surface'
    DataRepresentation16.CustomBounds = [-6.0343999999999998, -0.4723, -1.7038, 6.1294000000000004, -9.8377999999999997, 13.2585]
    DataRepresentation16.CubeAxesXAxisMinorTickVisibility = 0
    DataRepresentation16.Orientation = [0.0, 0.0, 0.0]
    DataRepresentation16.UseLookupTableScalarRange = 0
    DataRepresentation16.CubeAxesXTitle = ''
    DataRepresentation16.CubeAxesInertia = 1
    DataRepresentation16.BackfaceOpacity = 1.0
    DataRepresentation16.SelectionCellFieldDataArrayName = ''
    DataRepresentation16.SelectionColor = [1.0, 0.0, 1.0]
    DataRepresentation16.SelectionPointLabelVisibility = 0
    DataRepresentation16.SelectionPointLabelFontSize = 18
    DataRepresentation16.BackfaceAmbientColor = [1.0, 1.0, 1.0]
    DataRepresentation16.SelectionLineWidth = 2.0
    DataRepresentation16.CubeAxesZAxisMinorTickVisibility = 0
    DataRepresentation16.CubeAxesXAxisVisibility = 1
    DataRepresentation16.Interpolation = 'Gouraud'
    DataRepresentation16.SelectionCellLabelFontFamily = 'Arial'
    DataRepresentation16.SelectionCellLabelItalic = 0
    DataRepresentation16.CubeAxesYAxisMinorTickVisibility = 0
    DataRepresentation16.CubeAxesZGridLines = 1
    DataRepresentation16.SelectionPointLabelOpacity = 1.0
    DataRepresentation16.Pickable = 1
    DataRepresentation16.CustomBoundsActive = [0, 0, 0]
    DataRepresentation16.SelectionRepresentation = 'Wireframe'
    DataRepresentation16.ClippingPlanes = []
    DataRepresentation16.SelectionPointLabelBold = 0
    DataRepresentation16.NumberOfSubPieces = 1
    DataRepresentation16.ColorArrayName = 'velocity'
    DataRepresentation16.SelectionPointLabelItalic = 0
    DataRepresentation16.SpecularColor = [1.0, 1.0, 1.0]
    DataRepresentation16.LookupTable = a3_velocity_PVLookupTable
    DataRepresentation16.SelectionCellLabelBold = 0
    


    for writer in cp_writers:
        if timestep % writer.cpFrequency == 0:
            writer.FileName = writer.cpFileName.replace("%t", str(timestep))
            writer.UpdatePipeline()

    if timestep % 1 == 0:
        renderviews = servermanager.GetRenderViews()
        imagefilename = "image_%t.png"
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
