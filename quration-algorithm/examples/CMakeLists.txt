set(CORE_LIB_NAME "${PROJECT_NAME}-core")
set(ALGORITHM_LIB_NAME "${PROJECT_NAME}-algorithm")

function(add_exe filename)
  set(target ${filename})
  add_executable(${target} ${target}.cpp)
  target_link_libraries(
    ${target} PRIVATE Boost::program_options ${CORE_LIB_NAME}
                      ${ALGORITHM_LIB_NAME})
  target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  set_target_properties(
    ${target}
    PROPERTIES CXX_STANDARD 20
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)
  target_compile_options(${target} PRIVATE ${QRET_COMPILE_OPTIONS})
  qret_copy_runtime_dependencies(${target})
  install(TARGETS ${target})
endfunction()

add_exe(compile_adder_to_distributed_chip)
add_exe(generate_quration_core_test_circuits)
add_exe(qpe)
add_exe(shor)
