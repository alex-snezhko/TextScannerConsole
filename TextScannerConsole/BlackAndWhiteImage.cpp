#include "pch.h"
#include "BlackAndWhiteImage.h"


BlackAndWhiteImage::BlackAndWhiteImage(const char* fileName)
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

	readHeader(file);
	readPixelData(file);	

	fclose(file);

	// converts image data to either black or white pixels (if the file passed in hasn't already been converted)
	if (!isBlackAndWhite)
	{
		for (int y = 0; y < getHeight(); y++)
		{
			for (int x = 0; x < getWidth(); x++)
			{
				unsigned char b = pixelData[y][3 * x];
				unsigned char g = pixelData[y][3 * x + 1];
				unsigned char r = pixelData[y][3 * x + 2];

				int grayVal = (r + g + b) / 3;

				// only adjusts 'b' value because that is the only thing checked for in positionOccupied()

				// 160 before change ******
				pixelData[y][3 * x] = (grayVal > 100 ? 255 : 0);
			}
		}
	}
}

// reads all of the header data from the bmp and stores in headerData
void BlackAndWhiteImage::readHeader(FILE* file)
{
	// read file header info: 14 bytes
	const int FILE_HEADER_SIZE = 14;
	unsigned char fileHeader[FILE_HEADER_SIZE];
	fread(&fileHeader, sizeof(unsigned char), FILE_HEADER_SIZE, file);

	// find size of info header (DIB header)
	int infoHeaderSize = 0;
	fread(&infoHeaderSize, sizeof(unsigned char), 1, file);
	headerSize = FILE_HEADER_SIZE + infoHeaderSize;

	headerData = new unsigned char[headerSize];
	// copies file header data into full header data
	for (int i = 0; i < FILE_HEADER_SIZE; i++)
	{
		headerData[i] = fileHeader[i];
	}
	headerData[FILE_HEADER_SIZE] = (unsigned char)infoHeaderSize;
	// read info header into full header (+1 / -1 to account for seeking one byte previously to find size of header)
	fread(headerData + FILE_HEADER_SIZE + 1, sizeof(unsigned char), infoHeaderSize - 1, file);
}

// reads all of the pixel data from the bmp and stores in pixelData
void BlackAndWhiteImage::readPixelData(FILE* file)
{
	// amount of bytes required to store one row not accounting for row padding
	int unpaddedRowSize = 3 * getWidth();
	// amount of bytes required to store one row accounting for row padding; formula found on wikipedia 'BMP file format'
	const int paddedRowSize = 4 * ((24 * getWidth() + 31) / 32);
	bytesToRemove = paddedRowSize - unpaddedRowSize;

	// create grid of (height) x (3 * width)
	pixelData = new unsigned char*[getHeight()];
	for (int i = 0; i < getHeight(); i++)
	{
		pixelData[i] = new unsigned char[unpaddedRowSize];
	}

	// read row by row
	for (int i = 0; i < getHeight(); i++)
	{
		fread(pixelData[i], sizeof(unsigned char), unpaddedRowSize, file);
		// discards unnecessary bytes
		fseek(file, (long)bytesToRemove, SEEK_CUR);
	}
}

// returns true if color value for given pixel is black
bool BlackAndWhiteImage::positionOccupied(int x, int y)
{
	if (x < 0 || x >= getWidth() || y < 0 || y >= getHeight())
	{
		return false;
	}

	return pixelData[y][3 * x] == 0;
}

void BlackAndWhiteImage::free()
{
	delete[] headerData;

	for (int i = 0; i < getHeight(); i++)
	{
		delete[] pixelData[i];
	}
	delete[] pixelData;
}

void BlackAndWhiteImage::saveToFile(const char* fileName)
{
	std::ofstream myFile(fileName, std::ios::out | std::ios::trunc);
	for (int i = 0; i < headerSize; i++)
	{
		myFile << headerData[i];
	}
	for (int y = 0; y < getHeight(); y++)
	{
		for (int x = 0; x < getWidth(); x++)
		{
			myFile << pixelData[y][3 * x] << pixelData[y][3 * x] << pixelData[y][3 * x];
		}

		for (int j = 0; j < bytesToRemove; j++)
		{
			myFile << '\0';
		}
	}
	myFile.close();
}