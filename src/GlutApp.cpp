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

#include "GlutApp.h"

GlutApp::GlutApp(int argc, char** argv, int width, int height)
    : m_width(width),
      m_height(height)
{
    initGlut(argc, argv);
}

GlutApp::~GlutApp()
{

}

void GlutApp::initGlut(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2 - m_width/2,
                           glutGet(GLUT_SCREEN_HEIGHT)/2 - m_height/2);
    glutInitWindowSize(m_width, m_height);
    glutCreateWindow("");
}

float GlutApp::aspectRatio()
{
    return static_cast<float>(m_width) / m_height;
}

int GlutApp::run()
{
    glutMainLoop();
    return 0;
}

bool GlutApp::init()
{
    return true; // #TODO
}
void GlutApp::onResize()
{

}

void GlutApp::onMouseEvent(int button, int state, int x, int y)
{

}

void GlutApp::onKeyboardEvent(unsigned char key, int x, int y)
{

}

void GlutApp::onMotionEvent(int x, int y)
{

}