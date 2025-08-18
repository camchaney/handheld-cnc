#include <Arduino.h>
#include "mainMenu.h"
#include "globals.h"
#include "callbacks.h"

MenuRoot cutMenuRoot;
Menu cutMenu;
Menu stopCutMenu;


MenuItem cutItems[] = {
    makeFloat("Feed Rate Boost", &feedrateBoost, 0.1, 5, 0.1, "x"),
    makeSubmenu("Stop", &stopCutMenu),
    makeSubmenu("Rezero X/Y", &stopCutMenu),
    makeAction("Back", onCuttingMenuBack, nullptr),
};

MenuItem stopCutItems[] = {
    makeAction("Yes", onStartCalibrate, nullptr),
    makeBack("No"),
};

void setupCutMenuSystem() {
    stopCutMenu.title = "Stop Cutting";
    stopCutMenu.parent = &cutMenu;
    stopCutMenu.items = stopCutItems;
    stopCutMenu.itemCount = sizeof(stopCutItems) / sizeof(stopCutItems[0]);

    cutMenu.title = "";
    cutMenu.parent = nullptr;
    cutMenu.items = cutItems;
    cutMenu.itemCount = sizeof(cutItems) / sizeof(cutItems[0]);

    cutMenuRoot.top = &cutMenu;
    cutMenuRoot.current = cutMenuRoot.top;
    cutMenuRoot.focusIndex = 0;
    cutMenuRoot.editState.editing = false;
    cutMenuRoot.changed = true;
}