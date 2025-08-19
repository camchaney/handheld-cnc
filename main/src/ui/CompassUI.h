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

  typedef void (*ConfirmLoopCallback)();

class CompassUI {
public:
    CompassUI(Arduino_GFX *screen, ConfirmLoopCallback confirmLoopCallback);
    void enable(bool enable = true);
    void poll();
    void up();
    void down();
    void adjust(int increment);
    void enter();
    void back();
    void showCompass(bool show = true);
    void home();
    bool confirm(const char* message, const char* yesText = "Yes", const char* noText = "No");
    bool isConfirming() const { return _isConfirming; }
    MenuRoot& getCurrentRoot();
    void updateCompass(Position desPosition, float progress);
    static void render(MenuRoot& root, bool forceRedraw = false);
    static void drawCenteredText(const char* text, int size, uint16_t fgColor);
    DisplayState displayState;

private:
    void drawFixedCompassFrame();
    void drawCompass(Position desPosition, float progress, uint8_t i);
    void drawConfirmMenu();
    static void drawPresetMenuItem(MenuItem* item);
    static void renderEditScreen(MenuRoot& root);
    static void renderMenuScreen(MenuRoot& root);
    static Arduino_GFX *screen;
    static int16_t _tftWidth;
    static int16_t _tftHeight;
    static int16_t _centerX;
    static int16_t _centerY;
    static float _rectangleWidth;
    ConfirmLoopCallback _confirmLoopCallback;
    const char* _confirmOptions[2];
    const char* _confirmMessage;
    uint8_t _iter = 0;
    uint8_t _confirmSelection = 0;
    int16_t _lastTargetCircleX, _lastTargetCircleY;
    uint32_t _lastAdjust = 0;
    unsigned long lastChanged = 0;
    unsigned long lastDraw = 0;
    float _progress = 0.0f;
    bool _isEnabled = false;
    bool _isShowCompass = false;
    bool _isConfirming = false;
};
#endif