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

	Parser() : image(nullptr), letters{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
									'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' }
	{}

	std::vector<int*> findTextLines();
	int* findLetterBounds(int leftX, int lineBottomY, int lineTopY);

	char interpretToLetter(int* letterBounds);
	double findPercentSimilar(int comparisonLetterIndex, int leftX, int bottomY, int width, int height);

public:
	static Parser* getInstance();

	std::string scanImage(BlackAndWhiteImage image);
};

