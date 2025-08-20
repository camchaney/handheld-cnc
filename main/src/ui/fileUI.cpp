#include "fileUI.h"

// Display properties
int16_t tftHeight = screen->height();


unsigned long lastScrollTime = 0;
int scrollPosition = 0;
const int SCROLL_DELAY = 500;    // Time before scrolling starts (ms)
const int SCROLL_SPEED = 150;    // Time between each scroll step (ms)
const int CHARS_TO_DISPLAY = 12; // Max characters that fit on screen with text size 2

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
