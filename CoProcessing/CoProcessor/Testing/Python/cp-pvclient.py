from paraview.simple import *
import time
import sys

try: port = int(sys.argv[1])
except: port = 11111

#Connect("localhost", port)
ReverseConnect(50017)

pm = servermanager.vtkProcessModule.GetProcessModule()

l = LiveDataSource()
l.InvokeCommand("Listen")

while True:
    l.InvokeCommand("Poll")

    #l.UpdatePipelineInformation()
    #print l.TimestepValues
    time.sleep(1.0)

