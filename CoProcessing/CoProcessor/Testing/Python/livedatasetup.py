import os
from paraview import benchmark

# Determine a log filename
log_filename = os.getenv("PVSERVER_LOGFILE")
if not log_filename:
    log_filename = "pvserver_log.txt"


# Open the logfile for writing
logFile = open(log_filename, 'w')


AnnotateTime()
Show()

v = GetRenderView()
s = LiveDataSource()
r = Show()

Render()

benchmark.maximize_logs()
benchmark.clear_logs()

_first_call = True
def onLiveDataTimeStep():

    timesteps = s.TimestepValues
    if not timesteps:
        return

    v.ViewTime = timesteps[-1]

    global _first_call
    if _first_call:
        _first_call = False
        setupRenderView()

    Render()

    captureLogs()


def setupRenderView():

    lut = GetLookupTableForArray("velocity", 3)
    lut.RGBPoints = [0.0, 0.23, 0.29, 0.75, 112.25, 0.7, 0.01, 0.15]
    lut.VectorMode = 'Magnitude'
    lut.ColorSpace = 'Diverging'

    r.Representation = 'Surface'
    r.LookupTable = lut
    r.ColorArrayName = 'velocity'
    r.ColorAttributeType = 'POINT_DATA'

    v.CameraViewUp = [0.5, 0.3, 0.8]
    v.CameraPosition = [-13.3, -37.6, 23.8]
    v.CameraFocalPoint = [-5.8, 2.5, 3.8]
    v.CameraClippingRange = [25.4, 68.8]


def writeLog(msg):
    global logFile
    logFile.write(msg + "\n")


def flushLog():
    global logFile
    logFile.flush()


def captureLogs():

    current_timestep = v.ViewTime
    writeLog("---------------------------------------------------------------")
    writeLog("BEGIN PRINT LOG ")
    writeLog("ViewTime %f" % v.ViewTime)
    writeLog("ElapsedTime %f" % getElapsedTime())
    logs = benchmark.get_logs()
    for log in logs:
        writeLog("LogComponent: " + log.componentString())
        writeLog("LogProcess: " + str(log.rank))
        for line in log.lines: writeLog(line)
    writeLog("END PRINT LOG")
    flushLog()
    benchmark.clear_logs()


_last_time = paraview.vtk.vtkTimerLog.GetUniversalTime()
def getElapsedTime():
    global _last_time
    current_time = paraview.vtk.vtkTimerLog.GetUniversalTime()
    elapsed_time = current_time - _last_time
    _last_time = current_time
    return elapsed_time


