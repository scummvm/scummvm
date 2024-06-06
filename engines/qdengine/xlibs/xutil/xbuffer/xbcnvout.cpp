/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qdengine/xlibs/xutil/xglobal.h"
#include "math.h"


namespace QDEngine {

double strtod_(char* str, char** strOut) {
	while (isspace(*str))
		++str;

	bool positive = true;
	if (*str == '-') {
		positive = false;
		++str;
	}

	double val = 0;
	while (isdigit(*str)) {
		val *= 10;
		val += (double)(*str++ - '0');
	}

	if (*str == '.') {
		++str;
		double dig = 1;
		while (isdigit(*str)) {
			dig  /= 10;
			val += ((double)(*str++ - '0')) * dig;
		}
	}

	if (*str == 'e' || *str == 'E' || *str == 'd' || *str == 'D') {
		bool positivePower = true;
		if (*++str == '+')
			++str;
		else if (*str == '-') {
			positivePower = false;
			++str;
		}
		int power = strtol(str, &str, 10);
		if (!positivePower)
			power = -power;
		val *= pow(10., power);
	}

	*strOut = str;
	return positive ? val : -val;
}

XBuffer& XBuffer::operator>= (char& var) {
	char* p = buffer_ + offset_;
	var = (char)strtol(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned char& var) {
	char* p = buffer_ + offset_;
	var = (unsigned char)strtoul(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (short& var) {
	char* p = buffer_ + offset_;
	var = (short)strtol(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned short& var) {
	char* p = buffer_ + offset_;
	var = (unsigned short)strtoul(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (int& var) {
	char* p = buffer_ + offset_;
	var = strtol(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned& var) {
	char* p = buffer_ + offset_;
	var = strtoul(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (long& var) {
	char* p = buffer_ + offset_;
	var = strtol(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (unsigned long& var) {
	char* p = buffer_ + offset_;
	var = strtoul(p, &p, 0);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (double& var) {
	char* p = buffer_ + offset_;
	var = strtod_(p, &p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (long double& var) {
	char* p = buffer_ + offset_;
	var = strtod_(p, &p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}

XBuffer& XBuffer::operator>= (float& var) {
	char* p = buffer_ + offset_;
	var = (float)strtod_(p, &p);
	offset_ += p - (buffer_ + offset_);
	return *this;
}
} // namespace QDEngine
