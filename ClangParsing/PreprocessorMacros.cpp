#include "stdafx.h"
#include "PreprocessorMacros.h"

std::vector<std::string> PreprocessorMacros::getMacros()
{
	macros.emplace_back("AVI");
	return macros;
}