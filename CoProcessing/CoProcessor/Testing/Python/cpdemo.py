# This script was generated using the coprocessor export plugin.
# The code in this file has not been modified from the generated version
# except to add a call to sleep() in RequestDataDescription which was added
# to throttle the demonstration driver.

from paraview.simple import *
from CPUtils import *

host = "localhost"
port = 22222

set_use_network(True)
set_do_reduce_data(True)
set_do_writing(False)
set_use_psets(False)
set_log_messages(False)
set_procs_per_partition(32)
set_final_partition_size(1)

cp_writers = []

def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"
    timestep = datadescription.GetTimeStep()

    # This call to sleep is added to throttle the solver speed for demo purposes.
    import time
    time.sleep(1.0)

    #########################
    #begin
    set_timestep(timestep)
    if get_use_network():
        open_connection(host, port) # no-op if connection is already established
        receive_messages()

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

    Sphere1 = CreateProducer( datadescription, "input" )
    
    Reflect1 = Reflect(Plane='Y Max')
    
    Clip1 = Clip(ClipType="Plane")
    Clip1.ClipType.Normal = [0.0, 0.0, -1.0]
    Clip1.ClipType.Origin = [0.0, 0.0, 0.0]
    
    ProcessIdScalars1 = ProcessIdScalars()
    
    ExtractSurface1 = ExtractSurface()
    
    ParallelPolyDataWriter1 = CreateWriter( XMLPPolyDataWriter, "filename_%t.pvtp", 1 )
    



    ################################
    #begin
    set_timestep(timestep)
    load_proxy_states()

    # Update pipelines for active sinks
    sinks = [writer.Input for writer in cp_writers]

    for sink_tag, sink in enumerate(sinks):
        if get_sink_status(sink_tag):
            print "  updating:", sink.GetXMLLabel()
            sink.UpdatePipeline()
    print "-------------------------------"

    # Get data objects
    sinkDataObjects = [sink.GetClientSideObject().GetOutputDataObject(0) for sink in sinks]

    if get_use_network():
        if get_connection_is_active():

            reduced_data_objects = list()
            for sink_tag, sink_data_object in enumerate(sinkDataObjects):
                if get_sink_status(sink_tag):
                    reduced_data = reduce_data(sink_data_object)
                    reduced_data_objects.append((sink_tag, reduced_data))



            send_extracts(reduced_data_objects, datadescription.GetTimeStep(),
                                           datadescription.GetTime())
            send_state(sinks)
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
