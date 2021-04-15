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

#include "snatcher/render.h"

namespace Snatcher {

class SCDRenderer : public Renderer {
public:
	SCDRenderer() : Renderer(), _screenWidth(320), _screenHeight(224) {}
	~SCDRenderer() override {}

	uint16 screenWidth() const { return _screenWidth; }
	uint16 screenHeight() const { return _screenHeight; }

private:
	const uint16 _screenWidth, _screenHeight;
};

Renderer *Renderer::createSegaRenderer() {
	return new SCDRenderer();
}

} // End of namespace Snatcher
