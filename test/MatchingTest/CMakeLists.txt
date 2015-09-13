


include_directories("../Matching")
include_directories("../Model")
add_executable (MatchingTest
        MatchingTest.cpp
        ../../src/Standalone/DumpDisassemblerAPI.cpp
        ../../src/Standalone/DumpDisassemblerAPI.h
)
target_link_libraries (MatchingTest
                       Matching
                       Model
                       )