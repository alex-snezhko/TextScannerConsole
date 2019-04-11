#pragma once
class BlackAndWhiteImage
{
	static const int HEADER_SIZE = 54;
	int width;
	int height;
	int size;
	int bytesToRemove;
	unsigned char headerData[HEADER_SIZE];
	unsigned char** fileData;

	void toBlackAndWhite();

protected:
	void loadBitmap(const char* fileName, bool isBlackAndWhite = false);

public:
	BlackAndWhiteImage(const char* fileName);

	int getWidth() { return width; }
	int getHeight() { return height; }
	//int getSize() { return size; }
	bool positionOccupied(int x, int y);
	//void saveToFile(const char* fileName);
	void free();
};

