#ifndef GLUT_APP_H
#define GLUT_APP_H

// ogl includes
#include <GL/glew.h>
#include <GL/glut.h>

#ifndef _WIN32
#   include <GL/glxew.h>
#endif

class GlutApp
{
public:
    GlutApp(int argc, char** argv, int width, int height);
    ~GlutApp();

    float aspectRatio();
    int run();

    virtual bool init();
    virtual void onResize();
    virtual void updateScene(float dt) = 0;
    virtual void drawScene() = 0;

    virtual void onMouseEvent();
    virtual void onKeyboardEvent();

protected:
    void initGlut(int argc, char** argv);

    int m_width;
    int m_height;
};

#endif // GLUT_APP_H