
import paraview.simple as simple
import libvtkParallelPython as vtkparallel
import libvtkPVFiltersPython as pvfilters
servermanager = simple.servermanager
vtk = servermanager.vtk


import PartitionUtils

##########################################
# Timestep
_timestep = 0

def get_timestep():
    return _timestep

def set_timestep(t):
    global _timestep
    _timestep = t

##########################################
# Messages and timing
_messages = []
_log_messages = False
gettime = vtk.vtkTimerLog.GetUniversalTime

def set_log_messages(value):
  global _log_messages
  _log_messages = value

def print_messages():
    global _messages
    for m in _messages:
        print m
    _messages = []

def _push_message(m):
    if _log_messages:  _messages.append(m)
    else: print m

class mytimer(object):

    def __init__(self): self.start()
    def start(self): self.time = gettime()
    def event(self, message):
        newtime = gettime()
        message = "cp_event (%d): %s %d %f" % (pid, message, _timestep, newtime - self.time)
        _push_message(message)
        self.time = newtime


def myprint(message):
    message = "cp_msg (%d): %s" % (pid, message)
    _push_message(message)

###########################################
# Process id and number of procs

_pid = None
_number_of_procs = None

def get_pid():
    global _pid
    if _pid is None:
        _pid = servermanager.vtkProcessModule.GetProcessModule().GetPartitionId()
    return _pid

def get_number_of_procs():
    global _number_of_procs
    if _number_of_procs is None:
        _number_of_procs = servermanager.vtkProcessModule.GetProcessModule().GetNumberOfLocalPartitions()
    return _number_of_procs


pid = get_pid()
number_of_procs = get_number_of_procs()


################################################
_connection_is_active = False
def get_connection_is_active():
    return _connection_is_active


_procs_per_partition = 32
_final_partition_size = 1

_use_network = False
_do_reduce_data = False
_do_writing = False

_transfer = None
_partitions = None
_subcontrollers = None
_sent_state = 0
_states = dict()
_sink_status = dict()


_use_psets = False
def get_use_psets():
    return _use_psets

def set_use_psets(value):
    global _use_psets
    _use_psets = value

def get_psets():
    #import PSets
    #return PSets.sets[get_number_of_procs()]

    import libvtkCoProcessorPython as vtkcp
    ranks = vtk.vtkIntArray()
    vtkcp.vtkCPPartitionHelper.ComputePSetRanks(ranks)
    ranks = [ranks.GetValue(i) for i in xrange(ranks.GetNumberOfTuples())]
    return _compute_psets(ranks)


def get_partitions():
    return _partitions

def get_subcontrollers():
    return _subcontrollers

def set_use_network(value):
    global _use_network
    _use_network = value

def get_use_network():
    return _use_network

def set_do_reduce_data(value):
    global _do_reduce_data
    _do_reduce_data = value

def get_do_reduce_data():
    return _do_reduce_data

def get_global_controller():
    return servermanager.vtkProcessModule.GetProcessModule().GetController()

def set_do_writing(value):
    global _do_writing
    _do_writing = value

def get_do_writing():
    return _do_writing

def set_procs_per_partition(num):
    global _procs_per_partition
    _procs_per_partition = num

def get_procs_per_partition():
    return _procs_per_partition

def set_final_partition_size(num):
    global _final_partition_size
    _final_partition_size = num

def get_final_partition_size():
    return _final_partition_size

def get_final_partition_group():
    return _partitions[-1][0]

def get_transfer():
    global _transfer
    if not _transfer:
        _transfer = pvfilters.vtkCoProcessorTransfer()
    return _transfer


def create_pset_partitions(psets, final_partition_size):

    partitions = PartitionUtils.partition_id_sets(psets, final_partition_size)
    #if get_pid() == 0: PartitionUtils.pretty_print_partitions(partitions)
    return partitions


def create_partitions(number_of_procs, procs_per_partition, final_partition_size):

    partitions = PartitionUtils.partition(number_of_procs,
                                  procs_per_partition, final_partition_size)
    #if get_pid() == 0: PartitionUtils.pretty_print_partitions(partitions)
    return partitions


def init_partitions():
    global _partitions, _subcontrollers

    # Don't do anything if we have already initialized the subcontrollers
    if _subcontrollers is not None: return

    # Create the partitions
    if get_use_psets():
        _partitions = create_pset_partitions(get_psets(), get_final_partition_size())
    else:
        _partitions = create_partitions(get_number_of_procs(), get_procs_per_partition(), get_final_partition_size())

    # Initialize the subcontrollers
    _subcontrollers = create_controllers(_partitions, get_global_controller())



def open_connection(host, port):
    global _connection_is_active

    # Don't do anything if there is already a connection
    if _connection_is_active: return True

    # This method connects the root cp process to the root pvserver
    # process and returns the total number of pvserver processes
    number_of_pvserver_procs = get_transfer().CoProcessorConnectToServer(host, port)

    if not number_of_pvserver_procs:
        return False

    _connection_is_active = True
    #myprint("number of pvserver procs: %d" % number_of_pvserver_procs)

    set_final_partition_size(number_of_pvserver_procs)
    init_partitions()

    # Create a vtkIntArray that contains the process ids that belong
    # to the final partition group
    final_partition_group = get_final_partition_group()
    #myprint("final partition ids: " + str(final_partition_group))
    final_partition_ids = vtk.vtkIntArray()
    for i in final_partition_group:
        final_partition_ids.InsertNextValue(i)


    # Connect each process in the final partition group to a pvserver process
    #
    # Todo - what if this fails?
    #
    get_transfer().CoProcessorMakeConnections(final_partition_ids)

    # Do a barrier here so that we're all together
    get_global_controller().Barrier()

    return True


def send_extracts(extracts, timestep, time):

    if not get_connection_is_active():
        raise Exception("Cannot send extract because there is no connection.")

    get_transfer().SendExtractsCommand()

    if pid in get_final_partition_group():

        extract_collection = vtk.vtkDataObjectCollection()
        extract_tags = vtk.vtkIntArray()
        for sink_tag, sink_data_object in extracts:

            extract_collection.AddItem(sink_data_object)
            extract_tags.InsertNextValue(sink_tag)

        get_transfer().SendExtracts(extract_collection, extract_tags, timestep, time)


def reduce_all_to_n(data_object, controller, number_of_processes=1):

    if not data_object:
        myprint("-null data object")
        return None
    if not controller:
        raise Exception("null controller")

    number_of_procs = controller.GetNumberOfProcesses()
    if number_of_procs == 1:
        #myprint("reduce_all_to_n no-op- only 1 proc")
        return data_object

    #myprint("---reduce_all_to_n reducing-------")

    data_copy = vtk.vtkPolyData()
    data_copy.ShallowCopy(data_object);

    reduce_filter = pvfilters.vtkAllToNRedistributePolyData()
    reduce_filter.SetInput(data_copy);
    reduce_filter.SetController(controller);
    reduce_filter.SetNumberOfProcesses(number_of_processes);
    reduce_filter.Update()

    output = reduce_filter.GetOutput()

    #myprint("  %d cells" % output.GetNumberOfCells())
    if output.GetNumberOfCells() == 0:
        return None
    else:
        data_copy = vtk.vtkPolyData()
        data_copy.ShallowCopy(reduce_filter.GetOutput())
        return data_copy


def create_controllers(partitions, controller):

    controllers = []
    mpi_undefined = vtkparallel.vtkMPICommunicator.GetMPIUndefined()
    local_id = controller.GetLocalProcessId()

    # Iterate over all partitions except the last one
    for partitioning in partitions[:-1]:

        #myprint("------")
        color = mpi_undefined
        key = 0

        for group_index, id_list in enumerate(partitioning):

            # Don't create communicators for single process partitions
            if len(id_list) == 1:
                continue

            if local_id in id_list:
                color = group_index
                key = id_list.index(local_id)
                break

        #myprint("my color,key is %d,%d" % (color,key))

        subcontroller = controller.PartitionController(color, key)
        if subcontroller: subcontroller.UnRegister(None)

        controllers.append(subcontroller)

    return controllers


def reduce_data_one_shot(data_object, number_of_processes):

    if data_object.GetClassName() != "vtkPolyData":
        raise Exception("Cannot reduce data type:", data_object.GetClassName())

    controller = servermanager.vtkProcessModule.GetProcessModule().GetController()
    data_object = reduce_all_to_n(data_object, controller, number_of_processes)
    return data_object


def reduce_data(data_object):

    if data_object.GetClassName() != "vtkPolyData":
        raise Exception("Cannot reduce data type: " + data_object.GetClassName())

    if _subcontrollers is None:
        raise Exception("Subcontrollers have not been initialized.")

    for subcontroller in _subcontrollers:
        if not subcontroller:
            #myprint("skip")
            continue

        if not data_object:
            raise Exception("Cannot reduce data object, data object is null.")

        #nCells = data_object.GetNumberOfCells()
        #nCellsReduced = 0
        data_object = reduce_all_to_n(data_object, subcontroller)

        #if data_object: nCellsReduced = data_object.GetNumberOfCells()
        #myprint("reduce %d --> %d" % (nCells, nCellsReduced))

    if not data_object:
        data_object = vtk.vtkPolyData()
    return data_object



def load_proxy_states():
    iter = servermanager.vtkSMProxyIterator()
    iter.Begin()
    while not iter.IsAtEnd():
        if "prototypes" in iter.GetGroup():
            iter.Next()
            continue
        proxy = iter.GetProxy()
        iter.Next()
        proxyType = proxy.GetXMLName()
        if proxyType in _states:
            elem = _states[proxyType]
            proxy.LoadState(elem, None)
            proxy.UpdateVTKObjects()


def _extract_proxy_states(messages):

    states_element = messages.FindNestedElementByName("states")
    if not states_element: return

    for i in xrange(states_element.GetNumberOfNestedElements()):
        proxyElem = states_element.GetNestedElement(i)
        proxyType = proxyElem.GetAttribute("type")
        _states[proxyType] = proxyElem


def get_sink_status(sink_tag):
    if sink_tag in _sink_status:
        return _sink_status[sink_tag]
    else: return True

def _extract_sink_status(messages):

    status_element = messages.FindNestedElementByName("sink_status")
    if not status_element:
        return

    status_string = status_element.GetAttribute("status")
    status_list = status_string.split()

    for sink_tag, sink_status in zip(status_list[::2], status_list[1::2]):
        _sink_status[int(sink_tag)] = bool(int(sink_status))

def _extract_disconnect_notice(messages):

    disconnect_element = messages.FindNestedElementByName("disconnect")
    if disconnect_element:
        get_transfer().Disconnect()
        global _connection_is_active, _sent_state
        _connection_is_active = False
        _sent_state = False




def receive_messages():

    if get_connection_is_active():
        messages = get_transfer().ReceiveState()

        if messages:
            if messages.GetName() != "msg":
                print "Expect XML element to be 'msg'"
                return

            _extract_proxy_states(messages)
            _extract_sink_status(messages)
            _extract_disconnect_notice(messages)

            messages.UnRegister(None)



def send_state(sinks):

    # Send state information to the server
    global _sent_state
    if not _sent_state and get_connection_is_active():
        state_element = servermanager.ProxyManager().SaveState()

        helper = servermanager.vtkSMCPStateLoader()

        for sink_tag, sink in enumerate(sinks):
            sink_id = sink.GetSelfIDAsInt()
            sink_element = helper.LocateProxyElement(state_element, sink_id)
            if sink_element: sink_element.SetAttribute("sink_tag", str(sink_tag))

        get_transfer().SendState(state_element)
        state_element.UnRegister(None)
        _sent_state = True


def _compute_psets(ranks):
    """Compute psets given a flat list of rank_in_pset, pset_id pairs.
    For example, ranks[0] and ranks[1] are the rank_in_pset and pset_id for
    global rank 0.  Returns a list of lists, where each sublist contains all
    the global ranks in a pset, and each sublist is sorted by rank in pset."""

    if  (len(ranks) % 2) != 0:
        raise Exception("PSet rank list size is not a multiple of two.")

    psets = dict()
    for i in xrange(len(ranks)/2):
        global_rank = i
        rank_in_pset = ranks[i*2]
        pset_id = ranks[i*2+1]
        if not pset_id in psets: psets[pset_id] = list()
        psets[pset_id].append((rank_in_pset, global_rank))

    pset_list = list()
    for pset_id in xrange(len(psets)):
      pset_ranks = psets[pset_id]
      pset_ranks = sorted(pset_ranks, key=lambda rank_pair: rank_pair[0])
      pset_list.append([rank_pair[1] for rank_pair in pset_ranks])

    return pset_list
