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

#include "ExampleApp.h"
#include "CallbackHandler.h"

#include <iostream>
#include <fstream>

using namespace GLSLShader;

ExampleApp::ExampleApp(int argc, char** argv, int width, int height, unsigned int meshWidth, unsigned int meshHeight)
    : GlutApp(argc, argv, width, height),
      m_meshWidth(meshWidth),
      m_meshHeight(meshHeight),
      m_anim(0.0f),
      m_mouseBitMask(0),
      m_rotateX(0.0f),
      m_rotateY(0.0f),
      m_translateZ(-5.0f),
      m_argc(argc),
      m_drawMode(GL_LINE_STRIP),
      m_glslProgram(new GLSLProgram)
{
    m_global[0] = meshWidth;
    m_global[1] = meshHeight;
}

ExampleApp::~ExampleApp()
{

};

bool ExampleApp::init()
{
    if(!GlutApp::init())
    {
        return false;
    }

    ::g_app = this;
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutReshapeFunc(resize);

    glewInit();

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    initOCL();
    initShader();
    

    return true;
}

void ExampleApp::initShader()
{
    if(!m_glslProgram->compileShaderFromFile("test.vert", GLSLShader::VERTEX))
    {
        std::cerr << "Vertex shader failed to compile\n";
        std::cerr << "Build Log: " << m_glslProgram->log() << std::endl;
        exit(1);
    }

    if(!m_glslProgram->compileShaderFromFile("test.frag", GLSLShader::FRAGMENT))
    {
        std::cerr << "Fragment shader failed to compile\n";
        std::cerr << "Build Log: " << m_glslProgram->log() << std::endl;
        exit(1);
    }

    // possible call bindAttribLocation or bindFragDataLocation here
    m_glslProgram->bindAttribLocation(0, "vPos");
    m_glslProgram->bindFragDataLocation(0, "FragColor");

    if(!m_glslProgram->link())
    {
        std::cerr << "Shader program failed to link\n";
        std::cerr << "Link Log: " << m_glslProgram->log() << std::endl;
    }

    m_glslProgram->use();
    m_glslProgram->printActiveAttribs();
    m_glslProgram->printActiveUniforms();

    // init uniforms
    m_modelM = glm::mat4(1.0f);
    m_modelM *= glm::translate(0.0f, 0.0f, m_translateZ);
    m_viewM = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_projM = glm::perspective(45.0f, aspectRatio(), 0.1f, 100.0f);

    glGenVertexArrays(1, &m_vaoHandle);
    glBindVertexArray(m_vaoHandle);

    glEnableVertexAttribArray(0); // vPos

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindVertexArray(0);
}

void ExampleApp::initOCL()
{
    const unsigned int vboSize = m_meshWidth * m_meshHeight * 4 * sizeof(float);
    clGetPlatformIDs(1, &m_platform, NULL);

    if(m_argc > 1)
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 1, &m_device, NULL);
    }
    else
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 1, &m_device, NULL);
    }

    // add ocl context must be tied to the ogl context
#   ifdef _WIN32
    HGLRC glCtx = wglGetCurrentContext();
#   else
    GLXContext glCtx = glXGetCurrentContext();
#   endif

    cl_context_properties props[] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)m_platform,
#   ifdef _WIN32
                                     CL_WGL_HDC_KHR, (intptr_t) wglGetCurrentDC(),
#   else
                                     CL_GLX_DISPLAY_KHR, (intptr_t) glXGetCurrentDisplay();
#   endif
                                     CL_GL_CONTEXT_KHR, (intptr_t) glCtx, 0
    };

    // create context and queue
    m_context = clCreateContext(props, 1, &m_device, NULL, NULL, NULL);
    m_queue = clCreateCommandQueue(m_context, m_device, 0, NULL);

    // create position VBO
    glGenBuffers(1, &m_vbo);
    // allocates space on the device
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // init buffer object
    glBufferData(GL_ARRAY_BUFFER, vboSize, 0, GL_DYNAMIC_DRAW);
    // create ocl buffer grom ogl VBO
    m_vbocl = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_vbo, NULL);

    // load source files
    std::ifstream file("fxWaves.cl");
    std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    file.close();

    const char* source = prog.c_str();
    const size_t kernelsize = prog.length() + 1;
    m_program = clCreateProgramWithSource(m_context, 1, (const char**)&source, &kernelsize, NULL);

    // build program
    int err = clBuildProgram(m_program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        std::cerr << "Error: Failed to build program executable!" << std::endl;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        std::cerr << buffer << std::endl;
        exit(1);
    }

    // create the compute kernel in the program
    m_kernel = clCreateKernel(m_program, "sinewave", &err);
    if(!m_kernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel!" << std::endl;
        exit(1);
    }

    // set kernel arguments
    clSetKernelArg(m_kernel, 0, sizeof(cl_mem), (void*)&m_vbocl);
    clSetKernelArg(m_kernel, 1, sizeof(unsigned int), &m_meshWidth);
    clSetKernelArg(m_kernel, 2, sizeof(unsigned int), &m_meshHeight);
}

void ExampleApp::render()
{
    updateScene(0); // #TODO

    // called per frame
    m_anim += 0.01f;

    // map ogl buffer object for writing from OpenCL
    glFinish();
    clEnqueueAcquireGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);

    // set arg 3 and queue the kernel
    clSetKernelArg(m_kernel, 3, sizeof(float), &m_anim);
    clEnqueueNDRangeKernel(m_queue, m_kernel, 2, NULL, m_global, NULL, 0, 0, 0); // #TODO tweak

    // queue unmap buffer object
    clEnqueueReleaseGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);
    clFinish(m_queue); // sync

    checkGLError(__FILE__,__LINE__);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set uniforms
    m_modelM = glm::mat4(1.0f);
    m_modelM *= glm::translate(0.0f, 0.0f, m_translateZ);
    m_modelM *= glm::rotate((m_rotateX), glm::vec3(1.0f,0.0f,0.0f));
    m_modelM *= glm::rotate((m_rotateY), glm::vec3(0.0f,1.0f,0.0f));

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);


    // render from the p_vbo
    glBindVertexArray(m_vaoHandle);

    // draw points, lines or triangles according to the user keyboard input
    switch(m_drawMode)
    {
    case GL_LINE_STRIP:
        for(unsigned int i = 0; i < m_meshWidth * m_meshHeight; i += m_meshWidth)
        {
            glDrawArrays(GL_LINE_STRIP, i, m_meshWidth);
        }
        break;
    default:
        glDrawArrays(GL_POINTS, 0, m_meshWidth * m_meshHeight);
        break;
    }

    glutSwapBuffers();
}

void ExampleApp::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(45.0f, aspectRatio(), 0.1f, 100.0f);

    glutPostRedisplay();
}

void ExampleApp::updateScene(float dt)
{
}

void ExampleApp::onMouseEvent(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        m_mouseBitMask |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        m_mouseBitMask = 0;
    }

    m_prevX = x;
    m_prevY = y;
    glutPostRedisplay();
}

void ExampleApp::onKeyboardEvent(unsigned char key, int x, int y)
{
    switch(key)
    {
    case('q'):
    case(27):
        exit(0);
        break;

    case 'w':
    case 'W':
        switch(m_drawMode)
        {
        case GL_POINTS: m_drawMode = GL_LINE_STRIP; break;
        case GL_LINE_STRIP: m_drawMode = GL_POINTS; break;
        default: m_drawMode = GL_POINTS; break;
        }
    }
}

void ExampleApp::onMotionEvent(int x, int y)
{
    float dx, dy;
    dx = static_cast<float>(x - m_prevX);
    dy = static_cast<float>(y - m_prevY);

    if(m_mouseBitMask & 1)
    {
        m_rotateX += dy * 0.2f;
        m_rotateY += dx * 0.2f;
    }
    else if(m_mouseBitMask & 4)
    {
        m_translateZ += dy * 0.01f;
    }

    m_prevX = x;
    m_prevY = y;
}