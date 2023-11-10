
file(GLOB pyapi_compat_if_SRC
  "*.h"
  "*.cpp"
  )
  
add_library(pyapi-compat-if SHARED ${pyapi_compat_if_SRC})

target_include_directories(pyapi-compat-if PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${debug_mgr_INCDIR}
    )

target_link_directories(pyapi-compat-if PRIVATE
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib64
    "${debug_mgr_LIBDIR}"
    )
  
install(TARGETS pyapi-compat-if
    DESTINATION lib
    EXPORT pyapi-compat-if-targets)


