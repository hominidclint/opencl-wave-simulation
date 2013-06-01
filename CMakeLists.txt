# Copyright (c) 2013, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8.10.2)
project(OGL_OCL_Interop)
set(target OGLInteropOCL)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#find libs
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(GLUT REQUIRED)

#include helper
include(${CMAKE_MODULE_PATH}/helper.cmake)

set(sources
	src/main.cpp
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
	src/GLSLProgram.h
	src/GLSLProgram.cpp
	src/OpenGLOnlyApp.h
	src/OpenGLOnlyApp.cpp
	src/CallbackHandler.h
	src/WaveSim.h
	src/WaveSim.cpp
)

set(kernels
	src/fxWaves.cl
	src/WaveSimulation.cl
)

set(fx
	src/shader/simple.vert
	src/shader/simple.frag
	src/shader/test.vert
	src/shader/test.frag
)

set(all_sources
	${sources}
	${kernels}
	${fx}
)

add_executable(${target} ${all_sources})
configureDebugPostfix("d")
configureSourceGroups()
include_directories(
	${OPENCL_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
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
install(FILES ${fx} DESTINATION bin)

# hack
install(FILES ${GLEW_INCLUDE_DIR}/../bin/glew32.dll DESTINATION bin)