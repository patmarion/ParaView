
from paraview.simple import *
from CPUtils import *
import ugrid_to_pdata

host = "localhost"
#host = "172.17.9.86"
port = 22222

set_use_network(True)
set_do_reduce_data(False)
set_do_writing(False)
set_use_psets(False)
set_log_messages(True)
set_procs_per_partition(64)
set_final_partition_size(1)

cp_writers = []

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    timestep = datadescription.GetTimeStep()

    #########################
    #begin

    get_global_controller().Barrier()
    
    set_timestep(timestep)
    t = mytimer()
    if get_use_network():
        open_connection(host, port) # no-op if connection is already established
        t.event("open_connection")
        receive_messages()
        t.event("receive_messages")

    elif get_do_reduce_data():
        init_partitions()
    #end
    ##########################


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

    t = mytimer()

    input_mesh = CreateProducer( datadescription, "input" )

    if timestep == 1:
        nCells = input_mesh.GetClientSideObject().GetOutputDataObject(0).GetNumberOfCells()
        myprint("number of input cells: %d" % nCells)
        myprint("input data size: %f" % (input_mesh.GetClientSideObject().GetOutputDataObject(0).GetActualMemorySize() / 1024.0))

    #SetActiveSource(input_mesh)
    #ProcessIds1 = ProcessIdScalars()
    #Calculator1 = Calculator( guiName="calculator_1", Function='ProcessId', ReplacementValue=0.0, ResultArrayName='cp-process-ids', ReplaceInvalidResults=1, AttributeMode='point_data', CoordinateResults=0 )


    #SetActiveSource(input_mesh)
    #Writer1 = CreateWriter( XMLUnstructuredGridWriter, "cpout_fullmesh_%p_%t.vtu", 1 )

    #SetActiveSource(Calculator1)
    #Writer2 = CreateWriter( XMLUnstructuredGridWriter, "cpout_procids_%p_%t.vtu", 1 )

    # Create polydata for input mesh
    """
    pdata = paraview.vtk.vtkPolyData()
    ugrid_to_pdata.convert(input_mesh.GetClientSideObject().GetOutputDataObject(0), pdata)
    pdataProducer = CreateProducerForData(pdata)
    SetActiveSource(pdataProducer)
    ParallelPolyDataWriter4 = CreateWriter( XMLPolyDataWriter, "cpout_fullmesh_%p_%t.vtp", 1 )
    """

    #ExtractSurface()
    #ParallelPolyDataWriter1 = CreateWriter( XMLPPolyDataWriter, "cpout_full_%t.pvtp", 1 )


    ################################
    #begin

    t.event("create_pipeline")
    set_timestep(timestep)
    load_proxy_states()
    t.event("load_states")

    # Update pipelines for active sinks
    sinks = [writer.Input for writer in cp_writers]

    pipelineTimer = mytimer()
    for sink_tag, sink in enumerate(sinks):
        if get_sink_status(sink_tag):
            #print "  updating:", sink.GetXMLLabel()
            t.start()
            sink.UpdatePipeline()
            t.event("update_pipeline_%s" % sink.GetXMLName())
    pipelineTimer.event("update_pipeline_all")
    #print "-------------------------------"

    # Get data objects
    sinkDataObjects = [sink.GetClientSideObject().GetOutputDataObject(0) for sink in sinks]

    if get_use_network():
        if get_connection_is_active():

            get_global_controller().Barrier()

            reduced_data_objects = list()
            extractCellCount = 0
            reduceTimer = mytimer()
            for sink_tag, sink_data_object in enumerate(sinkDataObjects):
                if get_sink_status(sink_tag):


                    if get_do_reduce_data():
                        t.start()
                        reduced_data = reduce_data(sink_data_object)
                        t.event("reduce_data_%s" % sinks[sink_tag].GetXMLName())
                    else:
                        reduced_data = sink_data_object

                    reduced_data_objects.append((sink_tag, reduced_data))

                    reduceDataCells = 0
                    if reduced_data: reduceDataCells = reduced_data.GetNumberOfCells()

                    extractCellCount += reduced_data.GetNumberOfCells()
                    myprint("number of %s cells: %d" % (sinks[sink_tag].GetXMLName(),
                                                        reduceDataCells))

            reduceTimer.event("reduce_data_all")

            myprint("number of all_extracts cells: %d" % extractCellCount)

            get_global_controller().Barrier()

            t.start()
            send_extracts(reduced_data_objects, datadescription.GetTimeStep(),
                                           datadescription.GetTime())
            t.event("send_extracts")

            get_global_controller().Barrier()

            t.start()
            send_state(sinks)
            t.event("send_state")

    elif get_do_reduce_data():
        sinkDataObjects = [reduce_data(d) for d in sinkDataObjects]

    if not get_do_writing():
        cp_writers = []
    #end
    ################################

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
    t.start()
    tobedeleted = GetProxiesToDelete()
    while len(tobedeleted) > 0:
        Delete(tobedeleted[0])
        tobedeleted = GetProxiesToDelete()
    t.event("clean_up")

    print_messages()

    #import time
    #time.sleep(3)


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

def CreateProducerForData(dataObject):
    producer = TrivialProducer()
    producer.GetClientSideObject().SetOutput(dataObject)
    producer.UpdatePipeline()
    return producer


def CreateProducer(datadescription, gridname):
    "Creates a producer proxy for the grid"
    if not datadescription.GetInputDescriptionByName(gridname):
        raise RuntimeError, "Simulation input name '%s' does not exist" % gridname
    grid = datadescription.GetInputDescriptionByName(gridname).GetGrid()
    return CreateProducerForData(grid)



def CreateWriter(proxy_ctor, filename, freq):
    global cp_writers
    writer = proxy_ctor()
    writer.FileName = filename
    writer.add_attribute("cpFrequency", freq)
    writer.add_attribute("cpFileName", filename)
    cp_writers.append(writer)
    return writer
