

set (SOURCES
     DisassemblyPersistence.cpp
     DisassemblyPersistence.h
     Logging.cpp
     Logging.h
     Pattern.cpp
     Pattern.h
     PatternPersistence.cpp
     PatternPersistence.h
     )

add_library(Model STATIC ${SOURCES})
