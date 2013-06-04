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

#ifndef GLUT_APP_H
#define GLUT_APP_H

// ogl includes
#include <GL/glew.h>
#include <GL/glut.h>

#ifndef _WIN32
#   include <GL/glxew.h>
#endif

// std
#include <string>

// own
#include "Chronometer.hpp"

class GlutApp
{
public:
    GlutApp(int argc, char** argv, const std::string& appName, int width, int height);
    ~GlutApp();

    float aspectRatio();
    int run();
    std::string queryVersionInformations() const;
    std::string queryExtensionInformations() const;
    int checkGLError(const char* file, int line);

    virtual bool init();
    virtual void onResize(int w, int h) = 0;
    virtual void updateScene(double dt) = 0;
    virtual void render() = 0;

    virtual void onMouseEvent(int button, int state, int x, int y) = 0;
    virtual void onKeyboardEvent(unsigned char key, int x, int y) = 0;
    virtual void onMotionEvent(int x, int y) = 0;

protected:
    void initGlut(int argc, char** argv);
    void measurePerformance();

    int m_width;
    int m_height;

    int m_argc;
    char** m_argv;
    std::string m_appName;

    Chronometer m_fpsChronometer;
};

#endif // GLUT_APP_H