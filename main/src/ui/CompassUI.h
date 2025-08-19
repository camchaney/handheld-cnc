#ifndef COMPASSUI_H
#define COMPASSUI_H
#include <Arduino_GFX_Library.h>
#include "mainMenu.h"
#include "cutMenu.h"
#include "types.h"

typedef enum DisplayState {
    MainMenu,
    Compass,
    CuttingMenu,
} DisplayState;

class CompassUI {
public:
    CompassUI(Arduino_GFX *screen);
    void enable(bool enable = true);
    void poll();
    void up();
    void down();
    void adjust(int increment);
    void enter();
    void back();
    void showCompass(bool show = true);
    void home();
    MenuRoot& getCurrentRoot();
    void updateCompass(Position desPosition, float progress);
    static void render(MenuRoot& root, bool forceRedraw = false);
    static void drawCenteredText(const char* text, int size, uint16_t fgColor);
    DisplayState displayState;

private:
    void drawFixedCompassFrame();
    void drawCompass(Position desPosition, float progress, uint8_t i);
    static void drawPresetMenuItem(MenuItem* item);
    static void renderEditScreen(MenuRoot& root);
    static void renderMenuScreen(MenuRoot& root);
    static Arduino_GFX *screen;
    static int16_t tftWidth;
    static int16_t tftHeight;
    static int16_t centerX;
    static int16_t centerY;
    static float rectangleWidth;
    uint8_t iter = 0;
    int16_t lastTargetCircleX, lastTargetCircleY;
    uint32_t lastAdjust = 0;
    unsigned long lastChanged = 0;
    unsigned long lastDraw = 0;
    float progress = 0.0f;
    bool isEnabled = false;
    bool isShowCompass = false;
};
#endif