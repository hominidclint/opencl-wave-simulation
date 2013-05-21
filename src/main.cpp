#include <iostream>
#include "ExampleApp.h"

GlutApp* g_app;

int main(int argc, char** argv)
{
    ExampleApp app(argc, argv, 800, 600, 128, 128);
    if(!app.init())
    {
        return 0;
    }

    return app.run();
}