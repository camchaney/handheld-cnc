#include "CompassUI.h"
#include "../math/geometry.h"

Arduino_GFX* CompassUI::screen = nullptr;
int16_t CompassUI::_tftWidth;
int16_t CompassUI::_tftHeight;
int16_t CompassUI::_centerX;
int16_t CompassUI::_centerY;
float CompassUI::_rectangleWidth;

CompassUI::CompassUI(Arduino_GFX *screen, ConfirmLoopCallback confirmLoopCallback) {
    _tftWidth = screen->width();
    _tftHeight = screen->height();
    _centerX = _tftWidth / 2;
    _centerY = _tftHeight / 2;
    _rectangleWidth = screen->width() / 2;
    CompassUI::screen = screen;
    this->_confirmLoopCallback = confirmLoopCallback;

    setupMenuSystem(drawPresetMenuItem);
    setupCutMenuSystem();
}

void CompassUI::enable(bool enable) {
    _isEnabled = enable;
}

void CompassUI::poll() {
    if (!_isEnabled) return;
    if (_isShowCompass) return;

    render(getCurrentRoot());
}

void CompassUI::up() {
    adjust(1);
}

void CompassUI::down() {
    adjust(-1);
}

void CompassUI::adjust(int increment) {
    if (_isConfirming) {
        _confirmSelection = (2 + _confirmSelection + increment) % 2;
        drawConfirmMenu();
        return;
    }

    // Accelerate when the encoder spins rapidly
    uint32_t now = millis();
    uint32_t dt = _lastAdjust ? (now - _lastAdjust) : 0xFFFFFFFF;
    _lastAdjust = now;

    int factor = 1;
    if (dt <= 32)       factor = 100;   // very fast spin
    else if (dt <= 64) factor = 20;   // fast
    else if (dt <= 128) factor = 5;   // moderate

    Serial.print(F("Adjusting by ")); Serial.print(increment); Serial.print(F(" (factor: ")); Serial.print(factor); Serial.println(F(")"));

    int scaled = increment * factor;
    CompassMenu::adjustCurrent(getCurrentRoot(), scaled);
}

void CompassUI::enter() {
    if (_isConfirming) {
        _isConfirming = false;
        Serial.print(F("Confirm selection: "));
        return;
    }

    if (_isShowCompass) {
         displayState = DisplayState::CuttingMenu;
        showCompass(false);
        return;
    }

    CompassMenu::enter(getCurrentRoot());
}

void CompassUI::back() {
    if (_isShowCompass || _isConfirming) return;

    CompassMenu::back(getCurrentRoot());
}

void CompassUI::showCompass(bool show) {
    _isShowCompass = show;
    if (show) 
        drawFixedCompassFrame();
    else
        render(getCurrentRoot(), true);
}

void CompassUI::home() {
    _isEnabled = true;
    _isShowCompass = false;
    displayState = DisplayState::MainMenu;
    state = ZEROED;
    render(getCurrentRoot(), true);
}

bool CompassUI::confirm(const char* message, const char* yesText, const char* noText) {
    _isConfirming = true;
    this->_confirmMessage = message;
	this->_confirmOptions[0] = yesText;
	this->_confirmOptions[1] = noText;
    this->_confirmSelection = 0;

	drawConfirmMenu();

    while (isConfirming()) _confirmLoopCallback();

    return _confirmSelection == 0; // Yes selected
}

MenuRoot& CompassUI::getCurrentRoot() {
    if (displayState == DisplayState::MainMenu)
        return mainMenuRoot;

    return cutMenuRoot;
}

void CompassUI::render(MenuRoot& root, bool forceRedraw) {
    if (!root.changed && !forceRedraw) return;
    root.changed = false;

    if (root.editState.editing) {
        renderEditScreen(root);
    } else {
        renderMenuScreen(root);
    }

    Serial.println();
    Serial.print(F("[ ")); Serial.print(root.current->title); Serial.println(F(" ]"));

    for (uint8_t i = 0; i < root.current->itemCount; ++i) {
        const MenuItem& it = root.current->items[i];
        bool focused = (i == root.focusIndex);

        Serial.print(focused ? F("> ") : F("  "));
        Serial.print(it.label);

        switch (it.type) {
        case MenuItemType::Bool:
            Serial.print(F(": "));
            Serial.println(*(it.data.boolean.value) ? F("ON") : F("OFF"));
            if (focused && root.editState.editing) Serial.print(F(" (edit)"));
            break;
        case MenuItemType::Int:
            Serial.print(F(": "));
            Serial.print(*(it.data.integer.value));
            if (focused && root.editState.editing) Serial.print(F(" (edit)"));
            Serial.println();
            break;
        default:
            Serial.println();
            break;
        }
    }
}

void CompassUI::drawCenteredText(const char* text, int size, uint16_t fgColor) {
    screen->fillScreen(BLACK);
    screen->setTextSize(size);
    screen->setTextColor(fgColor);

    // Split text into lines
    char *lines[10];
    int lineCount = 0;
    char textCopy[256];
    strncpy(textCopy, text, sizeof(textCopy) - 1);
    char *line = strtok(textCopy, "\n");
    
    while (line != NULL && lineCount < 10) {
        lines[lineCount++] = line;
        line = strtok(NULL, "\n");
    }

    int16_t totalHeight = lineCount * size * 10;
    int16_t yStart = _centerY - totalHeight / 2;

    for (int i = 0; i < lineCount; i++) {
        int16_t x1, y1;
        uint16_t w, h;
        screen->getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);
        int16_t xStart = _centerX - w / 2;
        screen->setCursor(xStart, yStart + i * size * 10);
        screen->println(lines[i]);
    }
}

void CompassUI::renderEditScreen(MenuRoot& root) {
    char text2send[50];
    MenuItem item = root.current->items[root.focusIndex];
    switch(item.type) {
        case MenuItemType::Int:
            sprintf(text2send, "Turn to\nset %s\n%d %s", item.label, *(item.data.integer.value), item.data.integer.unitOfMeasure);
            break;
        case MenuItemType::Float:
            sprintf(text2send, "Turn to\nset %s\n%.1f %s", item.label, *(item.data.floating.value), item.data.floating.unitOfMeasure);
            break;
        case MenuItemType::Bool:
            sprintf(text2send, "Turn to toggle\n%s\n%s", item.label, *(item.data.boolean.value) ? "ON" : "OFF");
            break;
        case MenuItemType::Action:
        case MenuItemType::Back:
        case MenuItemType::Submenu:
            break;
    }
    drawCenteredText(text2send, 2, WHITE);
}

void CompassUI::renderMenuScreen(MenuRoot& root) {
    if (root.current->renderType == MenuItemRenderType::Drawing) {
        root.current->drawCallback(&root.current->items[root.focusIndex]);
        return;
    }

    int visibleCount = root.current->itemCount < 3 ?  root.current->itemCount : 3;

    screen->fillScreen(BLACK);

    // Set text properties
    screen->setTextSize(2);
    screen->setTextColor(WHITE);

    // Calculate vertical spacing
    int16_t yStart = screen->height() / 3;
    int16_t ySpacing = 30;

    // calculate output offset
    int focus = root.focusIndex;
    int start = focus - visibleCount / 2;
    if (start < 0) start = 0;
    if (start > root.current->itemCount - visibleCount)
        start = root.current->itemCount - visibleCount;
    if (start < 0) start = 0;

    // Find max label width for ':' alignment (only for WithValue)
    int16_t maxLabelWidth = 96;
    if (root.current->renderType == MenuItemRenderType::WithValue) {
        for (int i = 0; i < visibleCount; i++) {
            int itemIdx = start + i;
            const MenuItem& it = root.current->items[itemIdx];
            int16_t x1, y1; uint16_t w, h;
            screen->getTextBounds(it.label, 0, 0, &x1, &y1, &w, &h);
            if (w > maxLabelWidth) maxLabelWidth = w;
        }
    }

    // Draw each option
    for (int i = 0; i < visibleCount; i++) {
        int itemIdx = start + i;
        const MenuItem& it = root.current->items[itemIdx];
        // Highlight selected option
        screen->setTextColor( itemIdx == root.focusIndex ? YELLOW : WHITE);

        int16_t x, y;
        y = yStart + (i * ySpacing);

        if (root.current->renderType == MenuItemRenderType::WithValue &&
            (it.type == MenuItemType::Bool || it.type == MenuItemType::Int || it.type == MenuItemType::Float)) {
            // Draw label, align ':'
            int16_t x1, y1; uint16_t w, h;
            screen->getTextBounds(it.label, 0, 0, &x1, &y1, &w, &h);
            x = (screen->width() - (maxLabelWidth + 8 + 5*12)) / 2; // 8px for ": ", 5*12px for value
            screen->setCursor(x, y);
            screen->print(it.label);

            // Draw ':' at aligned position
            int16_t colonX = x + maxLabelWidth;
            screen->setCursor(colonX, y);
            screen->print(":");

            // Draw value, right-aligned in 5 chars
            char valueStr[16] = "";
            switch (it.type) {
                case MenuItemType::Bool:
                    snprintf(valueStr, sizeof(valueStr), "%5s", (*(it.data.boolean.value) ? "ON" : "OFF"));
                    break;
                case MenuItemType::Int:
                    snprintf(valueStr, sizeof(valueStr), "%5d", *(it.data.integer.value));
                    break;
                case MenuItemType::Float:
                    snprintf(valueStr, sizeof(valueStr), "%5.1f", *(it.data.floating.value));
                    break;
                default:
                    valueStr[0] = '\0';
                    break;
            }
            int16_t valueX = colonX + 8; // 8px after colon
            // Optionally, measure value width and right-align in 5 chars
            screen->setCursor(valueX, y);
            screen->print(valueStr);
        } else {
            // Center text horizontally (default)
            int16_t x1, y1; uint16_t w, h;
            screen->getTextBounds(it.label, 0, 0, &x1, &y1, &w, &h);
            x = (screen->width() - w) / 2;
            screen->setCursor(x, y);
            screen->print(it.label);
        }
    }
}

void CompassUI::drawPresetMenuItem(MenuItem* item) {
    // Draw a preset menu item with a specific style
    if (item->type == MenuItemType::Back) {
        drawCenteredText("Back", 2, YELLOW);
        return;
    }

    int16_t size = min(_tftWidth, _tftHeight) / 3;
    int16_t dot_size = 2;

    float scale;
    int16_t minY,maxY,minX,maxX,y_quarter,y_3_quarter,x;

    screen->fillScreen(BLACK);
    switch (item->label[0]) {
        // TODO: draw an accurate representation of the design here
        case '0':
            // line
            screen->drawLine(_centerX, _centerY - size, _centerX, _centerY + size, WHITE);
            break;
        case '1':
            // sin
            scale = size / PI;
            for (int y = -size; y <= size; y++) {
                x = (int16_t) (scale * sin(y/scale));
                screen->drawPixel(_centerX + x, _centerY + y, WHITE);
            }
            break;
        case '2':
            // zigzag
            minY = _centerY - size;
            maxY = _centerY + size;
            minX = _centerX - size / 2;
            maxX = _centerX + size / 2;

            y_quarter = minY + size / 2;
            y_3_quarter = maxY - size / 2;

            screen->drawLine(_centerX, minY, maxX, y_quarter, WHITE);
            screen->drawLine(maxX, y_quarter, minX, y_3_quarter, WHITE);
            screen->drawLine(minX, y_3_quarter, _centerX, maxY, WHITE);

            break;
        case '3':
            // double line
            screen->drawLine(_centerX-size/4, _centerY-size, _centerX-size/4, _centerY+size, WHITE);
            screen->drawLine(_centerX+size/4, _centerY-size, _centerX+size/4, _centerY+size, WHITE);
            break;
        case '4':
            // diamond
            screen->drawLine(_centerX-size, _centerY, _centerX, _centerY+size, WHITE);
            screen->drawLine(_centerX, _centerY+size, _centerX+size, _centerY, WHITE);
            screen->drawLine(_centerX+size, _centerY, _centerX, _centerY-size, WHITE);
            screen->drawLine(_centerX, _centerY-size, _centerX-size, _centerY, WHITE);
            break;
        case '5':
            // square w/ squiggly
            screen->drawLine(_centerX-size, _centerY, _centerX, _centerY+size, WHITE);
            screen->drawLine(_centerX, _centerY+size, _centerX+size, _centerY, WHITE);
            screen->drawLine(_centerX+size, _centerY, _centerX, _centerY-size, WHITE);
            screen->drawLine(_centerX, _centerY-size, _centerX-size, _centerY, WHITE);
            scale = size / PI;
            for (int y = -size; y <= size; y++) {
                x = (int16_t) (scale*sin(y/scale));
                screen->drawPixel(_centerX+x, _centerY+y, WHITE);
            }
            break;
        case '6':
            // square with Make
            drawCenteredText("M:", 2, WHITE);
            screen->drawLine(_centerX-size, _centerY, _centerX, _centerY+size, WHITE);
            screen->drawLine(_centerX, _centerY+size, _centerX+size, _centerY, WHITE);
            screen->drawLine(_centerX+size, _centerY, _centerX, _centerY-size, WHITE);
            screen->drawLine(_centerX, _centerY-size, _centerX-size, _centerY, WHITE);
            break;
        case '7':
            // circle
            screen->drawCircle(_centerX, _centerY, size, WHITE);
            break;
        case '8':
            // square drill
            screen->drawCircle(_centerX-size, _centerY-size, dot_size, WHITE);
            screen->drawCircle(_centerX+size, _centerY-size, dot_size, WHITE);
            screen->drawCircle(_centerX-size, _centerY+size, dot_size, WHITE);
            screen->drawCircle(_centerX+size, _centerY+size, dot_size, WHITE);
            screen->drawCircle(_centerX, _centerY, dot_size, WHITE);
            break;
        // 	// hexagon
        // 	screen->drawLine(centerX, centerY+size, centerX+size*cos(M_PI/6), centerY-size*sin(M_PI/6), WHITE);
        // 	screen->drawLine(centerX+size*cos(M_PI/6), centerY-size*sin(M_PI/6), centerX+size*cos(M_PI/6), centerY-size*sin(M_PI/6), WHITE);
        // 	screen->drawLine(centerX+size*cos(M_PI/6), centerY-size*sin(M_PI/6), centerX, centerY-size, WHITE);
        // 	screen->drawLine(centerX, centerY-size, centerX-size*cos(M_PI/6), centerY-size*sin(M_PI/6), WHITE);
        // 	screen->drawLine(centerX-size*cos(M_PI/6), centerY-size*sin(M_PI/6), centerX-size*cos(M_PI/6), centerY+size*sin(M_PI/6), WHITE);
        // 	screen->drawLine(centerX-size*cos(M_PI/6), centerY+size*sin(M_PI/6), centerX, centerY+size, WHITE);
        // 	break;
    }
}

void CompassUI::drawConfirmMenu() {
	screen->fillScreen(BLACK);

	// Set text properties
	screen->setTextSize(2);
	screen->setTextColor(WHITE);
	
	// Calculate vertical spacing
	int16_t yStart = screen->height() / 3;
	int16_t ySpacing = 30;
    
    int16_t x1, y1;
    uint16_t w, h;
    screen->getTextBounds(this->_confirmMessage, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (screen->width() - w) / 2;
    screen->setCursor(x, yStart);
    screen->print(this->_confirmMessage);

	// Draw each option
	for (int i = 0; i < 2; i++) {
		// Highlight selected option
        screen->setTextColor(i == this->_confirmSelection ? YELLOW : WHITE);
		
		// Center text horizontally
		screen->getTextBounds(this->_confirmOptions[i], 0, 0, &x1, &y1, &w, &h);
		int16_t x = (screen->width() - w) / 2;
		
		// Draw option text
		screen->setCursor(x, yStart + ((i + 1) * ySpacing));
		screen->print(this->_confirmOptions[i]);
	}
}

void CompassUI::drawFixedCompassFrame() {
    screen->fillScreen(BLACK);
    
    // Draw bounds rectangle
    screen->drawRect(
        _centerX - _rectangleWidth / 2,
        _centerY - _rectangleWidth / 2,
        _rectangleWidth,
        _rectangleWidth,
        WHITE
    );

    int progressRadius = (screen->width()/2) - 10;
    for (float i = 0.0f; i < this->_progress; i += 0.001f) {
        float progressAngle = i * TWO_PI;
        for (int j = 0; j < 3; j++) {
            int x = _centerX + (progressRadius-1+j) * sinf(progressAngle);
            int y = _centerY - (progressRadius-1+j) * cosf(progressAngle);
            screen->drawPixel(x, y, GREEN);
        }
    }
}

void CompassUI::drawCompass(Position desPosition, float progress, uint8_t i) {
    float padding = 6;
    float windowSize = _rectangleWidth - 2*padding;
    int progressRadius = (screen->width()/2) - 10;
    float progressAngle = progress * TWO_PI;
    
    float dx = mapF(desPosition.getX(), -xRange/2, xRange/2, -windowSize/2, windowSize/2);
    float dy = -mapF(desPosition.getY(), -yRange/2, yRange/2, -windowSize/2, windowSize/2);

    switch (i%4) {
        case 0:
            // draw the center target
            screen->drawLine(_centerX, _centerY-5, _centerX, _centerY+5, WHITE);
            screen->drawLine(_centerX-15, _centerY, _centerX+15, _centerY, WHITE);
            break;
        case 1:
            // clear the old target circle
            screen->drawCircle(_lastTargetCircleX, _lastTargetCircleY, 5, BLACK);
            break;
        case 2:
            // draw new target circle
            _lastTargetCircleX = _centerX + dx;
            _lastTargetCircleY = _centerY + dy;

            if (cutState == NOT_CUT_READY) {
                screen->drawCircle(_lastTargetCircleX, _lastTargetCircleY, 5, RED);
            } else if (cutState == NOT_USER_READY) {
                screen->drawCircle(_lastTargetCircleX, _lastTargetCircleY, 5, YELLOW);
            } else {
                screen->drawCircle(_lastTargetCircleX, _lastTargetCircleY, 5, GREEN);
            }
                
            break;
        case 3:
            for (int i = 0; i < 3; i++) {
                int x = _centerX + (progressRadius-1+i) * sinf(progressAngle);
                int y = _centerY - (progressRadius-1+i) * cosf(progressAngle);
                screen->drawPixel(x, y, GREEN);
            }
            break;
    }
}

void CompassUI::updateCompass(Position desPosition, float progress) {
    this->_progress = progress;
    if (!_isShowCompass) return;

    if ((millis() - lastDraw) > 15) {
        iter = (iter + 1)%4;

        drawCompass(desPosition, progress, iter);
        lastDraw = millis();
    }
}