set(python_files
  CPUtils.py
  PartitionUtils.py
)

set(out_dir "${CMAKE_BINARY_DIR}/cpscripts")

foreach(file ${python_files})
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  set(tgt "${out_dir}/${file}")
  set(out_files ${out_files} ${tgt})
  add_custom_command(DEPENDS ${src}
                      COMMAND ${CMAKE_COMMAND} -E copy ${src} ${tgt}
                      OUTPUT ${tgt}
                      COMMENT "source copy")
endforeach()

add_custom_target(copy_cpscripts
                  ALL
                  DEPENDS ${out_files})
