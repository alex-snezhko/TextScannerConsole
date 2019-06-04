#include "pch.h"
#include <iostream>
#include "Parser.h"

int main()
{
	//BlackAndWhiteImage img("TestFiles/complex_test.bmp");
	//img.saveToFile("TestFiles/BW_complex_test.bmp");
	
	BlackAndWhiteImage img("TestFiles/sample_image_3.bmp");
	img.saveToFile("TestFiles/BW_sample_image_3.bmp");
	Parser* instance = Parser::getInstance();
	std::cout << instance->scanImage(img).c_str();
	img.free();
}