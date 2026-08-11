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

#ifndef DGDS_GAME_PALETTES_H
#define DGDS_GAME_PALETTES_H

#include "common/error.h"
#include "common/str.h"
#include "common/serializer.h"
#include "graphics/palette.h"

namespace Dgds {

class Decompressor;
class ResourceManager;

class DgdsPal : public Graphics::Palette {
public:
	DgdsPal();
	virtual ~DgdsPal() {}
	const Common::String &getName() { return _name; }
	void setName(const Common::String &name) { _name = name; }
private:
	Common::String _name;
};

class GamePalettes {
public:
	GamePalettes(ResourceManager *resourceMan, Decompressor *decompressor);
	int loadPalette(const Common::String &filename);
	void selectPalNum(int num);
	void setPalette();
	void clearPalette();

	// Reset the list to the post-game-load state (1 palette loaded)
	void reset();

	// Fade the colors in the current palette toward black. Start at col, and fade ncols of the palette.
	// Add coloff to the result to move toward white.
	void setFade(int col, int ncols, int coloff, int fade);

	Common::Error syncState(Common::Serializer &s);

	uint getCurPalNum() const { return _curPalNum; }

	const DgdsPal &getCurPal() const { return _curPal; }

private:
	ResourceManager *_resourceMan;
	Decompressor *_decompressor;

	DgdsPal _curPal;
	uint _curPalNum;
	Common::Array<DgdsPal> _palettes;
};

} // end namespace Dgds

#endif // DGDS_GAME_PALETTES_H
