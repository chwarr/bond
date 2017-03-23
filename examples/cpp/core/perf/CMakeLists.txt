add_bond_codegen(
  compat2.bond
  compat.bond
  compat_common.bond
  OPTIONS
  --allocator=PoolAlloc
  --header=\\\"allocator.h\\\")

add_executable(
  perf
  EXCLUDE_FROM_ALL
  perf.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat2_types.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_types.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_common_types.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat2_apply.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_apply.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_common_apply.cpp")

target_include_directories (perf PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(perf PRIVATE bond bond_apply)
