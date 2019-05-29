#include "pch.h"
#include <iostream>
#include "Parser.h"

int main()
{
	BlackAndWhiteImage img("TestFiles/sample_text.bmp");
	//BlackAndWhiteImage img("sample_image_3.bmp");
	img.saveToFile("BW_sample_text.bmp");
	//img.saveToFile("complex_test.bmp");

	//BlackAndWhiteImage img("TestFiles/lowercase_i_and_j_test.bmp");
	//Parser* instance = Parser::getInstance();
	//std::cout << instance->scanImage(img).c_str();
}