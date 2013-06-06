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
set(target1 OpenCL-Wave-Simulation)
set(target2 CPU-Wave-Simulation)
set(target3 OpenGL-Warm-Up)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#find libs
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(GLUT REQUIRED)

#include helper
include(${CMAKE_MODULE_PATH}/helper.cmake)

set(common_sources
	src/GlutApp.h
	src/GlutApp.cpp
	src/CallbackHandler.h
	src/MathUtils.h
	src/MathUtils.cpp
	src/GLSLProgram.h
	src/GLSLProgram.cpp
	src/Chronometer.hpp
)

set(sources_opencl_wave_simulation
	src/main.cpp
	src/GpuWaves.h
	src/GpuWaves.cpp
	src/OpenCLWaveSimulation.h
	src/OpenCLWaveSimulation.cpp
)

set(sources_cpu_wave_simulation
	src/cpu_wave_sim.cpp
	src/WaveApp.h
	src/WaveApp.cpp
	src/CpuWaves.h
	src/CpuWaves.cpp
)

set(sources_opengl_warm_up
	src/ogl_warm_up.cpp
	src/OpenGLOnlyApp.h
	src/OpenGLOnlyApp.cpp
)

set(kernels
	src/kernel/WaveSimulation.cl
)

set(fx_wave_simulation
	src/fx/render_waves.vert
	src/fx/render_waves.frag
)

set(fx_opengl_warm_up
	src/fx/test.vert
	src/fx/test.frag
)

set(fx
	${fx_wave_simulation}
	${fx_opengl_warm_up}
)

SOURCE_GROUP(common FILES ${common_sources})

add_executable(${target1} ${common_sources} ${sources_opencl_wave_simulation} ${kernels} ${fx_wave_simulation})
add_executable(${target2} ${common_sources} ${sources_cpu_wave_simulation} ${fx_wave_simulation})
add_executable(${target3} ${common_sources} ${sources_opengl_warm_up} ${fx_opengl_warm_up})
configureDebugPostfix("d")
configureSourceGroups()
include_directories(
	${OPENCL_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
)

target_link_libraries(${target1}
	${OPENCL_LIBRARY}
	${OPENGL_LIBRARY}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}
)

target_link_libraries(${target2}
	${OPENGL_LIBRARY}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}
)

target_link_libraries(${target3}
	${OPENGL_LIBRARY}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}
)

install(TARGETS ${target1} ${target2} ${target3} DESTINATION build)
install(FILES ${kernels} DESTINATION build)
install(FILES ${fx} DESTINATION build)