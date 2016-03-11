/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ADL_PICTURE_H
#define ADL_PICTURE_H

namespace Common{
class SeekableReadStream;
class Point;
}

namespace Adl {

class Display;

class PictureD {
public:
	PictureD(Display &display) : _display(display) { }

	void drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const;
	void draw(Common::SeekableReadStream &pic);

private:
	void clear();
	void drawCorners(Common::SeekableReadStream &pic, bool yFirst);
	void drawRelativeLines(Common::SeekableReadStream &pic);
	void drawAbsoluteLines(Common::SeekableReadStream &pic);
	void fillRow(const Common::Point &p, bool fillBit, byte pattern);
	void fill(Common::SeekableReadStream &pic);

	Display &_display;
	byte _color;
};

} // End of namespace Adl

#endif
