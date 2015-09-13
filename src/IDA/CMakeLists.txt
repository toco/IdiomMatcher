
set (SOURCES
        AddMissingSwitchPatterns.cpp
        AddMissingSwitchPatterns.h
        IDAAdapter.cpp
        IDAAdapter.h
        IDAUIActions.cpp
        IDAUIActions.h
        UIHooks.cpp
        UIHooks.h
        StringSwitchSearch.cpp
        StringSwitchSearch.h
        plugin.cpp
)

if (WIN32)
     set (IDA_SUFFIX ".plw")
elseif (APPLE)
     set (IDA_SUFFIX ".pmc")
elseif (UNIX)
     set (IDA_SUFFIX ".plx")
endif ()

if(IDA_LIB)
     add_library(IdiomMatcherPlugin MODULE ${SOURCES})
     set_target_properties(IdiomMatcherPlugin PROPERTIES OSX_ARCHITECTURES "i386")

     set_target_properties (IdiomMatcherPlugin
             PROPERTIES
             PREFIX ""
             SUFFIX ${IDA_SUFFIX}
             OUTPUT_NAME ${CMAKE_PROJECT_NAME})

     target_link_libraries (IdiomMatcherPlugin  Model Matching ${IDA_LIB})
     install(TARGETS IdiomMatcherPlugin DESTINATION ${IDA_DIR}/plugins)
endif()

if(IDA_LIB64)
     add_library(IdiomMatcherPlugin64 MODULE ${SOURCES})
     set_target_properties(IdiomMatcherPlugin64 PROPERTIES OSX_ARCHITECTURES "x86_64")

     set_target_properties (IdiomMatcherPlugin64
             PROPERTIES
             PREFIX ""
             SUFFIX ${IDA_SUFFIX}64
             OUTPUT_NAME ${CMAKE_PROJECT_NAME})

     target_compile_definitions(IdiomMatcherPlugin64 PUBLIC __X64__)

     target_link_libraries (IdiomMatcherPlugin64 Model Matching ${IDA_LIB64})
     install(TARGETS IdiomMatcherPlugin64 DESTINATION ${IDA_DIR}/plugins)
endif()