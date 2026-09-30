set(CORE_LIB_NAME "${PROJECT_NAME}-core")
set(ALGORITHM_LIB_NAME "${PROJECT_NAME}-algorithm")

function(add_qret_algorithm_test directory filename)
  string(REPLACE "/" "_" directory_name ${directory})
  set(target ${directory_name}_${filename})
  add_executable(${target} ${directory}/${filename}.cpp)
  target_link_libraries(
    ${target} PRIVATE GTest::gtest GTest::gtest_main ${CORE_LIB_NAME}
                      ${ALGORITHM_LIB_NAME})
  set_target_properties(
    ${target}
    PROPERTIES CXX_STANDARD 20
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)

  if(QRET_MEASURE_COVERAGE)
    target_compile_options(${target} PRIVATE -g -fprofile-instr-generate
                                             -fcoverage-mapping)
    target_link_options(${target} PRIVATE -g -fprofile-instr-generate
                        -fcoverage-mapping)
  endif()

  set(test_environment "GRIDSYNTH_PATH=${GRIDSYNTH_PATH}")
  set(test_environment_modification
      "PATH=path_list_prepend:$<TARGET_FILE_DIR:${CORE_LIB_NAME}>"
      "PATH=path_list_prepend:$<TARGET_FILE_DIR:${ALGORITHM_LIB_NAME}>"
      "PATH=path_list_prepend:$<TARGET_FILE_DIR:${target}>")

  if(WIN32)
    gtest_add_tests(
      TARGET ${target}
      SOURCES ${directory}/${filename}.cpp
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      TEST_LIST ${target}_tests)
    if(${target}_tests)
      set_tests_properties(
        ${${target}_tests}
        PROPERTIES ENVIRONMENT "${test_environment}"
                   ENVIRONMENT_MODIFICATION "${test_environment_modification}")
    endif()
  else()
    gtest_discover_tests(
      ${target}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} DISCOVERY_MODE PRE_TEST
      PROPERTIES ENVIRONMENT "${test_environment}" ENVIRONMENT_MODIFICATION
                 "${test_environment_modification}")
  endif()
endfunction()

# gate
add_qret_algorithm_test(gate/algorithm qft)
add_qret_algorithm_test(gate/algorithm qpe)
add_qret_algorithm_test(gate/arithmetic boolean)
add_qret_algorithm_test(gate/frontend circuit)
add_qret_algorithm_test(gate/arithmetic integer_add)
add_qret_algorithm_test(gate/arithmetic integer_compare)
add_qret_algorithm_test(gate/arithmetic integer_flip)
add_qret_algorithm_test(gate/arithmetic integer_inc)
add_qret_algorithm_test(gate/arithmetic integer_mul)
add_qret_algorithm_test(gate/arithmetic integer_scaled_add)
add_qret_algorithm_test(gate/arithmetic integer_sub)
add_qret_algorithm_test(gate/arithmetic modular)
add_qret_algorithm_test(gate/data qdict)
add_qret_algorithm_test(gate/data qrom)
add_qret_algorithm_test(gate/preparation arbitrary)
add_qret_algorithm_test(gate/preparation uniform)
add_qret_algorithm_test(gate/util bitwise)
add_qret_algorithm_test(gate/util multi_control)
add_qret_algorithm_test(gate/util rotation)
add_qret_algorithm_test(gate/util util)
