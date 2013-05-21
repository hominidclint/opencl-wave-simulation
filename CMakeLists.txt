cmake_minimum_required(VERSION 2.8.10.2)
project(OGL_OCL_Interop)
set(target OGLInteropOCL)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#find libs
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

#include helper
include(${CMAKE_MODULE_PATH}/helper.cmake)

set(sources
	src/main.cpp
	#src/gpu_waves.cpp
	src/GlutApp.h
	src/GlutApp.cpp
	src/WaveApp.h
	src/WaveApp.cpp
	src/GridGenerator.h
	src/GridGenerator.cpp
	src/MathUtils.h
	src/MathUtils.cpp
	src/CpuWaves.h
	src/CpuWaves.cpp
	src/GpuWaves.h
	src/GpuWaves.cpp
	src/ExampleApp.h
	src/ExampleApp.cpp
)

set(kernels
	src/fxWaves.cl
)

set(all_sources
	${sources}
	${kernels}
)

add_executable(${target} ${all_sources})
configureDebugPostfix("d")
configureSourceGroups()
include_directories(
	${OPENCL_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

target_link_libraries(${target}
	${OPENCL_LIBRARY}
	${OPENGL_LIBRARY}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}
)

install(TARGETS ${target} DESTINATION bin)
install(FILES ${kernels} DESTINATION bin)

# hack
install(FILES ${GLEW_INCLUDE_DIR}/../bin/glew32.dll DESTINATION bin)