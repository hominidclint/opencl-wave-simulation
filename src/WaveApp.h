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

#ifndef WAVE_APP_H
#define WAVE_APP_H

#include "GlutApp.h"
#include "GLSLProgram.h"
#include "GridGenerator.h"
#include "CpuWaves.h"

#include <string>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class WaveApp : public GlutApp
{
public:
    struct Vertex // for interleaved arrays
    {
        glm::vec3 position; // strided offset 0
        glm::vec3 normal; // strided offset 12 (3 * 4)
        glm::vec4 color;
        //glm::vec2 uv; // strided offset 24 (6 * 4)
    };

    WaveApp(int argc, char** argv, int width, int height);
    ~WaveApp();

    virtual bool init();
    virtual void render();
    virtual void updateScene(float dt);
    virtual void onResize(int w, int h);

    virtual void onMouseEvent(int button, int state, int x, int y);
    virtual void onKeyboardEvent(unsigned char key, int x, int y);
    virtual void onMotionEvent(int x, int y);

protected:
    void initScene();
    void buildWaveGrid();

private:
    GLSLProgram* m_glslProgram;
    GLuint m_vaoHandle;

    glm::mat4 m_modelM;
    glm::mat4 m_viewM;
    glm::mat4 m_projM;

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
    GLuint m_normalVBO;
    GLuint m_indicesVBO;
};

#endif // WAVE_APP_H