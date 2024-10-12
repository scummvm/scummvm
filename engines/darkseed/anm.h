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

#ifndef DARKSEED_ANM_H
#define DARKSEED_ANM_H

#include "common/array.h"
#include "darkseed/img.h"
namespace Darkseed {

class Anm {
private:
	Common::File _file;
	uint16 _numRecords = 0;
	uint16 _assetFileId = 0;
	int _deltaOffset = 0;

public:
	bool load(const Common::Path &filename, int deltaOffset = 0);
	bool getImg(uint16 index, Img &img, bool includesPosition = true);
	int numImages();
};

} // namespace Darkseed

#endif // DARKSEED_ANM_H
