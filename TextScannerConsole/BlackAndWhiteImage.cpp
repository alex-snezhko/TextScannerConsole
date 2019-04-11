#include "pch.h"
#include "BlackAndWhiteImage.h"
#include <fstream>


// converts image data to either black or white pixels
void BlackAndWhiteImage::toBlackAndWhite()
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			unsigned char* b = &fileData[y][3 * x];
			unsigned char* g = &fileData[y][3 * x + 1];
			unsigned char* r = &fileData[y][3 * x + 2];

			int grayVal = (*r + *g + *b) / 3;
			if (grayVal > 180)
			{
				*r = 255;
				*g = 255;
				*b = 255;
			}
			else
			{
				*r = 0;
				*g = 0;
				*b = 0;
			}
		}
	}
}

BlackAndWhiteImage::BlackAndWhiteImage(const char* fileName) : width(0), height(0), size(0)
{
	if (fileName != nullptr)
	{
		loadBitmap(fileName);
	}
}

void BlackAndWhiteImage::loadBitmap(const char* fileName, bool isBlackAndWhite)
{
	FILE* file;

	fopen_s(&file, fileName, "rb");

	if (file == nullptr)
	{
		return;
	}

	// read bmp header
	fread(headerData, sizeof(unsigned char), HEADER_SIZE, file);


	// extract image height and width from header
	width = *(int*)&headerData[18];
	height = *(int*)&headerData[22];

	// amount of bytes required to store one row not accounting for row padding
	int unpaddedRowSize = 3 * width;
	// amount of bytes required to store one row accounting for row padding; formula found on wikipedia 'BMP file format'
	const int paddedRowSize = 4 * ((24 * width + 31) / 32);

	bytesToRemove = paddedRowSize - unpaddedRowSize;
	size = paddedRowSize * height;

	// create grid of (height) x (3 * width)
	fileData = new unsigned char*[height];
	for (int i = 0; i < height; i++)
	{
		fileData[i] = new unsigned char[unpaddedRowSize];
	}

	// read row by row
	for (int i = 0; i < height; i++)
	{
		fread(fileData[i], sizeof(unsigned char), unpaddedRowSize, file);
		// discards unnecessary bytes
		fseek(file, (long)bytesToRemove, SEEK_CUR);
	}

	fclose(file);

	if (!isBlackAndWhite)
	{
		toBlackAndWhite();
	}
}

// returns true if color value for given pixel is black
bool BlackAndWhiteImage::positionOccupied(int x, int y)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return false;
	}

	return fileData[y][3 * x] == 0;
}

void BlackAndWhiteImage::free()
{
	for (int i = 0; i < height; i++)
	{
		delete[] fileData[i];
	}
	delete[] fileData;
}

/*void BlackAndWhiteImage::saveToFile(const char* fileName)
{
	std::string name = "BWLetters/BW";
	name.append(fileName);

	std::ofstream myFile(name.c_str(), std::ios::out | std::ios::trunc);
	for (int i = 0; i < HEADER_SIZE; i++)
	{
		myFile << headerData[i];
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < 3 * width + bytesToRemove; j++)
		{
			myFile << fileData[i][j];
		}
	}
	myFile.close();
}*/