


include_directories("../Model")

add_executable (ModelTest PatternTest.cpp)
target_link_libraries (ModelTest
                       Model
                       )