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

extern GlutApp* g_app;

extern "C" static void display()
{
    g_app->drawScene();
}

extern "C" static void keyboard(unsigned char key, int x, int y)
{
    g_app->onKeyboardEvent(key, x, y);
}

extern "C" static void mouse(int button, int state, int x, int y)
{
    g_app->onMouseEvent(button, state, x, y);
}

extern "C" static void motion(int x, int y)
{
    g_app->onMotionEvent(x, y);
}
WaveApp::WaveApp(int argc, char** argv, int width, int height)
    : GlutApp(argc, argv, width, height)
{
}

WaveApp::~WaveApp()
{

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

    return true;
}

void WaveApp::drawScene()
{
    updateScene(0); // #TODO

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
        glVertex3f(-0.5, -0.5, 0.0);
        glVertex3f(-0.5, 0.0, 0.0);
        glVertex3f(0.0, 0.5, 0.0);
    glEnd();
    glutSwapBuffers();
}

void WaveApp::updateScene(float dt)
{
    // #TODO
}

