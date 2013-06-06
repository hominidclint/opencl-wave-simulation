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
#include "CpuWaves.h"
#include "Chronometer.hpp"

#include <string>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

class WaveApp : public GlutApp
{
public:
    WaveApp(int argc, char** argv, const std::string& appName, int width, int height, int gridWidth, int gridHeight);
    ~WaveApp();

    virtual bool init();
    virtual void render();
    virtual void updateScene(double dt);
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
    glm::mat3 m_worldInvTransposeM;

    // navigation
    float m_theta;
    float m_phi;
    float m_radius;

    int m_prevX;
    int m_prevY;
    int m_mouseBitMask;
    Chronometer m_waveTrigger;

    CPUWaves m_waves;

    // light, material and camera
    glm::vec4 m_materialAmbient;
    glm::vec4 m_materialDiffuse;
    glm::vec4 m_materialSpecular; // w component for shininess

    glm::vec3 m_lightDir;
    glm::vec4 m_lightAmbient;
    glm::vec4 m_lightDiffuse;
    glm::vec4 m_lightSpecular;

    GLuint m_posVBO;
    GLuint m_normalVBO;
    GLuint m_indicesVBO;

    int m_gridWidth;
    int m_gridHeight;
};

#endif // WAVE_APP_H