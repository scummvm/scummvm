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

#ifndef NANCY_ACTION_TYPEWRITERTEXT_H
#define NANCY_ACTION_TYPEWRITERTEXT_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Nancy18. Writes an AUTOTEXT entry into a rectangle a few characters at a time,
// with a key sound for every batch. An event flag gets set once it is all there.
class TypewriterText : public RenderActionRecord {
public:
	TypewriterText() : RenderActionRecord(7) {}
	virtual ~TypewriterText() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "TypewriterText"; }

	// Returns how many characters got added
	uint typeCharacters(uint numCharacters);
	void redraw();

	// -- File data --
	Common::String _textKey;
	Common::Rect _bounds;
	double _charactersPerSecond = 0;
	uint32 _fontID = 0;
	uint32 _colorID = 0;
	RandomSoundBlock _keySound;
	FlagDescription _doneFlag;

	// -- Runtime state --
	Common::String _text;
	uint _numTypedCharacters = 0;
	uint32 _lastTickTime = 0;
	uint32 _pendingTicks = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TYPEWRITERTEXT_H
