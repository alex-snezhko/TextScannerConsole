#include "pch.h"
#include "Parser.h"
#include <string>

// rectangle bound indexes for callers of findLetterBounds()
constexpr int LEFT_X = 0;
constexpr int TOP_Y = 1;
constexpr int RIGHT_X = 2;
constexpr int BOTTOM_Y = 3;

// line bound indexes for findTextLines()
constexpr int LINE_BOTTOM_Y = 0;
constexpr int LINE_TOP_Y = 1;

Parser* Parser::instance = nullptr;

Parser* Parser::getInstance()
{
	// singleton pattern
	if (instance == nullptr)
	{
		instance = new Parser();
	}
	return instance;
}

std::string Parser::scanImage(BlackAndWhiteImage image)
{
	this->image = image;

	// finds text lines in image
	std::vector<int*> lineBounds = findTextLines();

	std::string word;
	
	// interprets each line of text
	for (int i = (int)lineBounds.size() - 1; i >= 0; i--)
	{
		// used for adding spaces
		int pixelsAfterLetter = 0;
		int lengthForSpace = 0;
		int lettersFoundOnLine = 0;

		// sweeps each column of pixels in line bounds from bottom to top of column and left to right to find black pixel
		for (int x = 0; x < image.getWidth(); x++)
		{
			for (int y = lineBounds.at(i)[LINE_BOTTOM_Y]; y < lineBounds.at(i)[LINE_TOP_Y]; y++)
			{
				// attempts to parse into letter if black pixel is found
				if (image.positionOccupied(x, y))
				{
					int* letterBounds = findLetterBounds(x, lineBounds.at(i)[LINE_BOTTOM_Y], lineBounds.at(i)[LINE_TOP_Y]);
					char c = interpretToLetter(letterBounds);

					if (c != '\0')
					{
						// attempts to solve issue of I and l having almost identical appearance in calibri
						if (c == 'I' && (word.empty() || word.at(word.length() - 1) != ' '))
						{
							c = 'l';
						}

						word += c;

						lettersFoundOnLine++;
						pixelsAfterLetter = 0;

						// finds reasonable pixel length required to indicate a space; average among all letters on line
						const int width = letterBounds[RIGHT_X] - letterBounds[LEFT_X] + 1;
						const int height = letterBounds[TOP_Y] - letterBounds[BOTTOM_Y] + 1;
						lengthForSpace = (int)((lengthForSpace * (lettersFoundOnLine - 1) + (width + height) / 3) / lettersFoundOnLine);
					}

					// moves scanner to right after the scanned pixels
					x = letterBounds[RIGHT_X];

					delete[] letterBounds;

					break;
				}
			}

			// for adding spaces
			if (lettersFoundOnLine > 0)
			{
				pixelsAfterLetter++;

				// adds a space to the word if there is a lot of space in between letters
				if (pixelsAfterLetter == lengthForSpace)
				{
					word += ' ';
				}
			}
		}

		// adds new line character after reaching the end of a line in the image
		word += '\n';

		delete[] lineBounds.at(i);
	}

	image.free();
	return word;
}

// returns the y-bounds of each line of text in the image as a vector of arrays
// each element of vector: [0] = bottom y of single line of text, [1] = top y of single line of text
std::vector<int*> Parser::findTextLines()
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
		int lineHeight = lineBounds.at(i)[LINE_TOP_Y] - lineBounds.at(i)[LINE_BOTTOM_Y];
		// squared so that dots of is and js will impact average line heigt less
		averageLineHeight += (int)pow(lineHeight, 2);
	}
	averageLineHeight = (int)sqrt((double)averageLineHeight / lineBounds.size());



	// search for dots on is and js and correct line bounds
	for (size_t i = 0; i < lineBounds.size() - 1; i++)
	{
		int nextLineHeight = lineBounds.at(i + 1)[LINE_TOP_Y] - lineBounds.at(i + 1)[LINE_BOTTOM_Y];
		if (nextLineHeight < averageLineHeight / 3)
		{
			lineBounds.at(i)[LINE_TOP_Y] = lineBounds.at(i + 1)[LINE_TOP_Y];

			delete[] lineBounds.at(i + 1);
			lineBounds.erase(lineBounds.begin() + i + 1);
			i--;
		}
	}

	return lineBounds;
}

// finds proper bounds of letter in a 4-element array: [0] = left x, [1] = top y, [2] = right x, [3] = bottom y
//
// TODO: this only works if no letters overlap in the same column
int* Parser::findLetterBounds(int leftX, int lineBottomY, int lineTopY)
{
	// initializes bounds as first column of pixels
	int* correctBounds = new int[4];
	correctBounds[LEFT_X] = correctBounds[RIGHT_X] = leftX;
	correctBounds[TOP_Y] = lineTopY;
	correctBounds[BOTTOM_Y] = lineBottomY;

	for (int x = leftX; x < image.getWidth(); x++)
	{
		// edge case
		if (x == image.getWidth() - 1)
		{
			correctBounds[RIGHT_X] = x;
			break;
		}

		for (int y = lineBottomY; y <= lineTopY; y++)
		{
			// checks if there is a pixel directly to the right or rightward diagonal to current pixel
			if (image.positionOccupied(x, y) && (image.positionOccupied(x + 1, y) || image.positionOccupied(x + 1, y + 1) || image.positionOccupied(x + 1, y - 1)))
			{
				correctBounds[RIGHT_X] = x + 1;
				break;
			}

			// break out of nested loop if no pixel in column fulfills prior condition ^
			if (y == lineTopY)
			{
				goto exitCheck;
			}
		}
	}

	exitCheck:

	// trim off whitespace from top
	for (int x = correctBounds[LEFT_X]; x <= correctBounds[RIGHT_X]; x++)
	{
		if (image.positionOccupied(x, correctBounds[TOP_Y]))
		{
			break;
		}

		if (x == correctBounds[RIGHT_X])
		{
			correctBounds[TOP_Y] = correctBounds[TOP_Y] - 1;
			x = correctBounds[LEFT_X];
		}
	}

	// trim off whitespace from bottom
	for (int x = correctBounds[LEFT_X]; x <= correctBounds[RIGHT_X]; x++)
	{
		if (image.positionOccupied(x, correctBounds[BOTTOM_Y]))
		{
			break;
		}

		if (x == correctBounds[RIGHT_X])
		{
			correctBounds[BOTTOM_Y] = correctBounds[BOTTOM_Y] + 1;
			x = correctBounds[LEFT_X];
		}
	}

	return correctBounds;
}

// attempts to parse a potential letter on the bitmap into a char by comparing to all ComparisonLetters in the 'letters' field
char Parser::interpretToLetter(int* letterBounds)
{
	double greatestSimilarityPercentage = 0;
	char mostSimilarLetter = '\0';

	for (int i = 0; i < 52; i++)
	{
		double similarity = findPercentSimilar(i, letterBounds[LEFT_X], letterBounds[BOTTOM_Y], letterBounds[RIGHT_X] - letterBounds[LEFT_X] + 1, letterBounds[TOP_Y] - letterBounds[BOTTOM_Y] + 1);

		if (similarity > greatestSimilarityPercentage)
		{
			greatestSimilarityPercentage = similarity;
			mostSimilarLetter = letters[i].getLetter();
		}	
	}

	// if image in bounds too dissimilar to any letter, return null character
	if (greatestSimilarityPercentage < 80)
	{
		return '\0';
	}

	return mostSimilarLetter;
}

// compares found letter to ComparisonLetter at given index in 'letters' field; returns percentage of matching pixels
double Parser::findPercentSimilar(int comparisonLetterIndex, int leftX, int bottomY, int width, int height)
{
	int pixelMatches = 0;
	const int area = width * height;

	// pixel at (x, y) on letter in image being parsed
	for (int yOffset = 0; yOffset < height; yOffset++)
	{
		for (int xOffset = 0; xOffset < width; xOffset++)
		{
			// compares pixels by geometric center of each pixel in letter being parsed, hence the 0.5 finds the center of the pixel
			int comparisonX = (int)((xOffset + 0.5) * ((double)letters[comparisonLetterIndex].getWidth() / width));
			int comparisonY = (int)((yOffset + 0.5) * ((double)letters[comparisonLetterIndex].getHeight() / height));

			if (image.positionOccupied(leftX + xOffset, bottomY + yOffset) == letters[comparisonLetterIndex].positionOccupied(comparisonX, comparisonY))
			{
				pixelMatches++;
			}
		}
	}

	return ((double)pixelMatches / area) * 100;
}
