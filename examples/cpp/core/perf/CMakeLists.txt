add_bond_executable(
  perf
  EXCLUDE_FROM_ALL
  perf.cpp
  compat2.bond
  compat.bond
  compat_common.bond)

# Add the _apply.cpp files, because, when iterating on a perf test, the
# already expanded functions are just fine
target_sources(perf PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat2_apply.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_apply.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compat_common_apply.cpp")
