#pragma once
#include "common_header.h"

template <typename T>
std::string NumberToString ( T Number )
{
	std::stringstream ss;
	ss << Number;
	return ss.str();
}

template <typename T>
T StringToNumber ( char *Text )//Text not by const reference so that the function can be used with a
{                               //character array as argument
	std::stringstream ss(Text);
	T result;
	return (ss >> result) ? result : 0;
}

template <typename T>
T StringToNumber (const std::string &Text )
{
	std::stringstream ss(Text.c_str());
	T result;
	return (ss >> result) ? result : 0;
}

template <typename T>
void GetPoint(T &v, std::string& coords )
{
	int comma = coords.find_first_of(',');

	v.x = StringToNumber<int>( coords.substr(0,comma) );
	v.y = StringToNumber<int>( coords.substr(comma+1));
}