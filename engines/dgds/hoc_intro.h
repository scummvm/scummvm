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

#ifndef DGDS_HOC_INTRO_H
#define DGDS_HOC_INTRO_H

#include "common/ptr.h"
#include "dgds/image.h"

namespace Dgds {

/**
 * Native scene ops for Heart of China intro.
 */
class HocIntro {
public:
	HocIntro();
	void init();
	void tick();
	void end();

private:
	void clean1(int16 xoff);
	void clean2(int16 xoff);
	void draw1(int16 xoff);
	void draw2(int16 xoff);

	void doCopy(int16 x1, int16 y1, int16 x2, int16 y2);
	void doScroll();

	int16 _scrollCountdown1;
	int16 _xOffset2042;
	int16 _xOffset203e;
	int16 _scrollCountdown2;
	Common::SharedPtr<Image> _noMaskImg;
	Common::SharedPtr<Image> _maskImg;
	Common::Rect _drawWin;
};

} // end namespace Dgds

#endif // DGDS_HOC_INTRO_H
