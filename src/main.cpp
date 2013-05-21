#include <iostream>
#include "WaveApp.h"

// callback prototypes
static void release();

GlutApp* g_app;

int main(int argc, char** argv)
{
    WaveApp app(argc, argv, 800, 600);
    if(!app.init())
    {
        return 0;
    }
    atexit(release);

    return app.run();
}

static void release()
{
    delete g_app;
}