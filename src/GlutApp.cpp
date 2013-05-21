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
    glutCreateWindow("Simple Wave Simulation for Software Profiling Seminar");
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

void GlutApp::onMouseEvent()
{

}

void GlutApp::onKeyboardEvent()
{

}