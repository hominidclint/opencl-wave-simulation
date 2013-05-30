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

#include "OpenGLOnlyApp.h"
#include "CallbackHandler.h"

#include <iostream>


OpenGLOnlyApp::OpenGLOnlyApp(int argc, char** argv, int width, int height)
    : GlutApp(argc, argv, width, height),
      m_mouseBitMask(0),
      m_rotateX(0.0f),
      m_rotateY(0.0f),
      m_translateZ(-5.0f),
      m_glslProgram(new GLSLProgram)
{
}

OpenGLOnlyApp::~OpenGLOnlyApp()
{
    delete m_glslProgram;
};

bool OpenGLOnlyApp::init()
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

void OpenGLOnlyApp::initScene()
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
    m_projM = glm::perspective(45.0f, aspectRatio(), 0.1f, 100.0f);

    float positionData[] = {
        -1.0f, -1.0f, -1.0f,
        -1.0f, +1.0f, -1.0f,
        +1.0f, +1.0f, -1.0f,
        +1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, +1.0f,
        -1.0f, +1.0f, +1.0f,
        +1.0f, +1.0f, +1.0f,
        +1.0f, -1.0f, +1.0f
    };

    float colorData[] = {
        1.0f, 1.0f, 1.0f, 1.0f, // white
        0.0f, 0.0f, 0.0f, 1.0f, // black
        1.0f, 0.0f, 0.0f, 1.0f, // red
        0.0f, 1.0f, 0.0f, 1.0f, // green
        0.0f, 0.0f, 1.0f, 1.0f, // blue
        1.0f, 1.0f, 0.0f, 1.0f, // yellow
        0.0f, 1.0f, 1.0f, 1.0f, // cyan
        1.0f, 0.0f, 1.0f, 1.0f // magenta
    };

    unsigned int indices[] = {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3, 
        4, 3, 7
    };

    GLuint vboHandles[3];
    glGenBuffers(3, vboHandles);
    GLuint posVBO = vboHandles[0];
    GLuint colorVBO = vboHandles[1];
    GLuint indicesVBO = vboHandles[2];

    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 3, positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 4, colorData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 * 6, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vaoHandle);
    glBindVertexArray(m_vaoHandle);

    glEnableVertexAttribArray(0); // vPos;
    glEnableVertexAttribArray(1); // vColor;

    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
    glBindVertexArray(0);
}

void OpenGLOnlyApp::onResize(int w, int h)
{
    m_width = w;
    m_height = h;
    glViewport(0, 0, m_width, m_height);
    m_projM = glm::perspective(45.0f, aspectRatio(), 0.1f, 100.0f);

    glutPostRedisplay();
}

void OpenGLOnlyApp::drawScene()
{
    checkGLError(__FILE__,__LINE__);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // set uniforms
    m_modelM = glm::mat4(1.0f);
    m_modelM *= glm::translate(0.0f, 0.0f, m_translateZ);
    m_modelM *= glm::rotate((m_rotateX), glm::vec3(1.0f,0.0f,0.0f));
    m_modelM *= glm::rotate((m_rotateY), glm::vec3(0.0f,1.0f,0.0f));

    glm::mat4 mv = m_viewM * m_modelM;
    m_glslProgram->setUniform("MVP", m_projM * mv);

    glBindVertexArray(m_vaoHandle);
    glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0))); // 6 faces
    
    glutSwapBuffers();
}

void OpenGLOnlyApp::updateScene(float dt)
{
}

void OpenGLOnlyApp::onMouseEvent(int button, int state, int x, int y)
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

void OpenGLOnlyApp::onKeyboardEvent(unsigned char key, int x, int y)
{
    switch(key)
    {
    case('q'):
    case(27):
        exit(0);
        break;
    }
}

void OpenGLOnlyApp::onMotionEvent(int x, int y)
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