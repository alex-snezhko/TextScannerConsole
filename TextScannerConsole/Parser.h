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
	int* findLetterBounds(int leftX, int lineBottomY, int lineTopY);

	char interpretToLetter(int* letterBounds);
	double findPercentSimilar(int comparisonLetterIndex, int leftX, int bottomY, int width, int height);

public:
	static Parser* getInstance();

	std::string scanImage(BlackAndWhiteImage image);
};

