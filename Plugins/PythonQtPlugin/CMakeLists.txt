
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(PythonQt REQUIRED)

include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${PYTHONQT_INCLUDE_DIR})
include_directories(${VTK_INCLUDE_DIRS})

qt4_wrap_cpp(MOC_SRCS
  pqPythonQtPlugin.h
  pqPythonQtMethodHelpers.h
  pqPluginDecorators.h
  pqPythonEventFilter.h
  )

set(srcs
  pqPythonQtPlugin.cxx
  pqPythonQtWrapperFactory.cxx
  )

set(deps
  pqApplicationComponents
  ${PYTHONQT_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

add_paraview_auto_start(
  OUTIFACES
  OUTSRCS
  CLASS_NAME pqPythonQtPlugin)


add_paraview_plugin(pqPythonQtPlugin "1.0"
                    GUI_INTERFACES ${OUTIFACES}
                    GUI_SOURCES ${OUTSRCS} ${MOC_SRCS} ${srcs}
                    REQUIRED_ON_CLIENT)

target_link_libraries(pqPythonQtPlugin ${deps})
