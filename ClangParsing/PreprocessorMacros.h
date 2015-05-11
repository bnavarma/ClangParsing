#include "stdafx.h"
#include <vector>

class PreprocessorMacros{
private:
	std::vector<std::string> macros;

public:
	std::vector<std::string> getMacros();
};