import vtk
import time
import os
import math
import sys

try:
    outDir = sys.argv[1]
except IndexError:
    outDir = os.path.split(sys.argv[0])[0]
    if not outDir: outDir = "."

if not os.path.exists(outDir):
    print "directory '%s' does not exist." % outDir
    sys.exit(1)

def removeFiles(dirname, endsWithString):
    for filename in os.listdir(dirname):
        if filename.endswith(endsWithString):
            os.remove(dirname + "/" + filename)

removeFiles(outDir, ".vtp")

s = vtk.vtkSphereSource()
writer = vtk.vtkXMLPolyDataWriter()
writer.SetInput(s.GetOutput())

fileName = outDir + "/out_%d.vtp"
index = 0

while True:

    radius = math.fabs(math.sin(0.1 * index))
    s.SetRadius(1.0 + radius)

    writer.SetFileName(fileName % index)
    writer.Update()

    index += 1
    time.sleep(1.0)

