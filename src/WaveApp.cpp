// Copyright (c) 2013, Hannes W�rfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
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
#include "MathUtils.h"

#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>

WaveApp::WaveApp(int argc, char** argv, const std::string& appName, int width, int height, int gridWidth, int gridHeight)
    : GlutApp(argc, argv, appName, width, height),
    m_mouseBitMask(0),
    m_glslProgram(new GLSLProgram),
    m_theta(1.5f * MathUtils::Pi),
    m_phi(0.1f * MathUtils::Pi),
    m_radius(600.0f),
    m_prevX(0),
    m_prevY(0),
    m_gridWidth(gridWidth),
    m_gridHeight(gridHeight)
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
    m_waves.init(m_gridWidth, m_gridHeight, 1.0f, 0.03f, 3.25f, 0.4f);

    GLuint vboHandles[3];
    glGenBuffers(3, vboHandles);
    m_posVBO = vboHandles[0];
    m_normalVBO = vboHandles[1];
    m_indicesVBO = vboHandles[2];

    // create vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * m_gridWidth*m_gridHeight, reinterpret_cast<float*>(m_waves.getCurrentWaves()), GL_STREAM_DRAW);

    // create normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * m_gridWidth*m_gridHeight, reinterpret_cast<float*>(m_waves.getCurrentNormals()), GL_STREAM_DRAW);

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
    glEnableVertexAttribArray(1); // vNormal;

    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVBO);
    glBindVertexArray(0);

    delete[] indices;
}

void WaveApp::initScene()
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

void WaveApp::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(glm::degrees(0.25f * MathUtils::Pi), aspectRatio(), 1.0f, 2048.0f);

    glutPostRedisplay();
}

void WaveApp::render()
{
    checkGLError(__FILE__,__LINE__);

    measurePerformance();
    updateScene(m_fpsChronometer.getPassedTimeSinceStart());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_vaoHandle);
    glDrawElements(GL_TRIANGLES, 3 * m_waves.triangleCount(), GL_UNSIGNED_INT, ((GLubyte*)NULL + (0)));

    glutSwapBuffers();
}

void WaveApp::updateScene(double dt)
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

    if(m_waveTrigger.getPassedTimeSinceStart() >= 0.05) // 50ms
    {
        int i = 5 + rand() % (m_waves.rowCount()-10);
        int j = 5 + rand() % (m_waves.columnCount()-10);

        float r = MathUtils::randF(1.0f, 2.0f);

        m_waves.disturb(i, j, r);
        m_waveTrigger.stop();
        m_waveTrigger.start();
    }

    m_waves.update(dt);

    glBindBuffer(GL_ARRAY_BUFFER, m_posVBO);
    glm::vec4* positionData = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    
    for(unsigned int i = 0; i < m_waves.vertexCount(); ++i)
    {
        positionData[i] = m_waves[i];
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glm::vec4* normalData = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    for(unsigned int i = 0; i < m_waves.vertexCount(); ++i)
    {
        normalData[i] = m_waves.normal(i);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
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

void WaveApp::onMotionEvent(int x, int y)
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