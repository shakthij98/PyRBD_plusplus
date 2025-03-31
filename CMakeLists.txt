
cmake_minimum_required(VERSION 3.14)

project(RBD)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set gprof flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pg -O3")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

# execute pip show pybind11 to find the path of pybind11
execute_process(
    COMMAND pip show pybind11
    OUTPUT_VARIABLE PIP_SHOW_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX MATCH "Location: ([^\n]+)" MATCHED_LOCATION "${PIP_SHOW_OUTPUT}")
string(REGEX REPLACE "Location: " "" PYBIND11_LOCATION "${MATCHED_LOCATION}")

# set the path of pybind11
if (PYBIND11_LOCATION)
    set(pybind11_DIR "${PYBIND11_LOCATION}/pybind11/share/cmake/pybind11")
    message(STATUS "pybind11 found at: ${pybind11_DIR}")
else()
    message(FATAL_ERROR "pybind11 location not found. Ensure pybind11 is installed.")
endif()

# check if the pybind11 directory exists
if (EXISTS ${pybind11_DIR})
    message(STATUS "pybind11 directory exists: ${pybind11_DIR}")
else()
    message(FATAL_ERROR "pybind11 directory does not exist: ${pybind11_DIR}")
endif()

# find and include pybind11
find_package(pybind11 REQUIRED)

# find nlohmann_json library
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# set source files
set(SRC_FILES
    src/rbd.cpp
)

# include_directories(${IGRAPH_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# create a static library
add_library(rbd_library STATIC ${SRC_FILES})

# set the -fPIC flag on
set_target_properties(rbd_library PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rbd_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rbd_library nlohmann_json::nlohmann_json)

# link the pthread library
target_link_libraries(rbd_library pthread)

# set up rbd executable
add_executable(rbd src/rbd.cpp)
target_link_libraries(rbd rbd_library)

# set up python module
pybind11_add_module(rbd_bindings src/python_bindings.cpp)
target_link_libraries(rbd_bindings PRIVATE rbd_library)
