#include "menu.h"
#ifndef mainMenu_h
#define mainMenu_h
using namespace CompassMenu;

// ===== Beispiel-Statusvariablen =====
extern bool ledEnabled;
extern int brightness;
extern float threshold;

void onStartPresetCut(void* ctx);

extern MenuRoot mainMenuRoot;

extern Menu mainMenu;
extern Menu cutDesignMenu;
extern Menu settingsMenu;

extern MenuItem cutDesignItems[];
extern MenuItem settingsItems[];
extern MenuItem mainItems[];

void buildMenus(MenuDrawCallback drawCallback);
void setupMenuSystem(MenuDrawCallback drawCallback);
#endif