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

// own
#include "OpenCLWaveSimulation.h"
#include "MathUtils.h"
#include "CallbackHandler.h"

// std
#include <iostream>
#include <fstream>

#define VERTEX_SIZE 3

OpenCLWaveSimulation::OpenCLWaveSimulation(int argc, char** argv, int width, int height, int gridWidth, int gridHeight)
    : GlutApp(argc, argv, width, height),
      m_gridWidth(gridWidth),
      m_gridHeight(gridHeight),
      m_mouseBitMask(0),
      m_glslProgram(new GLSLProgram),
      m_theta(1.5f * MathUtils::Pi),
      m_phi(0.1f * MathUtils::Pi),
      m_radius(200.0f),
      m_prevX(0),
      m_prevY(0),
      m_pingpong(true)
{
    m_global[0] = gridWidth;
    m_global[1] = gridHeight;
}

OpenCLWaveSimulation::~OpenCLWaveSimulation()
{
    // TODO
    delete m_glslProgram;
};

bool OpenCLWaveSimulation::init()
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
    m_timer.start();

    return true;
}

void OpenCLWaveSimulation::initScene()
{
    if(!m_glslProgram->compileShaderFromFile("render_waves.vert", GLSLShader::VERTEX))
    {
        std::cerr << "Vertex shader failed to compile\n";
        std::cerr << "Build Log: " << m_glslProgram->log() << std::endl;
        exit(1);
    }

    if(!m_glslProgram->compileShaderFromFile("render_waves.frag", GLSLShader::FRAGMENT))
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

    // init uniforms
    m_modelM = glm::mat4(1.0f);
    m_viewM = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 1000.0f);

    buildWaveGrid();
}

void OpenCLWaveSimulation::buildWaveGrid()
{
    assert(m_gridWidth == m_gridHeight);
    m_waves.init(m_gridWidth, m_gridHeight, 1.0f, 0.03f, 3.25f, 0.4f); // #TODO

    GLuint vboHandles[4];
    glGenBuffers(4, vboHandles);
    m_vboPing = vboHandles[0];
    m_vboPong = vboHandles[1];
    m_indicesPing = vboHandles[2];
    m_indicesPong = vboHandles[3];

    // create vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPing);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * m_gridWidth*m_gridHeight, reinterpret_cast<float*>(m_waves.getCurrentWaves()), GL_STREAM_DRAW); // #TODO reinterpret_cast

    glBindBuffer(GL_ARRAY_BUFFER, m_vboPong);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * m_gridWidth*m_gridHeight, reinterpret_cast<float*>(m_waves.getCurrentWaves()), GL_STREAM_DRAW); // #TODO reinterpret_cast

    // #TODO refactoring: move to GPUWaves class
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesPing);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_waves.triangleCount(), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesPong);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_waves.triangleCount(), indices, GL_STATIC_DRAW);

    GLuint vaoHandles[2];
    glGenVertexArrays(2, vaoHandles);
    m_vaoPing = vaoHandles[0];
    m_vaoPong = vaoHandles[1];

    glBindVertexArray(m_vaoPing);
    glEnableVertexAttribArray(0); // vPos; #TODO color, normals etc
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPing);
    glVertexAttribPointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesPing);
    glBindVertexArray(0);

    glBindVertexArray(m_vaoPong);
    glEnableVertexAttribArray(0); // vPos #TODO
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPong);
    glVertexAttribPointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesPong);
    glBindVertexArray(0);

    delete[] indices;
}

void OpenCLWaveSimulation::initOCL()
{
    const unsigned int vboSize = m_gridWidth * m_gridHeight * VERTEX_SIZE * sizeof(float);
    clGetPlatformIDs(1, &m_platform, NULL);

    if(m_argc > 1)
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 1, &m_device, NULL);
    }
    else
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 1, &m_device, NULL);
    }

    // ocl context must be tied to the ogl context
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

    // create buffers
    m_clPing = clCreateFromGLBuffer(m_context, CL_MEM_READ_WRITE, m_vboPing, NULL);
    m_clPong = clCreateFromGLBuffer(m_context, CL_MEM_READ_WRITE, m_vboPong, NULL);

    // load source file
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

    // create the compute kernels in the program
    m_vertexDisplacementKernel = clCreateKernel(m_program, "compute_vertex_displacement", &err);
    if(!m_vertexDisplacementKernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel: compute_vertex_displacement!" << std::endl;
        exit(1);
    }

    m_finiteDifferenceSchemeKernel = clCreateKernel(m_program, "compute_finite_difference_scheme", &err);
    if(!m_finiteDifferenceSchemeKernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel: compute_finite_difference_scheme!" << std::endl;
        exit(1);
    }

    m_disturbKernel = clCreateKernel(m_program, "disturb_grid", &err);
    if(!m_disturbKernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel: disturb_grid!" << std::endl;
        exit(1);
    }
}

void OpenCLWaveSimulation::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 1000.0f);

    glutPostRedisplay();
}

void OpenCLWaveSimulation::render()
{
    checkGLError(__FILE__,__LINE__);

    static float anim = 0;
    anim += 0.01f;
    updateScene(anim);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if(m_pingpong)
    {
        glBindVertexArray(m_vaoPing);
    }
    else
    {
        glBindVertexArray(m_vaoPong);
    }
    glDrawElements(GL_TRIANGLES, 3 * m_waves.triangleCount(), GL_UNSIGNED_INT, ((GLubyte*)NULL + (0)));
    //glDrawArrays(GL_POINTS, 0, m_waves.vertexCount());

    m_pingpong = !m_pingpong;
    glutSwapBuffers();

    //////////////////////////////////////////////////////////////////////////
    // test
    //

    //glBindBuffer(GL_ARRAY_BUFFER, m_vboPing);
    //glm::vec4* mappedData = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));

    /*if(m_pingpong)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vboPing);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vboPong);
    }
    glm::vec3* mappedData = reinterpret_cast<glm::vec3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    */
    for(unsigned int i = 0; i < 1000; ++i)
    {
        //mappedData[i] = m_waves[i];
        std::cout << "tata" << std::endl;
    }
    //glUnmapBuffer(GL_ARRAY_BUFFER);
}

void OpenCLWaveSimulation::updateScene(float dt)
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

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);

    glFinish();

    if(m_timer.getPassedTimeSinceStart() >= 0.4f) //0.4 seconds
    {
        std::cout << "disturb()\n";
        //disturbGrid(); // hat wenig auswirkung
        m_timer.stop();
        m_timer.start();
    }

    computeVertexDisplacement();
}

void OpenCLWaveSimulation::computeVertexDisplacement()
{
    clEnqueueAcquireGLObjects(m_queue, 1, &m_clPing, 0, 0, 0);
    clEnqueueAcquireGLObjects(m_queue, 1, &m_clPong, 0, 0, 0);

    if(m_pingpong)
    {
        clSetKernelArg(m_vertexDisplacementKernel, 0, sizeof(cl_mem), (void*)&m_clPing);
        clSetKernelArg(m_vertexDisplacementKernel, 1, sizeof(cl_mem), (void*)&m_clPong);
    }
    else
    {
        clSetKernelArg(m_vertexDisplacementKernel, 0, sizeof(cl_mem), (void*)&m_clPong);
        clSetKernelArg(m_vertexDisplacementKernel, 1, sizeof(cl_mem), (void*)&m_clPing); // #TODO
    }

    clSetKernelArg(m_vertexDisplacementKernel, 2, sizeof(int), &m_gridWidth);
    clSetKernelArg(m_vertexDisplacementKernel, 3, sizeof(float), m_waves.k1());
    clSetKernelArg(m_vertexDisplacementKernel, 4, sizeof(float), m_waves.k2());
    clSetKernelArg(m_vertexDisplacementKernel, 5, sizeof(float), m_waves.k3());

    // compute vertex displacement
    if(clEnqueueNDRangeKernel(m_queue, m_vertexDisplacementKernel, 2, NULL, m_global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Vertex Displacement Kernel Execution failed\n";
    }


    clEnqueueReleaseGLObjects(m_queue, 1, &m_clPing, 0, 0, 0);
    clEnqueueReleaseGLObjects(m_queue, 1, &m_clPong, 0, 0, 0);
    clFinish(m_queue);
}

void OpenCLWaveSimulation::disturbGrid()
{
    clEnqueueAcquireGLObjects(m_queue, 1, &m_clPing, 0, 0, 0);
    clEnqueueAcquireGLObjects(m_queue, 1, &m_clPong, 0, 0, 0);

    if(m_pingpong)
    {
        // computeVertex displacement swapped the buffers
        clSetKernelArg(m_disturbKernel, 0, sizeof(cl_mem), (void*)&m_clPong);
    }
    else
    {
        clSetKernelArg(m_disturbKernel, 0, sizeof(cl_mem), (void*)&m_clPing);
    }

    int i = 5 + rand() % (m_waves.rowCount()-10);
    int j = 5 + rand() % (m_waves.columnCount()-10);
    float r = MathUtils::randF(1.0f, 2.0f);

    // hack
    m_waves.disturb(i, j, r);
    m_waves.update(0);

    clSetKernelArg(m_disturbKernel, 1, sizeof(unsigned int), &i);
    clSetKernelArg(m_disturbKernel, 2, sizeof(unsigned int), &j);
    clSetKernelArg(m_disturbKernel, 3, sizeof(int), &m_gridWidth);
    clSetKernelArg(m_disturbKernel, 4, sizeof(float), &r);


    // disturb grid
    if(clEnqueueNDRangeKernel(m_queue, m_disturbKernel, 2, NULL, m_global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Disturb Grid Kernel Execution failed\n";
    }

    clEnqueueReleaseGLObjects(m_queue, 1, &m_clPing, 0, 0, 0);
    clEnqueueReleaseGLObjects(m_queue, 1, &m_clPong, 0, 0, 0);
    clFinish(m_queue);
}

void OpenCLWaveSimulation::onMouseEvent(int button, int state, int x, int y)
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

void OpenCLWaveSimulation::onKeyboardEvent(unsigned char key, int x, int y)
{
    if(key == 27)
    {
        exit(0);
    }
}

void OpenCLWaveSimulation::onMotionEvent(int x, int y)
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
        float dx = 0.8f * static_cast<float>(x - m_prevX);
        float dy = 0.8f * static_cast<float>(y - m_prevY);

        m_radius += dx - dy;
        m_radius = MathUtils::clamp(m_radius, 1.0f, 500.0f);
    }

    m_prevX = x;
    m_prevY = y;
}