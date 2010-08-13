import sys
import time
import os
import shutil

sys.path.append("/source/paraview/build/bin")
sys.path.append("/source/paraview/build/VTK/Wrapping/Python")
from vtk import *
from libvtkPVFiltersPython import *

LIVE_DATA_OUTPUT_DIR = "/source/paraview/build/live-output"
base = "/home/pat/Desktop/cow-time/cow_%02d.vtp"

def getTimeSteps(algorithm, port=0):
    executive = algorithm.GetExecutive()
    outInfo = executive.GetOutputInformation(port)
    TIME_STEPS = vtkStreamingDemandDrivenPipeline.TIME_STEPS()
    return [TIME_STEPS.Get(outInfo, i) for i in xrange(TIME_STEPS.Length(outInfo))]

def printTimeSteps(algorithm, port=0):
    print getTimeSteps(algorithm, port)

def setUpdateTime(algorithm, time, port=0):
    algorithm.GetExecutive().SetUpdateTimeStep(0, time)

def getMaxTime(algorithm):
    timesteps = getTimeSteps(algorithm)
    if not timesteps: return 0
    return timesteps[-1]

vtkAlgorithm.SetDefaultExecutivePrototype(vtkCompositeDataPipeline())



reader = vtkDebugSource()
sink = reader



#debugFilter = vtkDebugFilter()
#debugFilter.AddInputConnection(sink.GetOutputPort())
#sink = debugFilter


executive = sink.GetExecutive()
#executive.SetUpdateTimeStep(0, 6.0)

mapper = vtkPolyDataMapper()
actor = vtkActor()
actor.SetMapper(mapper)
mapper.SetInputConnection(sink.GetOutputPort())


ren = vtkRenderer()
ren.AddActor(actor)

renWin = vtkRenderWindow()
renWin.AddRenderer(ren)

iren = vtkRenderWindowInteractor()
iren.SetInteractorStyle(vtkInteractorStyleTrackballCamera())
renWin.SetInteractor(iren)
renWin.Render()

printTimeSteps(sink)

#iren.Start()

def onSliderChanged(value):
    sliderRange = slider.maximum() - slider.minimum()
    maxTime = getMaxTime(sink)
    time = maxTime * (value / float(sliderRange))
    setUpdateTime(sink, time)
    mapper.Modified()
    ren.ResetCameraClippingRange()
    renWin.Render()

from PyQt4 import QtCore, QtGui

app = QtGui.QApplication(sys.argv)
slider = QtGui.QSlider(QtCore.Qt.Horizontal)
#slider.setRange(0, 9)
slider.connect(slider, QtCore.SIGNAL('valueChanged(int)'), onSliderChanged)
slider.show()

app.exec_()



