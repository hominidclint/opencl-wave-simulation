#include "WaveApp.h"

extern GlutApp* g_app;

extern "C" void display()
{
    g_app->drawScene();
}

extern "C" void keyboard(unsigned char key, int x, int y)
{
    g_app->onKeyboardEvent(); // #TODO
}

extern "C" void mouse(int button, int state, int x, int y)
{
    g_app->onMouseEvent(); // #TODO
}

extern "C" void motion(int x, int y)
{
    g_app->onMouseEvent(); // #TODO
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

