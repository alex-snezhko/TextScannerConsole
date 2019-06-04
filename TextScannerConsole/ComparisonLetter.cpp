#include "pch.h"
#include "ComparisonLetter.h"
#include <string>

ComparisonLetter::ComparisonLetter(char letter) : BlackAndWhiteImage(nullptr)
{
	// collects data from bmp file
	this->letter = letter;
	std::string fileName = "BWLetters/";
	if (letter >= 97 && letter <= 122)
	{
		fileName.append("BWLower");
		letter -= 32;
	}
	else if (letter >= 65 && letter <= 90)
	{
		fileName.append("BWUpper");
	}
	else
	{
		return;
	}
	fileName += letter;
	fileName.append("Sample.bmp");

	loadBitmap(fileName.c_str(), true);
}

// gets the letter which this ComparisonImage represents
char ComparisonLetter::getLetter()
{
	return letter;
}


