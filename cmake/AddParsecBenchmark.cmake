#set (PARSEC_PATH)

#set(LIBRARY_INCLUDE_DIR ${OTHER_LIB_ROOT_DIR}/database/include)
#./apps/ferret/inst/amd64-linux.gcc

#macro(AddParsecBenchmark benchmark)
#  set(multiValueArgs DEFINITIONS RENAME)
#  cmake_parse_arguments(AddParsecBenchmark
#    ""
#    ""
#    "ARGS;PATH"
#    ${ARGN})
#
#  set(PARSEC_APP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../parsec-3.0/pkgs)
#
#  add_custom_command(TARGET install_parsec PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E
#    copy ${PARSEC_APP_PATH}/${AddParsecBenchmark_PATH}/inst/amd64-linux.gcc
#    ${CMAKE_CURRENT_SOURCE_DIR}/${benchmark})
#
#  add_custom_target(bench-${benchmark}-pthread
#    COMMAND ${benchmark} ${AddParsecBenchmark_ARGS}
#    DEPENDS install_parsec)
#  #add_custom_target(bench-${benchmark}-tthread
#  #  COMMAND ${benchmark}-tthread ${AddParsecBenchmark_ARGS}
#  #  DEPENDS install_parsec)
#endmacro(AddBenchmark)
