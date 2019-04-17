#include "pch.h"
#include "Parser.h"
#include <string>

// rectangle bound indexes for callers of findRectangleForLetter()
constexpr int LEFT_X = 0;
constexpr int TOP_Y = 1;
constexpr int RIGHT_X = 2;
constexpr int BOTTOM_Y = 3;

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
		int middleOfLine = (lineBounds.at(i)[1] + lineBounds.at(i)[0]) / 2;
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

				int width = letterBounds[RIGHT_X] - letterBounds[LEFT_X];
				int height = letterBounds[TOP_Y] - letterBounds[BOTTOM_Y];		

				char c = interpretToLetter(letterBounds[LEFT_X], letterBounds[BOTTOM_Y], width, height);

				if (c != '\0')
				{
					word += c;

					firstLetterOnLineFound = true;
					pixelsAfterLetter = 0;
					// finds reasonable pixel length required to indicate a space
					lengthForSpace = (width + height) / 2;
				}		

				x = letterBounds[RIGHT_X];

				delete[] letterBounds;
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
		averageLineHeight += (int)pow(lineHeight, 2);
	}
	averageLineHeight = (int)sqrt((double)averageLineHeight / lineBounds.size());



	// search for dots on is and js and correct line bounds
	for (size_t i = 0; i < lineBounds.size() - 1; i++)
	{
		int nextLineHeight = lineBounds.at(i + 1)[1] - lineBounds.at(i + 1)[0];
		if (nextLineHeight < averageLineHeight / 3)
		{
			lineBounds.at(i)[1] = lineBounds.at(i + 1)[1];

			delete[] lineBounds.at(i + 1);
			lineBounds.erase(lineBounds.begin() + i + 1);
			i--;
		}
	}

	return lineBounds;
}

// returns bounds of letter in a 4-element array: top left x, top left y, bottom right x, bottom right y
// called when a black pixel is found while sweeping rows
// attempts to box each individual letter
int* Parser::findBoundsOfLetter(int x, int y)
{
	int* letterRectangle = findRectangleForLetter(x, y, x, y);

	// attempt to find dots on is and js
	int width = letterRectangle[RIGHT_X] - letterRectangle[LEFT_X];
	int height = letterRectangle[TOP_Y] - letterRectangle[BOTTOM_Y];
	if ((double)height / width > 2.5)
	{
		for (int yOffset = 0; yOffset < (int)((double)height / 3); yOffset++)
		{
			for (int xOffset = 0; xOffset < width; xOffset++)
			{
				// searches directly above the shape to find dots
				int xPos = letterRectangle[LEFT_X] + xOffset;
				int yPos = letterRectangle[TOP_Y] + yOffset;
				if (image.positionOccupied(xPos, yPos))
				{
					int* dotBounds = findRectangleForLetter(xPos, yPos, xPos, yPos);

					if (dotBounds != nullptr)
					{
						int dotWidth = dotBounds[RIGHT_X] - dotBounds[LEFT_X];
						int dotHeight = dotBounds[TOP_Y] - dotBounds[BOTTOM_Y];

						if (dotHeight < (int)((double)height / 3) && dotWidth < (int)((double)height / 3))
						{
							// sets new bounds for i or j
							letterRectangle[TOP_Y] = dotBounds[TOP_Y];			
							letterRectangle[LEFT_X] = (int)fmin(letterRectangle[LEFT_X], dotBounds[LEFT_X]);
							letterRectangle[RIGHT_X] = (int)fmax(letterRectangle[RIGHT_X], dotBounds[RIGHT_X]);
						}
					}
					goto afterDot;
				}
			}
		}
	}

	afterDot:

	// removes extra white pixel on border
	letterRectangle[LEFT_X]++;
	letterRectangle[TOP_Y]--;
	letterRectangle[RIGHT_X]--;
	letterRectangle[BOTTOM_Y]++;

	return letterRectangle;
}

// recursively enlarges searching field and when complete returns array with 4 elements (order of fields) of rectangle containing proper info
int* Parser::findRectangleForLetter(int leftX, int topY, int rightX, int bottomY)
{
	// Prevents infinite recursion
	if (leftX < 0 || topY >= image.getHeight() || rightX >= image.getWidth() || bottomY < 0)
	{
		throw ParsingException();
	}

	bool expandLeft = false;
	bool expandRight = false;
	bool expandUp = false;
	bool expandDown = false;

	// checks if left expansion needed
	for (int y = bottomY; y <= topY; y++)
	{
		if (image.positionOccupied(leftX, y))
		{
			expandLeft = true;
			break;
		}
	}

	// checks if right expansion needed
	for (int y = bottomY; y <= topY; y++)
	{
		if (image.positionOccupied(rightX, y))
		{
			expandRight = true;
			break;
		}
	}

	// checks if upward expansion needed
	for (int x = leftX; x <= rightX; x++)
	{
		if (image.positionOccupied(x, topY))
		{
			expandUp = true;
			break;
		}
	}

	// checks if downward expansion needed
	for (int x = leftX; x <= rightX; x++)
	{
		if (image.positionOccupied(x, bottomY))
		{
			expandDown = true;
			break;
		}
	}

	// correct rectangle
	if (!(expandRight || expandLeft || expandUp || expandDown))
	{
		return new int[4]{ leftX, topY, rightX, bottomY };
	}

	// expand rectangle as needed and search again
	return findRectangleForLetter(
		leftX + (expandLeft ? -1 : 0),
		topY + (expandUp ? 1 : 0),
		rightX + (expandRight ? 1 : 0),
		bottomY + (expandDown ? -1 : 0)
	);
}

// attempts to parse a potential letter on the bitmap into a char by comparing to ComparisonLetters in the 'letters' field
char Parser::interpretToLetter(int leftX, int bottomY, int width, int height)
{
	double greatestSimilarityPercentage = 0;
	char mostSimilarLetter = '\0';

	for (int i = 0; i < 52; i++)
	{
		double similarity = findPercentSimilar(i, leftX, bottomY, width, height);

		if (similarity > greatestSimilarityPercentage)
		{
			greatestSimilarityPercentage = similarity;
			mostSimilarLetter = letters[i].getLetter();
		}
	}

	return mostSimilarLetter;
}

// compares found letter to ComparisonLetter at given index in 'letters' field; returns percentage of matching pixels
double Parser::findPercentSimilar(int lettersIndex, int leftX, int bottomY, int width, int height)
{
	int pixelMatches = 0;
	const int area = width * height;
	for (int yOffset = 0; yOffset < height; yOffset++)
	{
		for (int xOffset = 0; xOffset < width; xOffset++)
		{
			// finds which pixel in the ComparisonLetter to compare to
			double letterXDouble = xOffset * ((double)letters[lettersIndex].getWidth() / width);
			double letterYDouble = yOffset * ((double)letters[lettersIndex].getHeight() / height);

			int letterX = (int)(round(letterXDouble));
			int letterY = (int)(round(letterYDouble));

			if (image.positionOccupied(leftX + xOffset, bottomY + yOffset) == letters[lettersIndex].positionOccupied(letterX, letterY))
			{
				pixelMatches++;
			}
		}
	}

	return (double)pixelMatches / area;
}
