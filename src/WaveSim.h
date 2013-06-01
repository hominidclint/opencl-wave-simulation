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

#ifndef WAVE_SIM_H
#define WAVE_SIM_H

#include "GlutApp.h"
#include "GLSLProgram.h"
#include "GridGenerator.h"
#include "CpuWaves.h" // #TODO replace with gpu implementation
//#include "GpuWaves.h"

// ocl
#include <CL/cl.h>
#include <CL/cl_gl.h>

// std
#include <string>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class WaveSim : public GlutApp
{
public:
    struct Vertex // for interleaved arrays
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
    };

    WaveSim(int argc, char** argv, int width, int height);
    ~WaveSim();

    virtual bool init();
    virtual void render();
    virtual void updateScene(float dt);
    virtual void onResize(int w, int h);

    virtual void onMouseEvent(int button, int state, int x, int y);
    virtual void onKeyboardEvent(unsigned char key, int x, int y);
    virtual void onMotionEvent(int x, int y);

protected:
    void initScene();
    void initOCL();
    void buildWaveGrid();

private:
    GLSLProgram* m_glslProgram;
    GLuint m_vaoHandle;

    glm::mat4 m_modelM;
    glm::mat4 m_viewM;
    glm::mat4 m_projM;

    int m_meshWidth;
    int m_meshHeight;

    // navigation
    float m_theta;
    float m_phi;
    float m_radius;

    int m_prevX;
    int m_prevY;
    int m_mouseBitMask;

    GridGenerator m_gridGenerator;
    CPUWaves m_waves;

    GLuint m_posVBO;
    GLuint m_indicesVBO;

    // ocl
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    cl_kernel m_kernel;
    size_t m_kernelsize;
    size_t m_global[2];
    cl_mem m_vbocl;

    // file loading
    std::string m_fxFilePath;
    std::string m_programSource;

};

#endif // WAVE_SIM_H