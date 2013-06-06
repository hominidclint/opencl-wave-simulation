// Copyright (c) 2013, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef OPENCL_WAVE_SIMULATION_H
#define OPENCL_WAVE_SIMULATION_H

// own
#include "GlutApp.h"
#include "GLSLProgram.h"
#include "Chronometer.hpp"
#include "GpuWaves.h"

// std
#include <string>

// ocl
#include <CL/cl.h>
#include <CL/cl_gl.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class OpenCLWaveSimulation : public GlutApp
{
public:
    OpenCLWaveSimulation(int argc, char** argv, const std::string& appName, int width, int height, int gridWidth, int gridHeight);
    ~OpenCLWaveSimulation();

    virtual bool init();
    virtual void render();
    virtual void updateScene(double dt);
    virtual void onResize(int w, int h);

    virtual void onMouseEvent(int button, int state, int x, int y);
    virtual void onKeyboardEvent(unsigned char key, int x, int y);
    virtual void onMotionEvent(int x, int y);

protected:
    void initScene();
    void initOCL();
    void cleanup();

    void buildWaveGrid();

    void computeVertexDisplacement();
    void computeFiniteDifferenceScheme();
    void disturbGrid();
    void initGLBuffer();

private:
    // ocl
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;

    cl_kernel m_vertexDisplacementKernel;
    cl_kernel m_finiteDifferenceSchemeKernel;
    cl_kernel m_disturbKernel;
    cl_kernel m_glGridInitKernel;

    cl_mem m_clPositionInteropBuffer;
    cl_mem m_clNormalInteropBuffer;
    cl_mem m_clTangentInteropBuffer;
    cl_mem m_clPing;
    cl_mem m_clPong;

    size_t m_kernelsize;
    size_t m_global[2];
    std::string m_fxFilePath;
    std::string m_programSource;

    int m_gridWidth;
    int m_gridHeight;

    // ogl
    GLSLProgram* m_glslProgram;
    GLuint m_vaoWaves;

    // transformation matrices
    glm::mat4 m_modelM;
    glm::mat4 m_viewM;
    glm::mat4 m_projM;
    glm::mat3 m_worldInvTransposeM;

    // vbos
    GLuint m_positionVBO;
    GLuint m_normalVBO;
    GLuint m_tangentVBO;
    GLuint m_ibo;

    // light, material and camera
    glm::vec4 m_materialAmbient;
    glm::vec4 m_materialDiffuse;
    glm::vec4 m_materialSpecular; // w component for shininess

    glm::vec3 m_lightDir;
    glm::vec4 m_lightAmbient;
    glm::vec4 m_lightDiffuse;
    glm::vec4 m_lightSpecular;
    
    // animation
    Chronometer m_waveTrigger;
    bool m_pingpong;
    GPUWaves m_waves;

    // navigation
    float m_theta;
    float m_phi;
    float m_radius;

    int m_prevX;
    int m_prevY;
    int m_mouseBitMask;
};

#endif // OPENCL_WAVE_SIMULATION_H