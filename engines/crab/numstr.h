#pragma once
#include "common_header.h"

namespace Crab {

template<typename T>
Common::String NumberToString(T Number) {
	Common::String res;
	const T Num = Number;

	if (Number == 0) {
		return "0";
	}

	if (Number < 0)
		Number = -Number;

	while (Number > 0) {
		T d = Number % 10;
		res += ('0' + d);
		Number /= 10;
	}

	if (Num < 0)
		res += "-";

	// reverse string
	Common::String revStr;
	for (int i = res.size() - 1; i >= 0; i--)
		revStr += res[i];

	return revStr;
}

template<typename T>
T StringToNumber(char *Text) {
	T result = 0;
	char *num = Text;

	if (Text[0] == '-')
		num++;

	for (int i = 0; num[i]; i++)
		result = (result * 10) + (num[i] - '0');

	return Text[0] == '-' ? -result : result;
}

template<typename T>
T StringToNumber(const Common::String &Text) {
	return StringToNumber<T>(const_cast<char *>(Text.c_str()));
}

template<typename T>
void GetPoint(T &v, Common::String &coords) {
	int comma = coords.findFirstOf(',');

	v.x = StringToNumber<int>(coords.substr(0, comma));
	v.y = StringToNumber<int>(coords.substr(comma + 1));
}

} // End of namespace Crab
