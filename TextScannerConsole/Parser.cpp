#include "pch.h"
#include "Parser.h"
#include <string>


Parser* Parser::instance = nullptr;

Parser::Parser() : image(nullptr), letters{ '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' }
{
	for (int i = 0; i < 52; i++)
	{
		char letter = 'A';
		letter += i;
		if (letter >= 91)
		{
			letter += 6;
		}
		letters[i] = ComparisonLetter(letter);
	}
}

Parser* Parser::getInstance()
{
	if (instance == nullptr)
	{
		instance = new Parser();
	}
	return instance;
}

std::string Parser::scanImage(BlackAndWhiteImage image)
{
	this->image = image;

	std::vector<int*> lineBounds = findLineBounds();

	std::string word;
	
	// interprets each line of text
	for (size_t i = 0; i < lineBounds.size(); i++)
	{
		int middleOfLine = (lineBounds.at(i)[1] - lineBounds.at(i)[0]) / 2;
		delete[] lineBounds.at(i);

		// used for adding spaces
		int pixelsAfterLetter = 0;
		bool firstLetterOnLineFound = false;
		int lengthForSpace = 0;

		for (int x = 0; x < image.getWidth(); x++)
		{
			if (image.positionOccupied(x, middleOfLine))
			{
				int* letterBounds = findBoundsOfLetter(x, middleOfLine);

				int letterTopLeftX = letterBounds[0];
				int letterTopLeftY = letterBounds[1];
				int letterBottomRightX = letterBounds[2];
				int letterBottomRightY = letterBounds[3];

				delete[] letterBounds;

				int width = letterBottomRightX - letterTopLeftX;
				int height = letterTopLeftY - letterBottomRightY;			

				char c = interpretToLetter(letterTopLeftX, letterTopLeftY - height, width, height);

				if (c != '\0')
				{
					word += c;

					firstLetterOnLineFound = true;
					pixelsAfterLetter = 0;
					// finds reasonable pixel length required to indicate a space
					lengthForSpace = (width + height) / 2;
				}		

				x = letterBottomRightX;
			}

			if (firstLetterOnLineFound)
			{
				pixelsAfterLetter++;

				if (pixelsAfterLetter == lengthForSpace)
				{
					word += ' ';
				}
			}
		}

		word += '\n';
	}

	image.free();
	return word;
}

// vector of arrays; [0] = bottom y of single line of text, [1] = top y of single line of text
std::vector<int*> Parser::findLineBounds()
{
	// finds bitmap rows with text on them
	std::vector<int> rowsWithText;
	for (int y = 0; y < image.getHeight(); y++)
	{
		for (int x = 0; x < image.getWidth(); x++)
		{
			if (image.positionOccupied(x, y))
			{
				while (image.positionOccupied(x, y))
				{
					rowsWithText.push_back(y);
					y++;
				}
				x = 0;
			}
		}
	}

	if (rowsWithText.empty())
	{
		return std::vector<int*>();
	}
	
	// a list of lower and upper y coordinates of text lines on bitmap
	std::vector<int*> lineBounds;

	int lineBottom = rowsWithText.at(0);
	for (size_t i = 0; i < rowsWithText.size() - 2; i++)
	{	
		if (rowsWithText.at(i + 1) - rowsWithText.at(i) != 1)
		{
			lineBounds.push_back(new int[2]{ lineBottom, rowsWithText.at(i) });
			lineBottom = rowsWithText.at(i + 1);
		}
	}
	lineBounds.push_back(new int[2]{ lineBottom, rowsWithText.at(rowsWithText.size() - 1) });

	// find average line height
	int averageLineHeight = 0;
	for (size_t i = 0; i < lineBounds.size(); i++)
	{
		int lineHeight = lineBounds.at(i)[1] - lineBounds.at(i)[0];
		// squared so that dots of is and js will impact average line heigt less
		averageLineHeight += lineHeight ^ 2;
	}
	averageLineHeight = (int)sqrt((double)averageLineHeight / lineBounds.size());

	// search for dots on is and js and correct line bounds
	for (size_t i = 0; i < lineBounds.size() - 1; i++)
	{
		int nextLineHeight = lineBounds.at(i + 1)[1] - lineBounds.at(i + 1)[0];
		if (nextLineHeight < averageLineHeight / 3)
		{
			lineBounds.at(i)[0] = lineBounds.at(i + 1)[0];
			lineBounds.at(i)[1] = lineBounds.at(i + 1)[1];

			delete[] lineBounds.at(i + 1);
			lineBounds.erase(lineBounds.begin() + i);
		}
	}

	return lineBounds;
}

// returns bounds of letter in a 4-element array: top left x, top left y, bottom right x, bottom right y
// called when a black pixel is found while sweeping rows
// attempts to box each individual letter
//
// NOTE: incorrect for lowercase i and j
int* Parser::findBoundsOfLetter(int x, int y)
{
	int* rectangleData = findRectangleForLetter(x, y, x, y);

	if (rectangleData == nullptr)
	{
		throw ParsingException();
	}

	// attempt to find dots on is and js
	int width = rectangleData[2] - rectangleData[0];
	int height = rectangleData[1] - rectangleData[3];
	if ((double)height / width > 2.5)
	{
		for (int y = 0; y < (int)((double)height / 3); y++)
		{
			for (int x = 0; x < width; x++)
			{
				int xPos = rectangleData[0] + x;
				int yPos = rectangleData[3] + y;
				if (image.positionOccupied(xPos, yPos))
				{
					int* potentialDotBounds = findRectangleForLetter(xPos, yPos, xPos, yPos);

					if (potentialDotBounds != nullptr)
					{
						int dotWidth = potentialDotBounds[2] - potentialDotBounds[0];
						int dotHeight = potentialDotBounds[1] - potentialDotBounds[3];

						if (dotHeight < (int)((double)height / 3) && dotWidth < (int)((double)height / 3))
						{
							// sets new bounds for i or j
							rectangleData[1] = potentialDotBounds[1];			
							rectangleData[0] = potentialDotBounds[0];
							rectangleData[2] = potentialDotBounds[2];
						}
					}
					goto afterDot;
				}
			}
		}
	}

	afterDot:

	// removes extra white pixel on border
	rectangleData[0]++;
	rectangleData[1]--;
	rectangleData[2]--;
	rectangleData[3]++;

	return rectangleData;
}

// recursively enlarges searching field and when complete returns array with 4 elements (order of fields) of rectangle containing proper info
int* Parser::findRectangleForLetter(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
	// Prevents infinite recursion
	if (topLeftX < 0 || topLeftY >= image.getHeight() || bottomRightX >= image.getWidth() || bottomRightY < 0)
	{
		return nullptr;
	}

	bool expandLeft = false;
	bool expandRight = false;
	bool expandUp = false;
	bool expandDown = false;

	// checks if left expansion needed
	for (int y = bottomRightY; y <= topLeftY; y++)
	{
		if (image.positionOccupied(topLeftX, y))
		{
			expandLeft = true;
			break;
		}
	}

	// checks if right expansion needed
	for (int y = bottomRightY; y <= topLeftY; y++)
	{
		if (image.positionOccupied(bottomRightX, y))
		{
			expandRight = true;
			break;
		}
	}

	// checks if upward expansion needed
	for (int x = topLeftX; x <= bottomRightX; x++)
	{
		if (image.positionOccupied(x, topLeftY))
		{
			expandUp = true;
			break;
		}
	}

	// checks if downward expansion needed
	for (int x = topLeftX; x <= bottomRightX; x++)
	{
		if (image.positionOccupied(x, bottomRightY))
		{
			expandDown = true;
			break;
		}
	}

	// correct rectangle
	if (!(expandRight || expandLeft || expandUp || expandDown))
	{
		return new int[4]{ topLeftX, topLeftY, bottomRightX, bottomRightY };
	}

	// expand rectangle as needed and search again
	return findRectangleForLetter(
		topLeftX + (expandLeft ? -1 : 0),
		topLeftY + (expandUp ? 1 : 0),
		bottomRightX + (expandRight ? 1 : 0),
		bottomRightY + (expandDown ? -1 : 0)
	);
}

// attempts to parse a potential letter on the bitmap into a char
char Parser::interpretToLetter(int topLeftX, int topLeftY, int width, int height)
{
	double greatestSimilarity = 0;
	char mostSimilarLetter = '\0';

	for (int i = 0; i < 52; i++)
	{
		double similarity = findSimilarity(i, topLeftX, topLeftY, width, height);

		if (similarity > greatestSimilarity)
		{
			greatestSimilarity = similarity;
			mostSimilarLetter = letters[i].getLetter();
		}
	}

	return mostSimilarLetter;
}

// compares found letter to ComparisonLetters; returns percentage of matching pixels
double Parser::findSimilarity(int index, int origX, int origY, int width, int height)
{
	int pixelMatches = 0;
	const int area = width * height;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			double letterXDouble = x * ((double)letters[index].getWidth() / width);
			double letterYDouble = y * ((double)letters[index].getHeight() / height);

			int letterX = (int)(round(letterXDouble));
			int letterY = (int)(round(letterYDouble));

			if (image.positionOccupied(origX + x, origY + y) == letters[index].positionOccupied(letterX, letterY))
			{
				pixelMatches++;
			}
		}
	}

	return (double)pixelMatches / area;
}
