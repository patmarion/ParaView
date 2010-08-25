from paraview.simple import *
import time
import sys

try: port = int(sys.argv[1])
except: port = 11111

Connect("localhost", port)
#ReverseConnect(50017)

print "connected."

pm = servermanager.vtkProcessModule.GetProcessModule()
l = LiveDataSource()
l.InvokeCommand("Listen")
print "started live data source."


print "sleeping..."

while True:

    #l.InvokeCommand("Poll")
    #l.UpdatePipelineInformation()
    #print l.TimestepValues
    time.sleep(2.0)

