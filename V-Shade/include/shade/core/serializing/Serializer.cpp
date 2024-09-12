#include "shade_pch.h"
#include "Serializer.h"

bool shade::IsAllowedCharacter(const char c)
{
	const std::string unsupportedCharacters = "\\/:*?\"<>|";
	return std::find(unsupportedCharacters.begin(), unsupportedCharacters.end(), c) == unsupportedCharacters.end();
}

std::string shade::RemoveNotAllowedCharacters(const std::string& input)
{
	std::string result;
	std::copy_if(input.begin(), input.end(), std::back_inserter(result), IsAllowedCharacter);
	return result;
}