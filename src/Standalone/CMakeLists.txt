

add_executable(IdiomMatcherStandalone
		DumpDisassemblerAPI.cpp
		DumpDisassemblerAPI.h
		IdiomMatcherStandalone.cpp
		IdiomMatcherStandalone.h
		main.cpp
)


target_link_libraries(IdiomMatcherStandalone Model Matching)

