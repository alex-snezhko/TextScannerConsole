#include "pch.h"
#include "Parser.h"
#include <string>

// rectangle bound indexes for callers of findRectangleForLetter()
constexpr int LEFT_X = 0;
constexpr int TOP_Y = 1;
constexpr int RIGHT_X = 2;
constexpr int BOTTOM_Y = 3;

constexpr int DOTTED = 4;

Parser* Parser::instance = nullptr;

Parser::Parser() : image(nullptr), letters{ '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' }
{
	// loads all alphabetic letters into parser from images
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

		for (int x = 0; x < image.getWidth(); x++)
		{
			for (int y = lineBounds.at(i)[0]; y < lineBounds.at(i)[1]; y++)
			{
				// attempts to parse into letter if black pixel is found
				if (image.positionOccupied(x, y))
				{
					int* letterBounds = findBoundsOfLetter(x, y);
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

// vector of arrays; [0] = bottom y of single line of text, [1] = top y of single line of text
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

// returns bounds of letter in a 5-element array: top left x; top left y; bottom right x; bottom right y;
//		position 5: 1 if letter with dot (lowercase i or j) and 0 otherwise
//
// called when a black pixel is found while sweeping rows
// attempts to box each individual letter
int* Parser::findBoundsOfLetter(int x, int y)
{
	int* rect = findRectangleForShape(x, y, x, y);
	int* letterRectangle = new int[5]{ rect[LEFT_X], rect[TOP_Y], rect[RIGHT_X], rect[BOTTOM_Y], 0 };
	delete[] rect;

	// attempt to find dots on is and js
	int width = letterRectangle[RIGHT_X] - letterRectangle[LEFT_X] + 1;
	int height = letterRectangle[TOP_Y] - letterRectangle[BOTTOM_Y] + 1;
	if ((double)height / width > 2.5)
	{
		for (int yOffset = 1; yOffset < (int)((double)height / 3); yOffset++)
		{
			for (int xOffset = 0; xOffset < width; xOffset++)
			{
				// searches directly above the shape to find dots
				int xPos = letterRectangle[LEFT_X] + xOffset;
				int yPos = letterRectangle[TOP_Y] + yOffset;
				if (image.positionOccupied(xPos, yPos))
				{
					int* dotBounds = findRectangleForShape(xPos, yPos, xPos, yPos);

					if (dotBounds != nullptr)
					{
						int dotWidth = dotBounds[RIGHT_X] - dotBounds[LEFT_X] + 1;
						int dotHeight = dotBounds[TOP_Y] - dotBounds[BOTTOM_Y] + 1;

						if (dotHeight < (int)((double)height / 2.5) && dotWidth < (int)((double)height / 2.5))
						{
							// sets new bounds for i or j
							letterRectangle[TOP_Y] = dotBounds[TOP_Y];			
							letterRectangle[LEFT_X] = (int)fmin(letterRectangle[LEFT_X], dotBounds[LEFT_X]);
							letterRectangle[RIGHT_X] = (int)fmax(letterRectangle[RIGHT_X], dotBounds[RIGHT_X]);

							// indicates to caller that this is a dotted letter
							letterRectangle[DOTTED] = 1;
						}
					}
					goto afterDot;
				}
			}
		}
	}

	afterDot:

	return letterRectangle;
}

// recursively enlarges searching field and when complete returns array with 4 elements (order of fields) of rectangle containing proper info
int* Parser::findRectangleForShape(int leftX, int topY, int rightX, int bottomY)
{
	/*int* correctBounds = new int[4];
	correctBounds[LEFT_X] = correctBounds[RIGHT_X] = originalBounds[LEFT_X];
	correctBounds[TOP_Y] = originalBounds[TOP_Y];
	correctBounds[BOTTOM_Y] = originalBounds[BOTTOM_Y];

	for (int x = originalBounds[LEFT_X]; x < originalBounds[RIGHT_X]; x++)
	{
		for (int y = originalBounds[BOTTOM_Y]; y <= originalBounds[TOP_Y]; y++)
		{
			// checks if there is a pixel directly to the right or rightward diagonal to current pixel
			bool(*isPixel)(int, int) = image.positionOccupied;
			if (isPixel(x, y) && (isPixel(x + 1, y) || isPixel(x + 1, y + 1) || isPixel(x + 1, y - 1)))
			{
				correctBounds[RIGHT_X] = x + 1;

				x++;
				y = originalBounds[BOTTOM_Y];
				break;
			}

			// break if no pixel in column fulfills prior condition ^
			if (y == originalBounds[TOP_Y])
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

	delete[] originalBounds;
	originalBounds = correctBounds;*/





	// Prevents infinite recursion
	if (leftX < 0 || topY >= image.getHeight() || rightX >= image.getWidth() || bottomY < 0)
	{
		throw ParsingException();
	}

	bool expandLeft = false;
	bool expandRight = false;
	bool expandUp = false;
	bool expandDown = false;

	// checks if left/right expansion needed
	for (int y = bottomY; y <= topY; y++)
	{
		if (image.positionOccupied(leftX, y))
		{
			expandLeft = true;
		}

		if (image.positionOccupied(rightX, y))
		{
			expandRight = true;
		}

		if (expandLeft && expandRight)
		{
			break;
		}
	}

	// checks if up/downward expansion needed
	for (int x = leftX; x <= rightX; x++)
	{
		if (image.positionOccupied(x, topY))
		{
			expandUp = true;
		}

		if (image.positionOccupied(x, bottomY))
		{
			expandDown = true;
		}

		if (expandUp && expandDown)
		{
			break;
		}
	}

	// correct rectangle, removes extra white pixel on border
	if (!(expandRight || expandLeft || expandUp || expandDown))
	{
		return new int[4]{ leftX + 1, topY - 1, rightX - 1, bottomY + 1 };
	}

	// expand rectangle as needed and search again
	return findRectangleForShape(
		leftX + (expandLeft ? -1 : 0),
		topY + (expandUp ? 1 : 0),
		rightX + (expandRight ? 1 : 0),
		bottomY + (expandDown ? -1 : 0)
	);
}

// attempts to parse a potential letter on the bitmap into a char by comparing to ComparisonLetters in the 'letters' field
char Parser::interpretToLetter(int* letterBounds)
{
	double greatestSimilarityPercentage = 0;
	char mostSimilarLetter = '\0';

	for (int i = 0; i < 52; i++)
	{
		// ensures only is and js get checked if letter is dotted
		if (letterBounds[DOTTED] == 0 || letters[i].getLetter() == 'i' || letters[i].getLetter() == 'j')
		{
			double similarity = findPercentSimilar(i, letterBounds[LEFT_X], letterBounds[BOTTOM_Y], letterBounds[RIGHT_X] - letterBounds[LEFT_X] + 1, letterBounds[TOP_Y] - letterBounds[BOTTOM_Y] + 1);

			if (similarity > greatestSimilarityPercentage)
			{
				greatestSimilarityPercentage = similarity;
				mostSimilarLetter = letters[i].getLetter();
			}
		}
	}

	// if image in bounds too dissimilar to any letter, return null character
	if (greatestSimilarityPercentage < 80)
	{
		//correctLetterBounds(letterBounds);
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

// corrects bounds to try to find individual letter in the case that 2+ letters get formed into one
//
// TODO probably make this the original bounds algorithm with the line bounds being passed in as parameter
//
//
void Parser::correctLetterBounds(int* originalBounds)
{
	int* correctBounds = new int[4];
	correctBounds[LEFT_X] = correctBounds[RIGHT_X] = originalBounds[LEFT_X];
	correctBounds[TOP_Y] = originalBounds[TOP_Y];
	correctBounds[BOTTOM_Y] = originalBounds[BOTTOM_Y];

	for (int x = originalBounds[LEFT_X]; x < originalBounds[RIGHT_X]; x++)
	{
		for (int y = originalBounds[BOTTOM_Y]; y <= originalBounds[TOP_Y]; y++)
		{	
			// checks if there is a pixel directly to the right or rightward diagonal to current pixel	
			if (image.positionOccupied(x, y) && (image.positionOccupied(x + 1, y) || image.positionOccupied(x + 1, y + 1) || image.positionOccupied(x + 1, y - 1)))
			{
				correctBounds[RIGHT_X] = x + 1;

				x++;
				y = originalBounds[BOTTOM_Y];
				break;
			}

			// break if no pixel in column fulfills prior condition ^
			if (y == originalBounds[TOP_Y])
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

	delete[] originalBounds;
	originalBounds = correctBounds;
}
