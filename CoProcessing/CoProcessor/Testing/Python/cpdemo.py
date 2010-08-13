
from paraview.simple import *
from CPUtils import *


host = "172.31.23.200"
host = "localhost"
port = 22222

set_procs_per_partition(8)
set_final_partition_size(1)

set_use_network(True)
set_do_reduce_data(True)
set_do_writing(False)
set_use_psets(False)
set_log_messages(False)


cp_writers = []


def RequestDataDescription(datadescription):
    "Callback to populate the request for current timestep"

    timestep = datadescription.GetTimeStep()
    set_timestep(timestep)

    if get_use_network():
        open_connection(host, port)
    elif get_do_reduce_data():
        init_partitions()

    if (timestep % 1 == 0) :
        datadescription.GetInputDescriptionByName('input').AllFieldsOn()
        datadescription.GetInputDescriptionByName('input').GenerateMeshOn()


def DoCoProcessing(datadescription):
    "Callback to do co-processing for current timestep"

    global cp_writers
    cp_writers = []

    timestep = datadescription.GetTimeStep()
    set_timestep(timestep)

    timer = mytimer()


    # Build pipeline
    src = CreateProducer( datadescription, "input" )
    reflect = Reflect(Plane='Y Max')
    clip = Clip()
    idScalars = ProcessIdScalars()
    surface = ExtractSurface()


    # Receive state information from the server
    if get_connection_is_active():
        receive_state()


    surface.UpdatePipeline()


    extractData = GetActiveSource().GetClientSideObject().GetOutputDataObject(0)



    if get_use_network():
        if get_connection_is_active():
            extractData = reduce_data(extractData)
            send_extract(extractData)
            send_state()
    elif get_do_reduce_data():
        extractData = reduce_data(extractData)


    nCells = extractData.GetNumberOfCells()
    if timestep == 1:
        myprint("number of reduce cells: %d" % nCells)


    if get_do_writing():
        producer = CreateProducerForData(extractData)
        CreateWriter( XMLPolyDataWriter, "slices_%p_%t.vtp", 1 )
        for writer in cp_writers:
            if timestep % writer.cpFrequency == 0 and nCells > 0:
                writer.FileName = writer.cpFileName.replace("%t", str(timestep)).replace("%p", str(get_pid()))
                writer.UpdatePipeline()

    # Clean up
    for s in GetSources().values():
        Delete(s)

    import time
    time.sleep(2.0)


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

