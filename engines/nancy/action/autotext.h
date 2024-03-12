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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_AUTOTEXT_H
#define NANCY_ACTION_AUTOTEXT_H

#include "engines/nancy/misc/hypertext.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Action record used for rendering text inside the game viewport.
// Can be used in two ways: for single-use texts that get thrown away
// after a scene change, or for permanent storage (used in nancy's journal)
// Does not own or display any image data; it draws to a surface inside
// GraphicsManager, which other ActionRecords (Overlay and PeepholePuzzle) can use.
class Autotext : public virtual ActionRecord, public Misc::HypertextParser {
public:
	Autotext() {}
	virtual ~Autotext() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "Autotext"; }

	virtual void readExtraData(Common::SeekableReadStream &stream);

	uint16 _transparency = kPlayOverlayPlain;
	uint16 _surfaceID = 0;
	uint16 _fontID = 0;
	uint16 _textColor = 0;
	Common::Point _offset;
	uint16 _surfWidth = 0;
	uint16 _surfHeight = 0;

	bool _useAutotextChunk = false;

	// Only one of these can be valid
	Common::String _textKey;
	Common::String _embeddedText;

	uint16 _order = kListFIFO;
	bool _shouldDrawMarks = false;

	Common::Array<uint16> _hotspotScenes;

	Graphics::ManagedSurface _image;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_AUTOTEXT_H
