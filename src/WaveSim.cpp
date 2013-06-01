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

#include "WaveSim.h"
#include "CallbackHandler.h"
#include "MathUtils.h"

#include <iostream>
#include <fstream>

#define SIZE 128


WaveSim::WaveSim(int argc, char** argv, int width, int height)
    : GlutApp(argc, argv, width, height),
    m_mouseBitMask(0),
    m_glslProgram(new GLSLProgram),
    m_theta(1.5f * MathUtils::Pi),
    m_phi(0.1f * MathUtils::Pi),
    m_radius(200.0f),
    m_prevX(0),
    m_prevY(0),
    m_meshWidth(SIZE),
    m_meshHeight(SIZE)
{
    m_global[0] = m_meshWidth;
    m_global[1] = m_meshHeight;
}

WaveSim::~WaveSim()
{
    delete m_glslProgram;
};

bool WaveSim::init()
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
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    initScene();
    initOCL();

    return true;
}

void WaveSim::initOCL()
{
    const unsigned int vboSize = SIZE * SIZE * 3 * sizeof(float);
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

    // create ocl vbo from ogl buffer
    int errorCode;
    m_vbocl = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_posVBO, &errorCode);

    if(errorCode == CL_SUCCESS)
    {
        std::cout << "CL Buffer creation was successful.\n";
    }

    // load source files
    std::ifstream file("WaveSimulation.cl");
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
    else
    {
        std::cout << "CL Program build was successful.\n";
    }

    // create the compute kernel in the program
    m_kernel = clCreateKernel(m_program, "wave_simulation", &err);
    if(!m_kernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel!" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "CL Kernel creation was successful.\n\n";
    }

    // set kernel arguments
    clSetKernelArg(m_kernel, 0, sizeof(cl_mem), (void*)&m_vbocl);
    clSetKernelArg(m_kernel, 1, sizeof(unsigned int), &m_meshWidth);
    clSetKernelArg(m_kernel, 2, sizeof(unsigned int), &m_meshHeight);
}

void WaveSim::buildWaveGrid()
{
    m_waves.init(SIZE, SIZE, 1.0f, 0.03f, 3.25f, 0.4f);

    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    m_posVBO = vboHandles[0];
    m_indicesVBO = vboHandles[1];

    // TODO hack
    glm::vec3* test = m_waves.getCurrentWaves();
    glm::vec4 test2[SIZE*SIZE];

    for(unsigned int i = 0; i < m_waves.vertexCount(); ++i)
    {
        test2[i] = glm::vec4(m_waves[i], 1.0f);
    }

    // create vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * SIZE*SIZE, reinterpret_cast<float*>(m_waves.getCurrentWaves()), GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * SIZE*SIZE, reinterpret_cast<float*>(&test2[0]), GL_DYNAMIC_DRAW);

    // create index buffer
    unsigned int* indices = new unsigned int[3 * m_waves.triangleCount()];
    unsigned int m = m_waves.rowCount();
    unsigned int n = m_waves.columnCount();
    unsigned int k = 0;
    for(unsigned int i = 0; i < m-1; ++i)
    {
        for(unsigned int j = 0; j < n-1; ++j)
        {
            indices[k]   = i*n+j;
            indices[k+1] = i*n+j+1;
            indices[k+2] = (i+1)*n+j;

            indices[k+3] = (i+1)*n+j;
            indices[k+4] = i*n+j+1;
            indices[k+5] = (i+1)*n+j+1;

            k += 6; // next quad
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_waves.triangleCount(), indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vaoHandle);
    glBindVertexArray(m_vaoHandle);

    glEnableVertexAttribArray(0); // vPos;
    //glEnableVertexAttribArray(1); // vColor; // TODO

    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVBO);
    glBindVertexArray(0);

    delete[] indices;
}

void WaveSim::initScene()
{
    if(!m_glslProgram->compileShaderFromFile("simple.vert", GLSLShader::VERTEX))
    {
        std::cerr << "Vertex shader failed to compile\n";
        std::cerr << "Build Log: " << m_glslProgram->log() << std::endl;
        exit(1);
    }

    if(!m_glslProgram->compileShaderFromFile("simple.frag", GLSLShader::FRAGMENT))
    {
        std::cerr << "Fragment shader failed to compile\n";
        std::cerr << "Build Log: " << m_glslProgram->log() << std::endl;
        exit(1);
    }

    // bindAttribLocation or bindFragDataLocation here
    m_glslProgram->bindAttribLocation(0, "vPos");
    m_glslProgram->bindAttribLocation(1, "vColor"); // #TODO
    m_glslProgram->bindFragDataLocation(0, "FragColor");

    if(!m_glslProgram->link())
    {
        std::cerr << "Shader program failed to link\n";
        std::cerr << "Link Log: " << m_glslProgram->log() << std::endl;
    }

    m_glslProgram->use();
    m_glslProgram->printActiveAttribs();
    m_glslProgram->printActiveUniforms();
    std::cout << std::endl;

    // init uniforms
    m_modelM = glm::mat4(1.0f);
    m_viewM = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 1000.0f);

    buildWaveGrid();
}

void WaveSim::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 1000.0f);

    glutPostRedisplay();
}

void WaveSim::render()
{
    checkGLError(__FILE__,__LINE__);

    static float anim = 0;
    anim += 0.01f;
    updateScene(anim);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(m_vaoHandle);
    glDrawElements(GL_TRIANGLES, 3 * m_waves.triangleCount(), GL_UNSIGNED_INT, ((GLubyte*)NULL + (0)));

    glutSwapBuffers();
}

void WaveSim::updateScene(float dt)
{
    // convert spherical to cartesian coordinates
    float x = m_radius * sinf(m_phi) * cosf(m_theta);
    float z = m_radius * sinf(m_phi) * sinf(m_theta);
    float y = m_radius * cosf(m_phi);

    // build view matrix
    glm::vec3 pos(x, y, z);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    m_viewM = glm::lookAt(pos, target, up);

    // build model matrix
    m_modelM = glm::mat4(1.0f);
    m_modelM *= glm::translate(-(float)SIZE/2.0f, 0.0f, -(float)SIZE/2.0f); // translate to world center
    

    int i = 5 + rand() % (m_waves.rowCount()-10);
    int j = 5 + rand() % (m_waves.columnCount()-10);

    float r = MathUtils::randF(1.0f, 2.0f);
    m_waves.disturb(i, j, r);

    m_waves.update(dt);

    // #TODO to gpu implementation
    // map ogl buffer object for writing from OpenCL
    glFinish();
    clEnqueueAcquireGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);

    // set arg 3 and queue the kernel
    clSetKernelArg(m_kernel, 3, sizeof(float), &dt);
    clEnqueueNDRangeKernel(m_queue, m_kernel, 2, NULL, m_global, NULL, 0, 0, 0); // #TODO tweak

    // queue unmap buffer object
    clEnqueueReleaseGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);
    clFinish(m_queue); // sync
}

void WaveSim::onMouseEvent(int button, int state, int x, int y)
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

void WaveSim::onKeyboardEvent(unsigned char key, int x, int y)
{
    if(key == 27)
    {
        exit(0);
    }
}

void WaveSim::onMotionEvent(int x, int y)
{
    if(m_mouseBitMask & 1)
    {
        float dx = glm::radians(0.25f * static_cast<float>(x - m_prevX));
        float dy = glm::radians(0.25f * static_cast<float>(y - m_prevY));

        m_theta += dx;
        m_phi += dy;

        m_phi = MathUtils::clamp(m_phi, 0.1f, MathUtils::Pi - 0.1f);
    }
    else if(m_mouseBitMask & 4)
    {
        float dx = 0.2f * static_cast<float>(x - m_prevX);
        float dy = 0.2f * static_cast<float>(y - m_prevY);

        m_radius += dx - dy;
        m_radius = MathUtils::clamp(m_radius, 1.0f, 500.0f);
    }

    m_prevX = x;
    m_prevY = y;
}