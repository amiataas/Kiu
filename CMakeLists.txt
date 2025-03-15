CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Kiu LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB SOURCES
	${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB HEADER_FILES
	${PROJECT_SOURCE_DIR}/include/*.h)

add_executable(Kiu ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support)
target_link_libraries(Kiu LLVM ${llvm_libs})
