#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include "../types.h"
#include "../config.h"
#include "../globals.h"
#include "../math/geometry.h"
#include "../actuation/actuator.h"

#define PACKET_START		0xAA
#define PACKET_END			0x55
// Constants for packet types
#define PACKET_SENSORS		0x01
#define PACKET_AUX			0x02
// Header/identifier constants
#define PACKET_HEADER		0xA0
#define PACKET_DESIGN_INFO	0xA1
#define PACKET_PATH			0xA2
#define PACKET_PATH_POINT	0xA3

#define LINE_BUFFER_SIZE	100
#define MAXBUFFER 			220000			// NOTE: what is this used for?

// Header information structure
struct FileHeader {
	uint8_t packetType;
	char firmwareVersion[MAX_STRING_LENGTH];
	char designName[MAX_STRING_LENGTH];
	// uint32_t timestamp;			// UNIX timestamp when logging started
	CalParams calParams[4];			// Calibration parameters
	uint16_t numPoints;
};

// Path point structure
struct PathPoint {
	uint8_t packetType;
	uint16_t pointIndex;
	float x;
	float y;
	float z;
	uint8_t featureType;
};

// Struct definitions for your packets
struct SensorData {
	float dx;
	float dy;
	uint8_t onSurface;
	uint8_t sq;
	uint8_t rawDataSum;
};

struct SensorsPacket {
	uint8_t packetStart;
	uint8_t packetType;
	uint32_t time;				// time since start (us)
	SensorData sensors[ns];		// Data for all 4 sensors
	uint32_t dt;				// sensingTime (us)
	uint8_t packetEnd;
};

struct AuxPacket {
	uint8_t packetStart;
	uint8_t packetType;
	uint32_t time;				// microseconds since start
	RouterPose pose;
	uint16_t currPointIndex;
	Point goal;
	float toolX;
	float toolY;
	float toolZ;
	float desX;
	float desY;
	float desZ;
	int8_t cutState;
	uint8_t packetEnd;
};

// Serial logging functions
void handleSerial();
void outputSerial(Point goal, float toolPos, float desPos);

// Debugging functions
void debugging(Point point1, Position pos);
void debugging(const char* format, ...);
void debugging(const __FlashStringHelper* format, ...);
void stopwatch();

// SD card functions
void updateFileList();
String getParentPath(const char* path);
void handleFileSelection();
void handleSpeedRun();
bool validCommand(const char* gLine);
bool validCoordinate(const char* gLine);
void parseGCodeFile();
void updatePathBuffer();
bool initializeLogFile();
void writeFileHeader(const char* designName, uint16_t numPaths);
// void writePathInfo(uint16_t pathIndex, uint8_t featureType);
void writePathPoint(uint16_t pathIndex, uint16_t pointIndex, Point point);
void writeSensorData(uint32_t time,  SensorData sensorArray[4], uint32_t dt);
void writeAuxData(Point goal, float toolX, float toolY, float toolZ, Position desPos);
void closeSDFile();
void logPath();

#endif
