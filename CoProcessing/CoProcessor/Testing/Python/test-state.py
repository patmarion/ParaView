from paraview.simple import *

import libvtkPVFiltersPython as pvfilters


s = Sphere()
r = Reflect()
g = GenerateIds()
clip = Clip()


c = servermanager.vtk.vtkCollection()
c.AddItem(s.SMProxy)
c.AddItem(r.SMProxy)
c.AddItem(clip.SMProxy)

clip.ClipType.Normal = [99,1,2]

elem = servermanager.ProxyManager().SaveState() #(c, True)


elem.PrintXML()

loader = servermanager.vtkSMCPStateLoader()
loader.Go(elem)
elem.UnRegister(None)



sources = [loader.GetSource(i) for i in xrange(loader.GetNumberOfSources())]
#print sources

sources[0].GetProperty("Center").SetElement(0, 99)
elem = sources[0].SaveState(None)
elem.PrintXML()


print s.Center
s.LoadState(elem, None)
print s.Center


#print clip.ClipType.Normal


elem.UnRegister(None)


c = servermanager.vtk.vtkCollection()
c.AddItem(s.SMProxy)
c.AddItem(r.SMProxy)
#servermanager.ProxyManager().SaveState(c, False).PrintXML()




