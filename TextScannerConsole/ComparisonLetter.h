#pragma once
#include "BlackAndWhiteImage.h"
class ComparisonLetter : public BlackAndWhiteImage
{
	char letter;

public:
	ComparisonLetter(char letter);

	char getLetter();
};
