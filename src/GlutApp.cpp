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
#include <sstream>
#include <iostream>
#include <sstream>

GlutApp::GlutApp(int argc, char** argv, const std::string& appName, int width, int height)
    : m_argc(argc),
      m_argv(argv),
      m_appName(appName),
      m_width(width),
      m_height(height)
{
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
    initGlut(m_argc, m_argv);
    return true;
}

std::string GlutApp::queryVersionInformations() const
{
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    std::stringstream sstream;
    sstream << "GL Vendor: " << vendor << "\n"
            << "GL Renderer: " << renderer << "\n"
            << "GL Version (string): " << version << "\n"
            << "GL Version (integer): " << major << "." << minor << "\n"
            << "GLSL Version: " << glslVersion << "\n";

    return sstream.str();
}

std::string GlutApp::queryExtensionInformations() const
{
    GLint nExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);

    std::stringstream sstream;
    for(int i = 0; i < nExtensions; ++i)
    {
        sstream << glGetStringi(GL_EXTENSIONS, i) << "\n";
    }

    return sstream.str();
}

int GlutApp::checkGLError(const char* file, int line)
{
    GLenum glError;
    int returnValue = 0;

    glError = glGetError();
    while(glError != GL_NO_ERROR)
    {
        std::string errorString;

        if(glError == GL_INVALID_ENUM)
        {
            errorString = "GL_INVALID_ENUM";
        }
        else if(glError == GL_INVALID_VALUE)
        {
            errorString = "GL_INVALID_VALUE";
        }
        else if(glError == GL_INVALID_OPERATION)
        {
            errorString = "GL_INVALID_OPERATION";
        }
        else if(glError == GL_INVALID_FRAMEBUFFER_OPERATION)
        {
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
        }
        else if(glError == GL_STACK_UNDERFLOW)
        {
            errorString = "GL_STACK_UNDERFLOW";
        }
        else if(glError == GL_STACK_OVERFLOW)
        {
            errorString = "GL_STACK_OVERFLOW";
        }
        else
        {
            errorString = "UNKNOWN";
        }
        std::cout << "glError in file " << file << " line " << line
                  << " "  << errorString << " " << gluErrorString(glError)
                  << std::endl;

        returnValue = 1;
        glError = glGetError();
    }
    return returnValue;
}

void GlutApp::measurePerformance()
{
    static int frameCounter = 0;
    static double elapsedTime = 0.0f;

    ++frameCounter;

    if((m_fpsChronometer.getPassedTimeSinceStart() - elapsedTime) >= 1.0)
    {
        double fps = static_cast<double>(frameCounter);
        double millisecsPerFrame = 1000.0 / fps;

        std::stringstream sstream;
        sstream.precision(4);
        sstream << m_appName << " | fps: " << fps << " | Time Per Frame: " << millisecsPerFrame << " (ms)";
        
        glutSetWindowTitle(sstream.str().c_str());

        elapsedTime += 1.0;
        frameCounter = 0;
    }
}