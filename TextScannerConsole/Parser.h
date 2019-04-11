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
	int* findRectangleForLetter(int topLeftX, int topLeftY, int bottomRightX, int bottomRightY);
	char interpretToLetter(int origX, int origY, int width, int height);
	double findSimilarity(int index, int origX, int origY, int width, int height);

public:
	static Parser* getInstance();

	std::string scanImage(BlackAndWhiteImage image);
};

