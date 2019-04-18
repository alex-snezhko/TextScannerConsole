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

	std::vector<int*> findLineBounds();
	int* findBoundsOfLetter(int x, int y);
	int* findRectangleForLetter(int leftX, int topY, int rightX, int bottomY);
	char interpretToLetter(int* letterBounds);
	double findPercentSimilar(int lettersIndex, int leftX, int bottomY, int width, int height);

public:
	static Parser* getInstance();

	std::string scanImage(BlackAndWhiteImage image);
};

