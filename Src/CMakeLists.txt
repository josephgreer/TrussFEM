cmake_minimum_required(VERSION 2.8)

Project(TrussFEM)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}/../external/armadillo/include
)

FILE(GLOB femFiles
	"*.cpp"
	"*.h"
)

ADD_EXECUTABLE(TrussFEM WIN32
	${femFiles}
)

SET_TARGET_PROPERTIES(TrussFEM PROPERTIES LINK_FLAGS_DEBUG /SUBSYSTEM:console)
 #Must be lowercase windows

SET_TARGET_PROPERTIES(TrussFEM PROPERTIES LINK_FLAGS_RELEASE /SUBSYSTEM:console)
 #Must be lowercase windows

 
TARGET_LINK_LIBRARIES(TrussFEM ${CMAKE_CURRENT_SOURCE_DIR}/../external/lapack/liblapack.lib ${CMAKE_CURRENT_SOURCE_DIR}/../external/lapack/libblas.lib winmm.lib)
