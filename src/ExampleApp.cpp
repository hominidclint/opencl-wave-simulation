#include "ExampleApp.h"
#include <iostream>
#include <fstream>

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

ExampleApp::ExampleApp(int argc, char** argv, int width, int height, unsigned int meshWidth, unsigned int meshHeight)
    : GlutApp(argc, argv, width, height),
      m_meshWidth(meshWidth),
      m_meshHeight(meshHeight),
      m_anim(0.0f),
      m_mouseBitMask(0),
      m_rotateX(0.0f),
      m_rotateY(0.0f),
      m_translateZ(-5.0f),
      m_argc(argc),
      m_drawMode(GL_LINE_STRIP)
{
    m_global[0] = meshWidth;
    m_global[1] = meshHeight;
}

ExampleApp::~ExampleApp()
{

};

bool ExampleApp::init()
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

    glewInit();

    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    glViewport(0, 0, m_width, m_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)m_width / (GLfloat)m_height, 0.1, 100.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    initOCL();

    return true;
}

void ExampleApp::initOCL()
{
    const unsigned int vboSize = m_meshWidth * m_meshHeight * 4 * sizeof(float);
    clGetPlatformIDs(1, &m_platform, NULL);

    if(m_argc > 1)
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 1, &m_device, NULL);
    }
    else
    {
        clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 1, &m_device, NULL);
    }

    // add ocl context must be tied to the ogl context
#   ifdef _WIN32
    HGLRC glCtx = wglGetCurrentContext();
#   else
    GLXContext glCtx = glXGetCurrentContext();
#   endif

    cl_context_properties props[] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)m_platform,
#   ifdef _WIN32
                                     CL_WGL_HDC_KHR, (intptr_t) wglGetCurrentDC(),
#   else
                                     CL_GLX_DISPLAY_KHR, (intptr_t) glXGetCurrentDisplay();
#   endif
                                     CL_GL_CONTEXT_KHR, (intptr_t) glCtx, 0
    };

    // create context and queue
    m_context = clCreateContext(props, 1, &m_device, NULL, NULL, NULL);
    m_queue = clCreateCommandQueue(m_context, m_device, 0, NULL);

    // create position VBO
    glGenBuffers(1, &m_vbo);
    // allocates space on the device
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // init buffer object
    glBufferData(GL_ARRAY_BUFFER, vboSize, 0, GL_DYNAMIC_DRAW);
    // create ocl buffer grom ogl VBO
    m_vbocl = clCreateFromGLBuffer(m_context, CL_MEM_WRITE_ONLY, m_vbo, NULL);

    // load source files
    std::ifstream file("fxWaves.cl");
    std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    file.close();

    const char* source = prog.c_str();
    const size_t kernelsize = prog.length() + 1;
    m_program = clCreateProgramWithSource(m_context, 1, (const char**)&source, &kernelsize, NULL);

    // build program
    int err = clBuildProgram(m_program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        std::cerr << "Error: Failed to build program executable!" << std::endl;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        std::cerr << buffer << std::endl;
        exit(1);
    }

    // create the compute kernel in the program
    m_kernel = clCreateKernel(m_program, "sinewave", &err);
    if(!m_kernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel!" << std::endl;
        exit(1);
    }

    // set kernel arguments
    clSetKernelArg(m_kernel, 0, sizeof(cl_mem), (void*)&m_vbocl);
    clSetKernelArg(m_kernel, 1, sizeof(unsigned int), &m_meshWidth);
    clSetKernelArg(m_kernel, 2, sizeof(unsigned int), &m_meshHeight);
}

void ExampleApp::drawScene()
{
    updateScene(0); // #TODO

    // called per frame
    m_anim += 0.01f;

    // map ogl buffer object for writing from OpenCL
    glFinish();
    clEnqueueAcquireGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);

    // set arg 3 and queue the kernel
    clSetKernelArg(m_kernel, 3, sizeof(float), &m_anim);
    clEnqueueNDRangeKernel(m_queue, m_kernel, 2, NULL, m_global, NULL, 0, 0, 0); // #TODO tweak

    // queue unmap buffer object
    clEnqueueReleaseGLObjects(m_queue, 1, &m_vbocl, 0, 0, 0);
    clFinish(m_queue); // sync

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render computed data
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, m_translateZ);
    glRotatef(m_rotateX, 1.0, 0.0, 0.0);
    glRotatef(m_rotateY, 0.0, 1.0, 0.0);

    // render from the p_vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    // draw points, lines or triangles according to the user keyboard input
    switch(m_drawMode)
    {
    case GL_LINE_STRIP:
        for(unsigned int i = 0; i < m_meshWidth * m_meshHeight; i += m_meshWidth)
        {
            glDrawArrays(GL_LINE_STRIP, i, m_meshWidth);
        }
        break;
    default:
        glDrawArrays(GL_POINTS, 0, m_meshWidth * m_meshHeight);
        break;
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glutSwapBuffers();
    glutPostRedisplay();
}

void ExampleApp::updateScene(float dt)
{
    // #TODO
}

void ExampleApp::onMouseEvent(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        m_mouseBitMask |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        m_mouseBitMask = 0;
    }

    m_prevX = x;
    m_prevY = y;
    glutPostRedisplay();
}

void ExampleApp::onKeyboardEvent(unsigned char key, int x, int y)
{
    switch(key)
    {
    case('q'):
    case(27):
        exit(0);
        break;

    case 'w':
    case 'W':
        switch(m_drawMode)
        {
        case GL_POINTS: m_drawMode = GL_LINE_STRIP; break;
        case GL_LINE_STRIP: m_drawMode = GL_POINTS; break;
        default: m_drawMode = GL_POINTS; break;
        }
        glutPostRedisplay();
    }
}

void ExampleApp::onMotionEvent(int x, int y)
{
    float dx, dy;
    dx = static_cast<float>(x - m_prevX);
    dy = static_cast<float>(y - m_prevY);

    if(m_mouseBitMask & 1)
    {
        m_rotateX += dy * 0.2f;
        m_rotateY += dx * 0.2f;
    }
    else if(m_mouseBitMask & 4)
    {
        m_translateZ += dy * 0.01f;
    }

    m_prevX = x;
    m_prevY = y;
}