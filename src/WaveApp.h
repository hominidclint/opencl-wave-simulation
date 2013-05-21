#ifndef WAVE_APP_H
#define WAVE_APP_H

#include "GlutApp.h"

class WaveApp : public GlutApp
{
public:
    WaveApp(int argc, char** argv, int width, int height);
    ~WaveApp();

    virtual bool init();
    virtual void drawScene();
    virtual void updateScene(float dt);

protected:
private:
};

#endif // WAVE_APP_H