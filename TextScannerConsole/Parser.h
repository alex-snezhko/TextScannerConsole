#pragma once
#include <exception>
#include "ComparisonLetter.h"
#include <vector>

class ParsingException : public std::exception {};

class Parser
{
	static Parser* instance;
	ComparisonLetter letters[52];
	BlackAndWhiteImage image;

	Parser();

	std::vector<int*> findTextLines();
	int* findBoundsOfLetter(int x, int y);
	int* findRectangleForShape(int leftX, int topY, int rightX, int bottomY);
	char interpretToLetter(int* letterBounds);
	double findPercentSimilar(int comparisonLetterIndex, int leftX, int bottomY, int width, int height);

public:
	static Parser* getInstance();

	std::string scanImage(BlackAndWhiteImage image);
};

