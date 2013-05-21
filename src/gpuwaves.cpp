// std includes
#include <iostream>
#include <fstream>

// ocl includes
#include <CL/cl.h>
#include <CL/cl_gl.h>

// ogl includes
#include <GL/glew.h>
#include <GL/glut.h>

#ifndef _WIN32
#   include <GL/glxew.h>
#endif

#define WIDTH 800
#define HEIGHT 600

const unsigned int mesh_width = 128;
const unsigned int mesh_height = 128;

cl_platform_id platform;
cl_device_id device;
cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel kernel;
size_t kernelsize;
size_t global[] = {mesh_width, mesh_height};

char* pathname = NULL;
char* source = NULL;

// globals position vertex buffer object
// could pack a struct data into vbo for example (vertex, normal, texcoord) instead of one vbo each
const unsigned int p_vbo_size = mesh_width * mesh_height * 4 * sizeof(float);
GLuint p_vbo;
cl_mem p_vbocl;

const unsigned int c_vbo_size = mesh_width * mesh_height * 4 * sizeof(unsigned char);
GLuint c_vbo;
cl_mem c_vbocl;

// glut globals & callbacks
float anim = 0.0;
int drawMode = GL_LINE_STRIP;
const char* drawStr = "line";
const char* platformString = "notset";

int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float translate_z = -5.0;

void display();
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void initgl(int argc, const char** argv);

// helper
void setTitle()
{
    char title[256];
    sprintf(title, "GL Interop Wrapper: mode %s device %s", drawStr, platformString);
    glutSetWindowTitle(title);
}

int main(int argc, const char** argv)
{
    initgl(argc, argv);

    clGetPlatformIDs(1, &platform, NULL);

    if(argc > 1)
    {
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        platformString = "CPU";
    }
    else
    {
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
        platformString = "GPU";
    }

    // add ocl context must be tied to the ogl context
#   ifdef _WIN32
        HGLRC glCtx = wglGetCurrentContext();
#   else
        GLXContext glCtx = glXGetCurrentContext();
#   endif

    cl_context_properties props[] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)platform,
#   ifdef _WIN32
                                     CL_WGL_HDC_KHR, (intptr_t) wglGetCurrentDC(),
#   else
                                     CL_GLX_DISPLAY_KHR, (intptr_t) glXGetCurrentDisplay();
#   endif
                                     CL_GL_CONTEXT_KHR, (intptr_t) glCtx, 0
    };

    // create context and queue
    context = clCreateContext(props, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, device, 0, NULL);

    // create position VBO
    glGenBuffers(1, &p_vbo);
    // allocates space on the device
    glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
    // init buffer object
    glBufferData(GL_ARRAY_BUFFER, p_vbo_size, 0, GL_DYNAMIC_DRAW);
    // create ocl buffer grom ogl VBO
    p_vbocl = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, p_vbo, NULL);

    // create color VBO
    glGenBuffers(1, &c_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
    glBufferData(GL_ARRAY_BUFFER, c_vbo_size, 0, GL_DYNAMIC_DRAW);
    c_vbocl = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, c_vbo, NULL);

    // load source files
    std::ifstream file("sinewave.cl");
    std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    file.close();

    const char* source = prog.c_str();
    const size_t kernelsize = prog.length() + 1;
    program = clCreateProgramWithSource(context, 1, (const char**)&source, &kernelsize, NULL);

    // build program
    int err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];
        std::cerr << "Error: Failed to build program executable!" << std::endl;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        std::cerr << buffer << std::endl;
        exit(1);
    }

    // create the compute kernel in the program
    kernel = clCreateKernel(program, "sinewave", &err);
    if(!kernel || err != CL_SUCCESS)
    {
        std::cerr << "Error: Failed to create compute kernel!" << std::endl;
        exit(1);
    }

    // set kernel arguments, argument 3 is set in display()
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&p_vbocl);
    clSetKernelArg(kernel, 1, sizeof(unsigned int), &mesh_width);
    clSetKernelArg(kernel, 2, sizeof(unsigned int), &mesh_height);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&c_vbocl);

    setTitle();
    glutMainLoop();
}

void initgl(int argc, const char** argv)
{
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2 - WIDTH/2,
                           glutGet(GLUT_SCREEN_HEIGHT)/2 - HEIGHT/2);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glewInit();

    glClearColor(0.75, 0.75, 0.75, 1.0);

    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1, 100.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display()
{
    // called per frame
    anim += 0.01f;

    // map ogl buffer object for writing from OpenCL
    glFinish();
    clEnqueueAcquireGLObjects(queue, 1, &p_vbocl, 0, 0, 0);
    clEnqueueAcquireGLObjects(queue, 1, &c_vbocl, 0, 0, 0);

    // set arg 3 and queue the kernel
    clSetKernelArg(kernel, 3, sizeof(float), &anim);
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, NULL, 0, 0, 0); // #TODO tweak

    // queue unmap buffer object
    clEnqueueReleaseGLObjects(queue, 1, &c_vbocl, 0, 0, 0);
    clEnqueueReleaseGLObjects(queue, 1, &p_vbocl, 0, 0, 0);
    clFinish(queue); // sync

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render computed data
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);

    // render from the p_vbo
    glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    // enable clors form the c_vbo
    glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
    glEnableClientState(GL_COLOR_ARRAY);

    // draw points, lines or triangles according to the user keyboard input
    switch(drawMode)
    {
    case GL_LINE_STRIP:
        for(int i = 0; i < mesh_width * mesh_height; i += mesh_width)
        {
            glDrawArrays(GL_LINE_STRIP, i, mesh_width);
        }
        break;
    default:
        glDrawArrays(GL_POINTS, 0, mesh_width * mesh_height);
        break;
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glutSwapBuffers();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
    case('q'):
    case(27):
        exit(0);
        break;

    case 'w':
    case 'W':
        switch(drawMode)
        {
        case GL_POINTS: drawMode = GL_LINE_STRIP; drawStr = "line"; break;
        case GL_LINE_STRIP: drawMode = GL_POINTS; drawStr = "points"; break;
        default: drawMode = GL_POINTS; drawStr = "points"; break;
        }
        setTitle();
        glutPostRedisplay();
    }
}

// Mouse event handler for GLUT
void mouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        mouse_buttons |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;
    glutPostRedisplay();
}

// Motion event handler for GLUT
void motion(int x, int y)
{
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if(mouse_buttons & 1)
    {
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    }
    else if(mouse_buttons & 4)
    {
        translate_z += dy * 0.01;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}