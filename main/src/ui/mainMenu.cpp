#include <Arduino.h>
#include "mainMenu.h"
#include "globals.h"
#include "callbacks.h"

MenuRoot mainMenuRoot;

Menu mainMenu;
Menu circleMenu;
Menu squareMenu;
Menu generalPresetMenu;
Menu cutDesignMenu;
Menu presetMenu;
Menu fileCutMenu;
Menu settingsMenu;

MenuItem circleItems[] = {
    makeAction("Cut!", onStartPresetCut, &circleMenu),
    makeFloat("Radius", &radius, 0, 10000, 0.1, "mm"),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeBack(),
};

MenuItem squareItems[] = {
    makeAction("Cut!", onStartPresetCut, &squareMenu),
    makeFloat("Length", &length, 0, 10000, 0.1, "mm"),
    makeFloat("Width", &width, 0, 10000, 0.1, "mm"),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Rotation", &rotation, 0, 360, 0.1, "deg"),
    makeBack(),
};

MenuItem generalPresetMenuItems[] = {
    makeAction("Cut!", onStartPresetCut, &generalPresetMenu),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeBack(),
};

MenuItem fileCutMenuItems[] = {
    makeAction("Cut!", onStartFileCut, nullptr),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeBool("Draw", &drawGCode),
    makeBack(),
};

MenuItem presetItems[] = {
    makeSubmenu("7", &circleMenu),
    makeSubmenu("4", &squareMenu),
    makeSubmenu("0", &generalPresetMenu),
    makeSubmenu("1", &generalPresetMenu),
    makeSubmenu("2", &generalPresetMenu),
    makeSubmenu("3", &generalPresetMenu),
    makeSubmenu("5", &generalPresetMenu),
    makeSubmenu("6", &generalPresetMenu),
    makeSubmenu("8", &generalPresetMenu),
    makeBack(),
};

MenuItem cutDesignItems[] = {
    makeSubmenu("Preset", &presetMenu),
    makeAction("From file", onStartSelectFile, nullptr),
    makeAction("Speed Run", onStartDummy, nullptr),
    makeBack(),
};

MenuItem settingsItems[] = {
    makeAction("Calibrate", onStartCalibrate, nullptr),
    makeAction("Clear Logs", onStartDummy, nullptr),
    makeBool("Auto Zero X/Y", &autoZeroXY),
    makeBack(),
};

MenuItem mainItems[] = {
    makeSubmenu("Cut Design!", &cutDesignMenu),
    makeSubmenu("Settings", &settingsMenu),
};

void buildMenus(MenuDrawCallback drawCallback) {
    circleMenu.title = "Circle";
    circleMenu.parent = &presetMenu;
    circleMenu.items = circleItems;
    circleMenu.itemCount = sizeof(circleItems) / sizeof(circleItems[0]);
    circleMenu.renderType = MenuItemRenderType::WithValue;

    squareMenu.title = "Square";
    squareMenu.parent = &presetMenu;
    squareMenu.items = squareItems;
    squareMenu.itemCount = sizeof(squareItems) / sizeof(squareItems[0]);
    squareMenu.renderType = MenuItemRenderType::WithValue;

    generalPresetMenu.title = "General Preset";
    generalPresetMenu.parent = &presetMenu;
    generalPresetMenu.items = generalPresetMenuItems;
    generalPresetMenu.itemCount = sizeof(generalPresetMenuItems) / sizeof(generalPresetMenuItems[0]);
    generalPresetMenu.renderType = MenuItemRenderType::WithValue;

    cutDesignMenu.title = "Cut Design";
    cutDesignMenu.parent = &mainMenu;
    cutDesignMenu.items = cutDesignItems;
    cutDesignMenu.itemCount = sizeof(cutDesignItems) / sizeof(cutDesignItems[0]);

    fileCutMenu.title = "Cut File";
    fileCutMenu.parent = &cutDesignMenu;
    fileCutMenu.items = fileCutMenuItems;
    fileCutMenu.itemCount = sizeof(fileCutMenuItems) / sizeof(fileCutMenuItems[0]);
    fileCutMenu.renderType = MenuItemRenderType::WithValue;

    presetMenu.title = "Preset";
    presetMenu.drawCallback = drawCallback;
    presetMenu.parent = &cutDesignMenu;
    presetMenu.items = presetItems;
    presetMenu.itemCount = sizeof(presetItems) / sizeof(presetItems[0]);
    presetMenu.renderType = MenuItemRenderType::Drawing;

    settingsMenu.title = "Settings";
    settingsMenu.parent = &mainMenu;
    settingsMenu.items = settingsItems;
    settingsMenu.itemCount = sizeof(settingsItems) / sizeof(settingsItems[0]);

    mainMenu.title = "";
    mainMenu.parent = nullptr;
    mainMenu.items = mainItems;
    mainMenu.itemCount = sizeof(mainItems) / sizeof(mainItems[0]);
}

void setupMenuSystem(MenuDrawCallback drawCallback) {
  buildMenus(drawCallback);
  mainMenuRoot.top = &mainMenu;
  mainMenuRoot.current = mainMenuRoot.top;
  mainMenuRoot.focusIndex = 0;
  mainMenuRoot.editState.editing = false;
  mainMenuRoot.changed = true;
}