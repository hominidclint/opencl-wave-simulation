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

#include "WaveApp.h"
#include "CallbackHandler.h"

#include <iostream>

#define SIZE 512


WaveApp::WaveApp(int argc, char** argv, int width, int height)
    : GlutApp(argc, argv, width, height),
    m_mouseBitMask(0),
    m_rotateX(0.0f),
    m_rotateY(0.0f),
    m_translateZ(-0.0f),
    m_glslProgram(new GLSLProgram)
{
}

WaveApp::~WaveApp()
{
    delete m_glslProgram;
};

bool WaveApp::init()
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

    return true;
}

void WaveApp::buildWaveGrid()
{
    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    m_posVBO = vboHandles[0];
    m_indicesVBO = vboHandles[1];

    glEnableVertexAttribArray(0); // vPos;

    // create vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * SIZE*SIZE, reinterpret_cast<float*>(m_waves.getCurrentWaves()), GL_STREAM_DRAW);

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

    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVBO);
    glBindVertexArray(0);

    delete[] indices;
}

void WaveApp::initScene()
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
    m_glslProgram->bindAttribLocation(1, "vColor");
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
    m_projM = glm::perspective(10.0f, aspectRatio(), 0.1f, 100.0f);

    // #TODO
    m_waves.init(SIZE, SIZE, 1.0f, 0.03f, 3.25f, 0.4f);
    buildWaveGrid();
}

void WaveApp::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(45.0f, aspectRatio(), 1.0f, 100.0f);

    glutPostRedisplay();
}

void WaveApp::drawScene()
{
    checkGLError(__FILE__,__LINE__);
    static float anim = 0;
    anim += 0.01;
    updateScene(anim);
    std::cout << m_translateZ << std::endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set uniforms
    m_modelM = glm::mat4(1.0f);
    m_modelM *= glm::translate(0.0f, 0.0f, m_translateZ);
    m_modelM *= glm::rotate((m_rotateX), glm::vec3(1.0f,0.0f,0.0f));
    m_modelM *= glm::rotate((m_rotateY), glm::vec3(0.0f,1.0f,0.0f));

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);

    glBindVertexArray(m_vaoHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVBO);
    glDrawElements(GL_TRIANGLE_FAN, 3 * m_waves.triangleCount(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0))); 

    glutSwapBuffers();
}

void WaveApp::updateScene(float dt)
{
    int i = 5 + rand() % (m_waves.rowCount()-10);
    int j = 5 + rand() % (m_waves.columnCount()-10);

    float r = 1.0 + ((float)(rand()) / (float)RAND_MAX) * (2.0 - 1.0); // randf(1.0, 2.0)
    m_waves.disturb(i, j, r);

    m_waves.update(dt);
    checkGLError(__FILE__,__LINE__);
    // update the wave vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glm::vec3* mappedData = reinterpret_cast<glm::vec3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    
    for(unsigned int i = 0; i < m_waves.vertexCount(); ++i)
    {
        mappedData[i] = m_waves[i];
        //std::cout << "(" << m_waves[i].x << ", " << m_waves[i].y << ", " << m_waves[i].z << std::endl;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    //std::cout << "----------------------------------------------------------------------------\n";
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
}

void WaveApp::onMouseEvent(int button, int state, int x, int y)
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

void WaveApp::onKeyboardEvent(unsigned char key, int x, int y)
{
    switch(key)
    {
    case('q'):
    case(27):
        exit(0);
        break;
    }
}

void WaveApp::onMotionEvent(int x, int y)
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
        //m_translateZ += dy * 0.01f;
        m_translateZ += dy;
    }

    m_prevX = x;
    m_prevY = y;
}