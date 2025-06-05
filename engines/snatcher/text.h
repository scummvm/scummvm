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


#ifndef SNATCHER_TEXT_H
#define SNATCHER_TEXT_H

#include "common/platform.h"
#include "common/scummsys.h"

namespace Snatcher {

class Animator;
class TextRenderer {
public:

	TextRenderer() {}
	virtual ~TextRenderer() {}

	virtual void setFont(const uint8 *font, uint32 fontSize) = 0;
	virtual void setCharWidthTable(const uint8 *table, uint32 tableSize) = 0;

	virtual void enqueuePrintJob(const uint8 *text) = 0;
	virtual void setPrintDelay(int delay) = 0;
	//virtual void setColor(uint8 color) = 0;

	virtual void draw() = 0;
	virtual void reset() = 0;
	//virtual void requestRefresh(bool req) = 0;
	virtual bool needsPrint() const = 0;

protected:

private:
	static TextRenderer *createSegaTextRenderer(Animator *animator);

public:
	static TextRenderer *create(Common::Platform platform, Animator *animator) {
		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSegaTextRenderer(animator);
		default:
			break;
		};
		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_TEXT_H
