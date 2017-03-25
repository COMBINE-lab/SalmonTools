include_directories(
${GAT_SOURCE_DIR}/include
${GAT_SOURCE_DIR}/external/install/include
)

set ( SALMON_TOOLS_MAIN_SRCS
SalmonTools.cpp
)

set (SALMON_TOOLS_LIB_SRCS
FastxParser.cpp
ExtractUnmapped.cpp
)

#set ( UNIT_TESTS_SRCS
#    ${GAT_SOURCE_DIR}/tests/UnitTests.cpp
#    FragmentLengthDistribution.cpp
#)


link_directories(
${GAT_SOURCE_DIR}/lib
${GAT_SOURCE_DIR}/external/install/lib
)

set(PTHREAD_LIB "")
# Set the RPATH
if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/../../lib:$ORIGIN/:$ORIGIN/../../external/install/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
else()
  # use, i.e. don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

  # the RPATH to be used when installing
  set(CMAKE_INSTALL_RPATH "")

  # don't add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

# Build the SalmonTools library
 add_library(salmon_tools_core STATIC ${SALMON_TOOLS_LIB_SRCS} )

# Build the salmon executable
add_executable(salmontools ${SALMON_TOOLS_MAIN_SRCS})

#add_executable(unitTests ${UNIT_TESTS_SRCS})

# Link the executable
target_link_libraries(salmontools
    salmon_tools_core
    #gff
    ${PTHREAD_LIB}
    ${ZLIB_LIBRARY}
    m
    ${NON_APPLECLANG_LIBS}
    ${LIBRT}
)

#add_dependencies(salmontools libcereal)
#add_dependencies(salmon unitTests)

##### ======================================
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    "${GAT_SOURCE_DIR}" CACHE PATH "Default install prefix" FORCE
    )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(INSTALL_LIB_DIR lib )
set(INSTALL_BIN_DIR bin )
set(INSTALL_INCLUDE_DIR include )

install(TARGETS salmontools 
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
        )

#add_custom_command(TARGET unitTests POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unitTests> ${GAT_SOURCE_DIR}/tests/$<TARGET_FILE_NAME:unitTests>
#    COMMENT "Copying unitTests"
#)
#set(POST_INSTALL_SCRIPT ${GAT_SOURCE_DIR}/cmake/PostInstall.cmake)
#install(
#    CODE
#    "
#    execute_process(COMMAND \"${CMAKE_COMMAND}\"
#                            -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
#                            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
#                            -P \"${POST_INSTALL_SCRIPT}\")
#    "
#)

#include(InstallRequiredSystemLibraries)
#add_test( NAME unit_tests COMMAND ${CMAKE_COMMAND} -DTOPLEVEL_DIR=${CMAKE_INSTALL_PREFIX} -P ${GAT_SOURCE_DIR}/cmake/UnitTests.cmake )
#add_test( NAME salmon_read_test_fmd COMMAND ${CMAKE_COMMAND} -DTOPLEVEL_DIR=${GAT_SOURCE_DIR} -P ${GAT_SOURCE_DIR}/cmake/TestSalmonFMD.cmake )
#add_test( NAME salmon_read_test_quasi COMMAND ${CMAKE_COMMAND} -DTOPLEVEL_DIR=${GAT_SOURCE_DIR} -P ${GAT_SOURCE_DIR}/cmake/TestSalmonQuasi.cmake )
