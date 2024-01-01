
ExternalProject_Add(PyEvalBase
  PREFIX pyeval_base
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/pyeval_base"
  DOWNLOAD_COMMAND
       ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/gen_py_if.py ${CMAKE_CURRENT_BINARY_DIR}/pyeval_base
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  )

file(GLOB pyapi_compat_if_SRC
  "*.h"
  "*.cpp"
  )
  
add_library(pyapi-compat-if SHARED ${pyapi_compat_if_SRC})

target_include_directories(pyapi-compat-if PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_BINARY_DIR}/pyeval_base/include
    ${debug_mgr_INCDIR}
    )

target_link_directories(pyapi-compat-if PRIVATE
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib64
    "${debug_mgr_LIBDIR}"
    )

add_dependencies(pyapi-compat-if PyEvalBase)
  
install(TARGETS pyapi-compat-if
    DESTINATION lib
    EXPORT pyapi-compat-if-targets)


