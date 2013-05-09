if (PARAVIEW_BUILD_QT_GUI AND PARAVIEW_ENABLE_PYTHON)

  pv_plugin(PythonQt
    DESCRIPTION "PythonQt Plugin"
    )

endif()
