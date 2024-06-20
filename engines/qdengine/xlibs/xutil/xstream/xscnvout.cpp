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

#include "qdengine/core/qd_precomp.h"


namespace QDEngine {

XStream& XStream::operator>= (char& var) {
	int ret = read(ConvertBuffer_, 4);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (char)strtol(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned char& var) {
	int ret = read(ConvertBuffer_, 4);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (unsigned char)strtoul(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (short& var) {
	int ret = read(ConvertBuffer_, 6);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (short)strtol(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned short& var) {
	int ret = read(ConvertBuffer_, 6);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (unsigned short)strtoul(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (int& var) {
	int ret = read(ConvertBuffer_, 16);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtol(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned& var) {
	int ret = read(ConvertBuffer_, 16);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtoul(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (long& var) {
	int ret = read(ConvertBuffer_, 16);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtol(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (unsigned long& var) {
	int ret = read(ConvertBuffer_, 16);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtoul(p, &p, 0);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (double& var) {
	int ret = read(ConvertBuffer_, CONV_BUFFER_LEN);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtod(p, &p);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (float& var) {
	int ret = read(ConvertBuffer_, CONV_BUFFER_LEN);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = (float)strtod(p, &p);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}

XStream& XStream::operator>= (long double& var) {
	int ret = read(ConvertBuffer_, CONV_BUFFER_LEN);
	if (!ret) return *this;
	char* p = ConvertBuffer_;
	p[ret] = ' ';
	var = strtod(p, &p);
	seek(p - ConvertBuffer_ + 1 - ret, XS_CUR);
	return *this;
}
} // namespace QDEngine
