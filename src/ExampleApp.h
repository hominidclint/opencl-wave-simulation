#ifndef EXAMPLE_APP_H
#define EXAMPLE_APP_H

#include "GlutApp.h"
#include <string>

// ocl includes
#include <CL/cl.h>
#include <CL/cl_gl.h>

class ExampleApp : public GlutApp
{
public:
    ExampleApp(int argc, char** argv, int width, int height, unsigned int meshWidth, unsigned int meshHeight);
    ~ExampleApp();

    virtual bool init();
    virtual void drawScene();
    virtual void updateScene(float dt);

    virtual void onMouseEvent(int button, int state, int x, int y);
    virtual void onKeyboardEvent(unsigned char key, int x, int y);
    virtual void onMotionEvent(int x, int y);

protected:
    void initOCL();

private:
    // rendering and computing
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    cl_kernel m_kernel;
    size_t m_kernelsize;
    size_t m_global[2];

    unsigned int m_meshWidth;
    unsigned int m_meshHeight;

    GLuint m_vbo;
    cl_mem m_vbocl;

    float m_anim;
    int m_drawMode;

    // file loading
    std::string m_fxFilePath;
    std::string m_programSource;

    // navigation
    int m_prevX;
    int m_prevY;
    int m_mouseBitMask;
    float m_rotateX;
    float m_rotateY;
    float m_translateZ;

    // hack
    int m_argc;
};

#endif // EXAMPLE_APP_H