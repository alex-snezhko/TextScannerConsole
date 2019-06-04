#pragma once
#include <fstream>

class BlackAndWhiteImage
{
	int headerSize;
	int bytesToRemove;
	unsigned char* headerData;
	// pixel data of file (each row in bmp gets its own row in 2D array)
	unsigned char** pixelData;

	void readHeader(FILE* file);
	void readPixelData(FILE* file);

protected:
	void loadBitmap(const char* fileName, bool isBlackAndWhite = false);

public:
	BlackAndWhiteImage(const char* fileName);

	int getWidth() { return *(int*)&headerData[18]; }
	int getHeight() { return *(int*)&headerData[22]; }
	bool positionOccupied(int x, int y);
	void saveToFile(const char* fileName);
	void free();
};

