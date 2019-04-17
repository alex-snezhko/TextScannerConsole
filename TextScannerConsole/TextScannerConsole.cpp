#include "pch.h"
#include <iostream>
#include "Parser.h"

int main()
{
	//BlackAndWhiteImage img("TestFiles/sample_text.bmp");

	BlackAndWhiteImage img("TestFiles/lowercase_i_and_j_test.bmp");
	Parser* instance = Parser::getInstance();
	std::cout << instance->scanImage(img).c_str();
}