#include "menu.h"
#ifndef cutMenu_h
#define cutMenu_h
using namespace CompassMenu;

extern float feedRate;

extern MenuRoot cutMenuRoot;

extern Menu cutMenu;
extern Menu cutDesignMenu;
extern Menu settingsMenu;

extern MenuItem cutItems[];

void setupCutMenuSystem();
#endif