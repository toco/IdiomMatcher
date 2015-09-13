#cmake  -D IDA_DIR="/Applications/idaq.app/Contents/MacOS" -D IDA_SDK="contrib/idasdk67/" -D BOOST_DIR="contrib/boost_1_58_0" -D RAPID_JSON_DIR="contrib/rapidjson/include/" install

cmake_minimum_required(VERSION 2.8)
project(IdiomMatcher)

cmake_policy(SET CMP0015 NEW)

set(CMAKE_BUILD_TYPE Release)

if (WIN32)

    add_definitions(-D__NT__=1
                    -DUNICODE
                    -DWIN32
                    -D__IDP__)

    find_library(IDA_LIB
                 NAMES "ida"
                 PATHS "${IDA_SDK}/lib/x86_win_vc_32")

    find_library (IDA_LIB64
            NAMES "ida64"
            PATHS "${IDA_SDK}/lib/x86_win_vc_64")

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi")

elseif (APPLE)

    add_definitions(-D__MAC__=1)

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wno-deprecated-writable-strings")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wno-deprecated-writable-strings")

    set (CMAKE_OSX_ARCHITECTURES "i386;x86_64")

    find_library (IDA_LIB
                  NAMES "ida"
                  PATHS "${IDA_DIR}")

    find_library (IDA_LIB64
                  NAMES "ida64"
                  PATHS "${IDA_DIR}")

elseif (UNIX)

    add_definitions(-D__LINUX__=1)
	
    set (CMAKE_CXX_FLAGS "-m32 -pipe -O2 -pthread")

    set (CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

    find_library (IDA_LIB
                  NAMES "ida"
                  PATHS "${IDA_DIR}")

    find_library (IDA_LIB64
                  NAMES "ida64"
                  PATHS "${IDA_DIR}")


endif ( )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DUSE_DANGEROUS_FUNCTIONS -DUSE_STANDARD_FILE_FUNCTIONS -D__GXX_EXPERIMENTAL_CXX0X__  -Wno-unused-local-typedef -ggdb" )


include_directories(${IDA_SDK}/include
                    ${IDA_DIR}/plugins/hexrays_sdk/include
                    ${BOOST_DIR}
                    ${RAPID_JSON_DIR}
                    )

message (STATUS "IDA_DIR: " ${IDA_DIR})
message (STATUS "IDA_SDK: " ${IDA_SDK})
message (STATUS "IDA_LIB: " ${IDA_LIB})
message (STATUS "IDA_LIB64: " ${IDA_LIB64})

include_directories("src")

add_subdirectory("src/Model")
add_subdirectory("src/Matching")

add_subdirectory("src/IDA")
add_subdirectory("src/Standalone")

add_subdirectory("test/ModelTest")
add_subdirectory("test/MatchingTest")

enable_testing()


# workaround for CLion
add_custom_target(install-plugin COMMAND make install
                         CLION_EXE_DIR=${IdiomMatcherPlugin_BINARY_DIR})

