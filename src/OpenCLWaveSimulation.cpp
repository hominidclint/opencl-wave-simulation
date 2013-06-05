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

#define VERTEX_SIZE 4

OpenCLWaveSimulation::OpenCLWaveSimulation(int argc, char** argv, const std::string& appName, int width, int height, int gridWidth, int gridHeight)
    : GlutApp(argc, argv, appName, width, height),
      m_gridWidth(gridWidth),
      m_gridHeight(gridHeight),
      m_mouseBitMask(0),
      m_glslProgram(new GLSLProgram),
      m_theta(1.5f * MathUtils::Pi),
      m_phi(0.1f),
      m_radius(600.0f),
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
    m_waveTrigger.start();
    m_fpsChronometer.start();

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
    m_glslProgram->bindAttribLocation(1, "vNormal");
    m_glslProgram->bindAttribLocation(2, "vTangent");
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
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 2048.0f);
    m_worldInvTransposeM = glm::transpose(glm::inverse(glm::mat3(m_modelM)));

    // light, material and camera
    m_materialAmbient  = glm::vec4(0.137f, 0.42f, 0.556f, 1.0f);
    m_materialDiffuse  = glm::vec4(0.137f, 0.42f, 0.556f, 1.0f);
    m_materialSpecular = glm::vec4(0.8f, 0.8f, 0.8f, 96.0f);

    m_lightDir      = glm::vec3(0.0f, -1.0f, 0.1f);
    m_lightAmbient  = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_lightDiffuse  = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_lightSpecular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    m_glslProgram->setUniform("materialAmbient", m_materialAmbient);
    m_glslProgram->setUniform("materialDiffuse", m_materialDiffuse);
    m_glslProgram->setUniform("materialSpecular", m_materialSpecular);
    m_glslProgram->setUniform("lightDir", m_lightDir);
    m_glslProgram->setUniform("lightAmbient", m_lightAmbient);
    m_glslProgram->setUniform("lightDiffuse", m_lightDiffuse);
    m_glslProgram->setUniform("lightSpecular", m_lightSpecular);

    buildWaveGrid();
}

void OpenCLWaveSimulation::buildWaveGrid()
{
    assert(m_gridWidth == m_gridHeight);
    m_waves.init(m_gridWidth, m_gridHeight, 1.0f, 0.03f, 3.25f, 0.4f); // #TODO

    std::cout << "\nScene statistics: \n"
              << "------------------------------------------------\n"
              << "Vertices  | " << m_waves.vertexCount() << "\n"
              << "Triangles | " << m_waves.triangleCount() << "\n"
              << "Grid Size | " << m_gridWidth << "x" << m_gridHeight << "\n\n";

    GLuint vboHandles[4];
    glGenBuffers(4, vboHandles);
    m_positionVBO = vboHandles[0];
    m_normalVBO = vboHandles[1];
    m_tangentVBO = vboHandles[2];
    m_ibo = vboHandles[3];

    // create vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * m_gridWidth*m_gridHeight, 0, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * m_gridWidth*m_gridHeight, 0, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_tangentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTEX_SIZE * m_gridWidth*m_gridHeight, 0, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_waves.triangleCount(), m_waves.getIndices(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vaoWaves);

    glBindVertexArray(m_vaoWaves);
    glEnableVertexAttribArray(0); // vPos
    glEnableVertexAttribArray(1); // vNormal
    glEnableVertexAttribArray(2); // vTangentX
    glBindBuffer(GL_ARRAY_BUFFER, m_positionVBO);
    glVertexAttribPointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glVertexAttribPointer(1, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindBuffer(GL_ARRAY_BUFFER, m_tangentVBO);
    glVertexAttribPointer(2, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindVertexArray(0);
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
    int errCode;
    m_clPositionInteropBuffer = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_positionVBO, &errCode);
    if(errCode != CL_SUCCESS)
    {
        std::cerr << "Failed creating cl_mem position buffer from gl buffer\n";
    }

    m_clNormalInteropBuffer = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_normalVBO, &errCode);
    if(errCode != CL_SUCCESS)
    {
        std::cerr << "Failed creating cl_mem normal buffer from gl buffer\n";
    }

    m_clTangentInteropBuffer = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_tangentVBO, &errCode);
    if(errCode != CL_SUCCESS)
    {
        std::cerr << "Failed creating cl_mem tangent buffer from gl buffer\n";
    }

    m_clPing = clCreateBuffer(m_context,
                              CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                              vboSize,
                              reinterpret_cast<float*>(m_waves.getVertices()),
                              &errCode);
    if(errCode != CL_SUCCESS)
    {
        std::cerr << "Failed creating cl_mem read write buffer\n";
    }

    m_clPong = clCreateBuffer(m_context,
                              CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                              vboSize,
                              reinterpret_cast<float*>(m_waves.getVertices()),
                              &errCode);
    if(errCode != CL_SUCCESS)
    {
        std::cerr << "Failed creating cl_mem read write buffer\n";
    }

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

    m_glGridInitKernel = clCreateKernel(m_program, "initialize_gl_grid", &err);
    if(!m_glGridInitKernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel: initialize_gl_grid!" << std::endl;
        exit(1);
    }
    initGLBuffer();
}

void OpenCLWaveSimulation::initGLBuffer()
{
    clFinish(m_queue);
    glFinish();
    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clPositionInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl position buffer\n";
    }

    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clNormalInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl normal buffer\n";
    }

    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clTangentInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl tangent buffer\n";
    }

    clSetKernelArg(m_glGridInitKernel, 0, sizeof(cl_mem), (void*)&m_clPositionInteropBuffer);
    clSetKernelArg(m_glGridInitKernel, 1, sizeof(cl_mem), (void*)&m_clNormalInteropBuffer);
    clSetKernelArg(m_glGridInitKernel, 2, sizeof(cl_mem), (void*)&m_clTangentInteropBuffer);
    clSetKernelArg(m_glGridInitKernel, 3, sizeof(cl_mem), (void*)&m_clPing);
    clSetKernelArg(m_glGridInitKernel, 4, sizeof(int), &m_gridWidth);

    if(clEnqueueNDRangeKernel(m_queue, m_glGridInitKernel, 2, NULL, m_global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "OpenGL Grid Init Kernel Execution failed\n";
    }


    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clPositionInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl position buffers\n";
    }

    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clNormalInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl normal buffers\n";
    }

    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clTangentInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl tangent buffers\n";
    }
    clFinish(m_queue);
}

void OpenCLWaveSimulation::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 2048.0f);

    glutPostRedisplay();
}

void OpenCLWaveSimulation::render()
{
    checkGLError(__FILE__,__LINE__);

    measurePerformance();
    updateScene(m_fpsChronometer.getPassedTimeSinceStart());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(m_vaoWaves);
    glDrawElements(GL_TRIANGLES, 3 * m_waves.triangleCount(), GL_UNSIGNED_INT, ((GLubyte*)NULL + (0)));

    glutSwapBuffers();    
}

void OpenCLWaveSimulation::updateScene(double dt)
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

    m_glslProgram->setUniform("eyePosW", pos);

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);
    m_worldInvTransposeM = glm::transpose(glm::inverse(glm::mat3(m_modelM)));
    m_glslProgram->setUniform("WorldMatrix", m_modelM);
    m_glslProgram->setUniform("WorldInvTranspose", m_worldInvTransposeM);
    glFinish();

    if(m_waveTrigger.getPassedTimeSinceStart() >= 0.05) // 50ms
    {
        disturbGrid();
        m_waveTrigger.stop();
        m_waveTrigger.start();
    }

    computeVertexDisplacement();
    computeFiniteDifferenceScheme();
}

void OpenCLWaveSimulation::computeVertexDisplacement()
{
    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clPositionInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl position buffer\n";
    }

    if(m_pingpong)
    {
        clSetKernelArg(m_vertexDisplacementKernel, 0, sizeof(cl_mem), (void*)&m_clPing);
        clSetKernelArg(m_vertexDisplacementKernel, 1, sizeof(cl_mem), (void*)&m_clPong);
    }
    else
    {
        clSetKernelArg(m_vertexDisplacementKernel, 0, sizeof(cl_mem), (void*)&m_clPong);
        clSetKernelArg(m_vertexDisplacementKernel, 1, sizeof(cl_mem), (void*)&m_clPing);
    }

    clSetKernelArg(m_vertexDisplacementKernel, 2, sizeof(cl_mem), (void*)&m_clPositionInteropBuffer);
    clSetKernelArg(m_vertexDisplacementKernel, 3, sizeof(int), &m_gridWidth);
    clSetKernelArg(m_vertexDisplacementKernel, 4, sizeof(float), m_waves.k1());
    clSetKernelArg(m_vertexDisplacementKernel, 5, sizeof(float), m_waves.k2());
    clSetKernelArg(m_vertexDisplacementKernel, 6, sizeof(float), m_waves.k3());

    if(clEnqueueNDRangeKernel(m_queue, m_vertexDisplacementKernel, 2, NULL, m_global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Vertex Displacement Kernel Execution failed\n";
    }


    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clPositionInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl position buffers\n";
    }
    clFinish(m_queue);

    // swap buffers
    m_pingpong = !m_pingpong;
}

void OpenCLWaveSimulation::computeFiniteDifferenceScheme()
{
    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clNormalInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl normal buffer\n";
    }

    if(clEnqueueAcquireGLObjects(m_queue, 1, &m_clTangentInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to acquire gl tangent buffer\n";
    }

    if(m_pingpong)
    {
        clSetKernelArg(m_finiteDifferenceSchemeKernel, 0, sizeof(cl_mem), (void*)&m_clPong);
    }
    else
    {
        clSetKernelArg(m_finiteDifferenceSchemeKernel, 0, sizeof(cl_mem), (void*)&m_clPing);
    }

    clSetKernelArg(m_finiteDifferenceSchemeKernel, 1, sizeof(cl_mem), (void*)&m_clNormalInteropBuffer);
    clSetKernelArg(m_finiteDifferenceSchemeKernel, 2, sizeof(cl_mem), (void*)&m_clTangentInteropBuffer);
    clSetKernelArg(m_finiteDifferenceSchemeKernel, 3, sizeof(int), &m_gridWidth);
    clSetKernelArg(m_finiteDifferenceSchemeKernel, 4, sizeof(float), m_waves.spatialStep());

    if(clEnqueueNDRangeKernel(m_queue, m_finiteDifferenceSchemeKernel, 2, NULL, m_global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Finite Difference Scheme Kernel Execution failed\n";
    }

    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clNormalInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl normal buffers\n";
    }

    if(clEnqueueReleaseGLObjects(m_queue, 1, &m_clTangentInteropBuffer, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Failed to release gl tangent buffers\n";
    }
    clFinish(m_queue);
}

void OpenCLWaveSimulation::disturbGrid()
{  
    int i = 5 + rand() % (m_waves.rowCount()-10);
    int j = 5 + rand() % (m_waves.columnCount()-10);
    float r = MathUtils::randF(1.0f, 2.0f);

    if(m_pingpong)
    {
        // computeVertex displacement swapped the buffers before
        clSetKernelArg(m_disturbKernel, 0, sizeof(cl_mem), (void*)&m_clPong);
    }
    else
    {
        clSetKernelArg(m_disturbKernel, 0, sizeof(cl_mem), (void*)&m_clPing);
    }
    
    clSetKernelArg(m_disturbKernel, 1, sizeof(unsigned int), &i);
    clSetKernelArg(m_disturbKernel, 2, sizeof(unsigned int), &j);
    clSetKernelArg(m_disturbKernel, 3, sizeof(int), &m_gridWidth);
    clSetKernelArg(m_disturbKernel, 4, sizeof(float), &r);

    size_t global[] = {1, 1};
    if(clEnqueueNDRangeKernel(m_queue, m_disturbKernel, 2, NULL, global, NULL, 0, 0, 0) != CL_SUCCESS)
    {
        std::cerr << "Disturb Grid Kernel Execution failed\n";
    }
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
    static bool state = true;
    if(key == 27)
    {
        exit(0);
    }
    else if(key == 'w')
    {
        if(state)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        state = !state;
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
        m_radius = MathUtils::clamp(m_radius, 1.0f, 1024.0f);
    }

    m_prevX = x;
    m_prevY = y;
}