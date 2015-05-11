#include "stdafx.h"
#include "UndefSymbols.h"

std::vector<std::string> UndefinedSymbols::getUndefinedSymbols()
{
	std::ifstream file("C:/undefsymbols.txt");
	std::string temp;
	while (!file.eof())
	{
		std::getline(file, temp);
		undefSymbols.emplace_back(temp);
	}
	return undefSymbols;
}