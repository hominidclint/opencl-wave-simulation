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

