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

#ifndef EXAMPLE_APP_H
#define EXAMPLE_APP_H

#include "GlutApp.h"
#include "GLSLProgram.h"

#include <string>

// ocl includes
#include <CL/cl.h>
#include <CL/cl_gl.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class ExampleApp : public GlutApp
{
public:
    struct Vertex // for interleaved arrays
    {
        glm::vec3 position; // strided offset 0
        glm::vec3 normal; // strided offset 12 (3 * 4)
        glm::vec2 uv; // strided offset 24 (6 * 4)
    };

    ExampleApp(int argc, char** argv, int width, int height, unsigned int meshWidth, unsigned int meshHeight);
    ~ExampleApp();

    virtual bool init();
    virtual void drawScene();
    virtual void updateScene(float dt);
    virtual void onResize(int w, int h);

    virtual void onMouseEvent(int button, int state, int x, int y);
    virtual void onKeyboardEvent(unsigned char key, int x, int y);
    virtual void onMotionEvent(int x, int y);

protected:
    void initOCL();
    void initShader();

private:
    // rendering and computing
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    cl_kernel m_kernel;
    size_t m_kernelsize;
    size_t m_global[2];

    unsigned int m_meshWidth;
    unsigned int m_meshHeight;

    GLSLProgram* m_glslProgram;
    GLuint m_vaoHandle;

    glm::mat4 m_modelM;
    glm::mat4 m_viewM;
    glm::mat4 m_projM;

    GLuint m_vbo;
    cl_mem m_vbocl;

    float m_anim;
    int m_drawMode;

    // file loading
    std::string m_fxFilePath;
    std::string m_programSource;

    // navigation
    int m_prevX;
    int m_prevY;
    int m_mouseBitMask;
    float m_rotateX;
    float m_rotateY;
    float m_translateZ;

    // hack
    int m_argc;
};

#endif // EXAMPLE_APP_H