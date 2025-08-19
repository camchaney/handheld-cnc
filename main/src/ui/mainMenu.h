#include "menu.h"
#ifndef mainMenu_h
#define mainMenu_h
using namespace CompassMenu;

extern MenuRoot mainMenuRoot;
extern Menu mainMenu;
extern Menu fileCutMenu;

void buildMenus(MenuDrawCallback drawCallback);
void setupMenuSystem(MenuDrawCallback drawCallback);
#endif