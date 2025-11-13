#include "display.h"

static int16_t lastTargetCircleX, lastTargetCircleY;

// Display properties
int16_t tftWidth = screen->width();
int16_t tftHeight = screen->height();
int16_t centerX = tftWidth / 2;
int16_t centerY = tftHeight / 2;

// Runtime UI properties
float rectangleWidth = screen->width() / 2;

unsigned long lastScrollTime = 0;
int scrollPosition = 0;
const int SCROLL_DELAY = 500;    // Time before scrolling starts (ms)
const int SCROLL_SPEED = 150;    // Time between each scroll step (ms)
const int CHARS_TO_DISPLAY = 12; // Max characters that fit on screen with text size 2

float exponentialSkew(float x) {
	if (x > 0.0f) {
		return x + (1.0f/expf(x));
	} else if (x < 0.0f) {
		return -(x + (1.0f/expf(x)));
	}
	return 0.0f;
}

// Convert 8-bit RGB to 16-bit RGB565
uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Gradient function: returns a packed 16-bit RGB565 color
uint16_t getGradientColor(float value, float maxValue) {
	if (value < 0.0f) value = 0.0f;
	if (value > maxValue) value = maxValue;

	const float midValue = 0.5f;
	float ratio = value / maxValue;
	uint8_t r, g, b;

	if (ratio <= midValue) {
		// Green to yellow
		float t = ratio / midValue;
		r = (uint8_t)(255.0f * t);  // 0 -> 255
		g = 255;
		b = 0;
	} else {
		// Yellow to red
		float t = (ratio - midValue) / midValue;
		r = 255;
		g = (uint8_t)(255.0f - (255.0f * t));  // 255 -> 0
		b = 0;
	}

	return rgbTo565(r, g, b);
}

void drawMenu(const char* options[], const int numOptions, int select) {
	screen->fillScreen(BLACK);

	// Set text properties
	screen->setTextSize(2);
	screen->setTextColor(WHITE);
	
	// Calculate vertical spacing
	int16_t yStart = screen->height() / 3;
	int16_t ySpacing = 30;
	
	// Draw each option
	for (int i = 0; i < numOptions; i++) {
		// Highlight selected option
		if (i == select) {
			screen->setTextColor(YELLOW);
		} else {
			screen->setTextColor(WHITE);
		}
		
		// Center text horizontally
		int16_t x1, y1;
		uint16_t w, h;
		screen->getTextBounds(options[i], 0, 0, &x1, &y1, &w, &h);
		int16_t x = (screen->width() - w) / 2;
		
		// Draw option text
		screen->setCursor(x, yStart + (i * ySpacing));
		screen->print(options[i]);
	}
}

void drawShape() {
	int16_t size = min(tftWidth, tftHeight) / 3;
	int16_t dot_size = 2;

	float scale;
	int16_t minY,maxY,minX,maxX,y_quarter,y_3_quarter,x;

	screen->fillScreen(BLACK);

	switch (designPreset) {
		// TODO: draw an accurate representation of the design here
		case 0:
			// line
			screen->drawLine(centerX, centerY-size, centerX, centerY+size, WHITE);
			break;
		case 1:
			// sin
			scale = size / PI;
			for (int y = -size; y <= size; y++) {
				x = (int16_t) (scale*sinf(y/scale));
				screen->drawPixel(centerX+x, centerY+y, WHITE);
			}
			break;
		case 2:
			// zigzag
			minY = centerY - size;
			maxY = centerY + size;
			minX = centerX - size / 2;
			maxX = centerX + size / 2;

			y_quarter = minY + size / 2;
			y_3_quarter = maxY - size / 2;

			screen->drawLine(centerX, minY, maxX, y_quarter, WHITE);
			screen->drawLine(maxX, y_quarter, minX, y_3_quarter, WHITE);
			screen->drawLine(minX, y_3_quarter, centerX, maxY, WHITE);

			break;
		case 3:
			// double line
			screen->drawLine(centerX-size/4, centerY-size, centerX-size/4, centerY+size, WHITE);
			screen->drawLine(centerX+size/4, centerY-size, centerX+size/4, centerY+size, WHITE);
			break;
		case 4:
			// diamond
			screen->drawLine(centerX-size/2, centerY, centerX, centerY+size, WHITE);
			screen->drawLine(centerX, centerY+size, centerX+size/2, centerY, WHITE);
			screen->drawLine(centerX+size/2, centerY, centerX, centerY-size, WHITE);
			screen->drawLine(centerX, centerY-size, centerX-size/2, centerY, WHITE);
			break;
		case 5:
			// square w/ squiggly
			screen->drawLine(centerX-size, centerY, centerX, centerY+size, WHITE);
			screen->drawLine(centerX, centerY+size, centerX+size, centerY, WHITE);
			screen->drawLine(centerX+size, centerY, centerX, centerY-size, WHITE);
			screen->drawLine(centerX, centerY-size, centerX-size, centerY, WHITE);
			scale = size / PI;
			for (int y = -size; y <= size; y++) {
				x = (int16_t) (scale*sinf(y/scale));
				screen->drawPixel(centerX+x, centerY+y, WHITE);
			}
			break;
		case 6:
			// square with Make
			drawCenteredText("M:",2);
			screen->drawLine(centerX-size, centerY, centerX, centerY+size, WHITE);
			screen->drawLine(centerX, centerY+size, centerX+size, centerY, WHITE);
			screen->drawLine(centerX+size, centerY, centerX, centerY-size, WHITE);
			screen->drawLine(centerX, centerY-size, centerX-size, centerY, WHITE);
			break;
		case 7:
			// circle
			screen->drawCircle(centerX, centerY, size, WHITE);
			break;
		case 8:
			// square drill
			screen->drawCircle(centerX-size, centerY-size, dot_size, WHITE);
			screen->drawCircle(centerX+size, centerY-size, dot_size, WHITE);
			screen->drawCircle(centerX-size, centerY+size, dot_size, WHITE);
			screen->drawCircle(centerX+size, centerY+size, dot_size, WHITE);
			screen->drawCircle(centerX, centerY, dot_size, WHITE);
			break;
		// 	// hexagon
		// 	screen->drawLine(centerX, centerY+size, centerX+size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), WHITE);
		// 	screen->drawLine(centerX+size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), centerX+size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), WHITE);
		// 	screen->drawLine(centerX+size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), centerX, centerY-size, WHITE);
		// 	screen->drawLine(centerX, centerY-size, centerX-size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), WHITE);
		// 	screen->drawLine(centerX-size*cosf(M_PI/6), centerY-size*sinf(M_PI/6), centerX-size*cosf(M_PI/6), centerY+size*sinf(M_PI/6), WHITE);
		// 	screen->drawLine(centerX-size*cosf(M_PI/6), centerY+size*sinf(M_PI/6), centerX, centerY+size, WHITE);
		// 	break;
	}
}

void listFiles() {
	screen->fillScreen(BLACK);
	screen->setTextSize(2);

	// Calculate vertical centering
	int totalHeight = displayLines * 20;  				// Total height of all lines (7 lines * 20px)
	int startY = (tftHeight - totalHeight) / 2;  		// Center vertically on screen
	
	// Calculate which files to show to keep selection centered
	int startIndex = max(0, current_file_idx - centerLine);
	
	// Adjust start index if we're near the end of the list
	if (startIndex + displayLines > totalFiles) {
		startIndex = max(0, totalFiles - displayLines);
	}

	// Handle scrolling for selected item
	unsigned long currentTime = millis();
	if (currentTime - lastScrollTime > SCROLL_SPEED) {
		lastScrollTime = currentTime;
		scrollPosition++;
	}
	
	// Draw each visible line
	for (int i = 0; i < displayLines; i++) {
		int fileIndex = startIndex + i;
		if (fileIndex >= totalFiles) break;
		
		// Calculate Y position
		int y = startY + i * 20;  // Assuming 20 pixels per line with text size 2
		screen->setCursor(0, y);

		String displayText = fileList[fileIndex];
		
		// Set color based on selection
		if (fileIndex == current_file_idx) {
			screen->setTextColor(YELLOW);
			screen->print("> ");

			// Handle scrolling for long filename
			if (displayText.length() > CHARS_TO_DISPLAY) {
				// Add spaces at the end before repeating
				displayText = displayText + "    " + displayText;
				int totalScroll = displayText.length();
				int currentPos = scrollPosition % totalScroll;
				displayText = displayText.substring(currentPos, currentPos + CHARS_TO_DISPLAY);
			}
		} else {
			screen->setTextColor(WHITE);
			screen->print("  ");

			// Truncate non-selected long filenames
			if (displayText.length() > CHARS_TO_DISPLAY) {
				displayText = displayText.substring(0, CHARS_TO_DISPLAY - 3) + "...";
			}
		}
		
		// Print file/folder name
		screen->println(displayText);
	}

	// Reset scroll position when selection changes
	static int lastIndex = -1;
	if (lastIndex != current_file_idx) {
		scrollPosition = 0;
		lastScrollTime = currentTime + SCROLL_DELAY; // Add delay before scrolling starts
		lastIndex = current_file_idx;
	}
}

void updateFileList() {
	totalFiles = 0;
	
	// Clear previous list
	for (int i = 0; i < MAX_FILES; i++) {
		fileList[i] = "";
	}

	if (!currentDir) {
		currentDir = sd.open("/");
		if (!currentDir) {
			Serial.println("Failed to open root directory!");
			return;
		}
	}
	
	// Add parent directory entry if not in root
	char currentDirName[256];
	currentDir.getName(currentDirName, sizeof(currentDirName));
	Serial.printf("Current directory: %s\n", currentDirName);
	if (strcmp(currentDirName, "/") != 0) {
		fileList[totalFiles++] = "../";
	}
	
	// First pass: add directories
	currentDir.rewindDirectory();
	while (FsFile entry = currentDir.openNextFile()) {
		if (totalFiles >= MAX_FILES) break;

		char nameBuf[256];
		entry.getName(nameBuf, sizeof(nameBuf));

		// Skip hidden files and folders
		if (nameBuf[0] == '.') {
			entry.close();
			continue;
		}

		if (entry.isDirectory()) {
			fileList[totalFiles++] = String(nameBuf) + "/";
		}
		entry.close();
	}
	
	// Second pass: add files
	currentDir.rewindDirectory();
	while (FsFile entry = currentDir.openNextFile()) {
		if (totalFiles >= MAX_FILES) break;

		char nameBuf[256];
		entry.getName(nameBuf, sizeof(nameBuf));

		// Skip hidden files
		if (nameBuf[0] == '.') {
			entry.close();
			continue;
		}

		if (!entry.isDirectory()) {
			fileList[totalFiles++] = String(nameBuf);
		}
		entry.close();
	}
	
	currentDir.rewindDirectory();
}

void drawCenteredText(const char* text, int size) {
	Serial.print("SCREEN: ");
	Serial.println(text);

	screen->fillScreen(BLACK);

	// screen->setFont(u8g2_font_littlemissloudonbold_tr);
	screen->setTextSize(size);
	screen->setTextColor(WHITE);

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
	int16_t yStart = centerY - totalHeight / 2;

	for (int i = 0; i < lineCount; i++) {
		int16_t x1, y1;
		uint16_t w, h;
		screen->getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);
		int16_t xStart = centerX - w / 2;
		screen->setCursor(xStart, yStart + i * size * 10);
		screen->println(lines[i]);
	}
}

void drawFixedUI() {
	int driftRadius = (screen->width()/2) - 20;

	screen->fillScreen(BLACK);
	
	// Draw bounds rectangle
	screen->drawRect(
		centerX - rectangleWidth/2.0f,
		centerY - rectangleWidth/2.0f,
		rectangleWidth,
		rectangleWidth,
		WHITE
	);

	float maxDriftAngle = TWO_PI * tanhf(maxDrift / maxDrift);
	// TODO: add more markers for fractions of max drift (i.e. 1/10, 2/10, etc.)
	for (int j = 0; j < 6; j++) {
		int x = centerX + (driftRadius-4+j) * sinf(maxDriftAngle);
		int y = centerY - (driftRadius-4+j) * cosf(maxDriftAngle);
		screen->drawPixel(x, y, WHITE);
	}
}

void drawUI(Position desPosition, float progress, uint8_t i) {
	float padding = 6.0f;
	float windowSize = rectangleWidth - 2.0f*padding;
	int progressRadius = (screen->width()/2) - 10;
	int driftRadius = (screen->width()/2) - 20;
	float progressAngle = progress * TWO_PI;
	float driftAngle = TWO_PI * tanhf(distanceTraveled * driftRate / maxDrift);
	float driftColorswitch = 1.2f * maxDrift;
	
	float dx = mapF(desPosition.getX(), -xRange/2.0f, xRange/2.0f, -windowSize/2.0f, windowSize/2.0f);
	float dy = -mapF(desPosition.getY(), -yRange/2.0f, yRange/2.0f, -windowSize/2.0f, windowSize/2.0f);

	switch (i%5) {
		case 0:
			// draw the center target
			screen->drawLine(centerX, centerY-5, centerX, centerY+5, WHITE);
			screen->drawLine(centerX-15, centerY, centerX+15, centerY, WHITE);
			break;
		case 1:
			// clear the old target circle
			screen->drawCircle(lastTargetCircleX, lastTargetCircleY, 5, BLACK);
			break;
		case 2:
			// draw new target circle
			lastTargetCircleX = centerX + dx;
			lastTargetCircleY = centerY + dy;

			if (cutState == NOT_CUT_READY) {
				screen->drawCircle(lastTargetCircleX, lastTargetCircleY, 5, RED);
			} else if (cutState == NOT_USER_READY) {
				screen->drawCircle(lastTargetCircleX, lastTargetCircleY, 5, YELLOW);
			} else {
				screen->drawCircle(lastTargetCircleX, lastTargetCircleY, 5, GREEN);
			}
				
			break;
		case 3:
			// draw progress circle
			for (int j = 0; j < 3; j++) {
				int x = centerX + (progressRadius-1+j) * sinf(progressAngle);
				int y = centerY - (progressRadius-1+j) * cosf(progressAngle);
				screen->drawPixel(x, y, GREEN);
			}
			break;
		case 4:
			// draw the drift
			for (int j = 0; j < 3; j++) {
				int x = centerX + (driftRadius-1+j) * sinf(driftAngle);
				int y = centerY - (driftRadius-1+j) * cosf(driftAngle);
				screen->drawPixel(x, y, getGradientColor(distanceTraveled*driftRate, driftColorswitch));
			}
			break;

	}
}

void updateUI(Position desPosition, float progress) {
	if ((millis()-lastDraw) > 15) {
		iter = (iter + 1)%5;
		// unsigned long now = micros();
		// motorPosX = stepperX.currentPosition()*1.0f/ConvLead;
		drawUI(desPosition, progress, iter);
		// Serial.printf("draw %d took %i us\n", iter, micros()-now);
		lastDraw = millis();
	}
}
