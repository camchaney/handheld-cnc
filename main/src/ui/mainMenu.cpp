#include <Arduino.h>
#include "mainMenu.h"
#include "globals.h"
#include "callbacks.h"

MenuRoot mainMenuRoot;

Menu mainMenu;
Menu circleMenu;
Menu rectangleMenu;
Menu lineMenu;
Menu generalPresetMenu;
Menu cutDesignMenu;
Menu presetMenu;
Menu fileCutMenu;
Menu speedRunMenu;
Menu settingsMenu;
Menu persistentSettingsMenu;

MenuItem circleItems[] = {
    makeAction("Cut!", onStartPresetCut, &circleMenu),
    makeFloat("Radius", &radius, 0, 10000, 0.1, "mm"),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeBack(),
};

MenuItem squareItems[] = {
    makeAction("Cut!", onStartPresetCut, &rectangleMenu),
    makeFloat("Length", &length, 0, 10000, 0.1, "mm"),
    makeFloat("Width", &width, 0, 10000, 0.1, "mm"),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Rotation", &rotation, 0, 360, 0.1, "deg"),
    makeBack(),
};

MenuItem lineItems[] = {
    makeAction("Cut!", onStartPresetCut, &lineMenu),
    makeFloat("Deepth", &deepth, 0, 50, 0.1, "mm"),
    makeFloat("Length", &length, 0, 10000, 0.1, "mm"),
    makeFloat("Rotation", &rotation, 0, 360, 0.1, "deg"),
    makeFloat("X Offset", &xOffset, -10000, 10000, 0.1, "mm"),
    makeFloat("Y Offset", &yOffset, -10000, 10000, 0.1, "mm"),
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

MenuItem speedRunMenuItems[] = {
    makeAction("Cut!", onStartSpeedRunCut, nullptr),
    makeFloat("Feed Rate", &feedrate, -10000, 10000, 0.1, "mm"),
    makeBack(),
};

MenuItem presetItems[] = {
    makeSubmenu("7", &circleMenu),
    makeSubmenu("4", &rectangleMenu),
    makeSubmenu("0", &lineMenu),
    makeSubmenu("1", &generalPresetMenu),
    makeSubmenu("2", &generalPresetMenu),
    makeSubmenu("3", &generalPresetMenu),
    makeSubmenu("5", &generalPresetMenu),
    makeSubmenu("6", &generalPresetMenu),
    //TODO makeSubmenu("8", &generalPresetMenu),
    makeBack(),
};

MenuItem cutDesignItems[] = {
    makeSubmenu("Preset", &presetMenu),
    makeAction("From file", onStartSelectFile, nullptr),
    makeSubmenu("Speed Run", &speedRunMenu),
    makeBack(),
};

MenuItem settingsItems[] = {
    makeBool("Auto Zero X/Y", &settings.autoZeroXY),
    makeAction("Calibrate", onStartCalibrate, nullptr),
    makeSubmenu("Stored Settings", &persistentSettingsMenu),
    //FIXME makeAction("Clear Logs", onStartDummy, nullptr),
    makeBack(),
};

MenuItem persistentSettingsMenuItems[] = {
    makeBool("Auto Zero X/Y", &settings.autoZeroXY),
    makeBool("Enable Logging", &settings.enableLogging),
    makeAction("Save Settings", onStartSaveSettings, nullptr),
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

    rectangleMenu.title = "Square";
    rectangleMenu.parent = &presetMenu;
    rectangleMenu.items = squareItems;
    rectangleMenu.itemCount = sizeof(squareItems) / sizeof(squareItems[0]);
    rectangleMenu.renderType = MenuItemRenderType::WithValue;

    lineMenu.title = "Line";
    lineMenu.parent = &presetMenu;
    lineMenu.items = lineItems;
    lineMenu.itemCount = sizeof(lineItems) / sizeof(lineItems[0]);
    lineMenu.renderType = MenuItemRenderType::WithValue;

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

    speedRunMenu.title = "Speed Run";
    speedRunMenu.parent = &cutDesignMenu;
    speedRunMenu.items = speedRunMenuItems;
    speedRunMenu.itemCount = sizeof(speedRunMenuItems) / sizeof(speedRunMenuItems[0]);

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

    persistentSettingsMenu.title = "Persistent Settings";
    persistentSettingsMenu.parent = &settingsMenu;
    persistentSettingsMenu.items = persistentSettingsMenuItems;
    persistentSettingsMenu.itemCount = sizeof(persistentSettingsMenuItems) / sizeof(persistentSettingsMenuItems[0]);

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