import sys

from libvtkCoProcessorPython import *
from libvtkGraphicsPython import *
from libvtkParallelPython import *
from libvtkIOPython import *
from libvtkFilteringPython import *
from libvtkCommonPython import vtkTransform


if len(sys.argv) != 3:
    print "Usage: %s <cp-script> <vtu file>" % sys.argv[0]
    sys.exit(1)


cpscript = sys.argv[1]
vtu_file = sys.argv[2]

procId = 0 #initialized later

def myprint(message):
    if False:
        print "driver (%d): %s" % (procId, message)


myprint("starting coprocessor")
processor = vtkCPProcessor()
processor.Initialize()
pipeline = vtkCPPythonScriptPipeline()

# mpi is not initialized until after creating the vtkCPPythonScriptPipeline().
globalController =  vtkMultiProcessController.GetGlobalController()
procId = globalController.GetLocalProcessId()


# read the coprocessing python file
myprint("loading pipeline python file: " + cpscript)
success = pipeline.Initialize(cpscript)
if not success:
    print "aborting"
    sys.exit(1)
processor.AddPipeline(pipeline)

# read and redistribute the data

d3Input = None

if procId == 0:
    myprint("reading data file: " + vtu_file)
    reader = vtkXMLUnstructuredGridReader()
    reader.SetFileName(vtu_file)
    reader.Update()
    d3Input = reader.GetOutput()
else:
    d3Input = vtkUnstructuredGrid()

d3 = vtkDistributedDataFilter()
d3.SetController(globalController)
d3.SetInput(d3Input)
d3.Update()

transformFilter = vtkTransformFilter()
transform = vtkTransform()
transform.PostMultiply()

transformFilter.SetInputConnection(d3.GetOutputPort())
transformFilter.SetTransform(transform)
transformFilter.Update()

center = [-1.34365, -0.899783, 13.0487]
axis = [-3.95467, 6.051383, -14.898]

startTime = 0.0
stepSize = 0.1
timestep = 1
lastTime = 0.0
steps_per_loop = 15

def nextStep():

    global timestep, lastTime
    newTime = startTime + (stepSize * timestep)
    timeStr = "time(" + str(timestep) + ", " + str(newTime) + ")"

    dataDesc = vtkCPDataDescription()
    dataDesc.AddInput("input")
    dataDesc.SetTimeData(newTime, timestep)
    timestep += 1
    lastTime = newTime

    myprint("call RequestDataDescription, " + timeStr)
    do_coprocessing = processor.RequestDataDescription(dataDesc)

    if do_coprocessing:

        # rotate the data
        angle = 360.0 * (float(timestep) / steps_per_loop)
        transform.Identity()
        transform.Translate(-center[0], -center[1], -center[2])
        transform.RotateWXYZ(angle, axis[0], axis[1], axis[2])
        transform.Translate(center[0], center[1], center[2])
        transformFilter.Update()

        myprint("calling CoProcess, " + timeStr)
        dataDesc.GetInputDescriptionByName("input").SetGrid(transformFilter.GetOutput())
        processor.CoProcess(dataDesc)



def onNextStep():
    globalController.Barrier()
    nextStep()
    setLastTime(lastTime)

def onStepClicked():
    stepButton.setEnabled(False)
    playButton.setEnabled(False)
    pauseButton.setEnabled(False)
    setStatusText("stepping", "green")
    onNextStep()
    onPauseClicked()


def onPlayNext():
    if not playButton.isEnabled():
        setStatusText("playing", "green")
        onNextStep()

    if not playButton.isEnabled():
        waitTime = waitTimeSpin.value()
        setStatusText("waiting %.02f seconds" % waitTime, "black")
        QtCore.QTimer.singleShot(waitTime*1000, onPlayNext)


def onPlayClicked():
    stepButton.setEnabled(False)
    playButton.setEnabled(False)
    pauseButton.setEnabled(True)
    onPlayNext()


def onPauseClicked():
    stepButton.setEnabled(True)
    playButton.setEnabled(True)
    pauseButton.setEnabled(False)
    setStatusText("paused", "black")


def setStatusText(text, color):
    statusLabel.setText("<b>status:</b> <font color=%s>%s</font>" % (color, text))
    app.processEvents()
    app.processEvents()

def setLastTime(value):
    lastTimeLabel.setText("<b>last time:</b> %.03f" % value)

if procId != 0:

    while True:
        globalController.Barrier()
        nextStep()


from PyQt4 import QtCore, QtGui

app = QtGui.QApplication(sys.argv)
window = QtGui.QWidget()
layout = QtGui.QVBoxLayout(window)

statusLabel = QtGui.QLabel("")
lastTimeLabel = QtGui.QLabel("")
setLastTime(lastTime)
waitTimeLabel = QtGui.QLabel("delay between timesteps:")
waitTimeSpin = QtGui.QDoubleSpinBox()
waitTimeSpin.setSuffix(" seconds")
waitTimeSpin.setValue(5)

stepButton = QtGui.QPushButton("step")
playButton = QtGui.QPushButton("play")
pauseButton = QtGui.QPushButton("pause")
onPauseClicked()

stepButton.connect(stepButton, QtCore.SIGNAL('clicked()'), onStepClicked)
playButton.connect(playButton, QtCore.SIGNAL('clicked()'), onPlayClicked)
pauseButton.connect(pauseButton, QtCore.SIGNAL('clicked()'), onPauseClicked)


hlayout = QtGui.QHBoxLayout()
hlayout.addWidget(playButton)
hlayout.addWidget(pauseButton)
layout.addLayout(hlayout)

layout.addWidget(stepButton)

hlayout = QtGui.QHBoxLayout()
hlayout.addWidget(waitTimeLabel)
hlayout.addWidget(waitTimeSpin)
layout.addLayout(hlayout)
layout.addWidget(QtGui.QLabel())
layout.addStretch()


layout.addWidget(statusLabel)
layout.addWidget(lastTimeLabel)


window.setWindowTitle("simulator")
window.show()

app.exec_()


myprint("finalizing")
processor.Finalize()
