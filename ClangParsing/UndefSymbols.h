#include "stdafx.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

class UndefinedSymbols{
private:
	std::vector<std::string> undefSymbols;

public:
	std::vector<std::string> getUndefinedSymbols();
};