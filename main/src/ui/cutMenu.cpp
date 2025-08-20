#include <Arduino.h>
#include "mainMenu.h"
#include "globals.h"
#include "callbacks.h"

MenuRoot cutMenuRoot;
Menu cutMenu;

MenuItem cutItems[] = {
    makeFloat("Feed Rate Boost", &feedrateBoost, 0.1, 5, 0.1, "x"),
    makeAction("Cancel Cut", onStartCancelCut, nullptr),
    makeAction("Rezero X/Y", onStartRezeroXY, nullptr),
    makeAction("Back", onCuttingMenuBack, nullptr),
};

void setupCutMenuSystem() {
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