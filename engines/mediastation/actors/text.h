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

#ifndef MEDIASTATION_TEXT_H
#define MEDIASTATION_TEXT_H

#include "common/str.h"

#include "mediastation/actor.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

enum TextJustification {
	kTextJustificationLeft = 0x25c,
	kTextJustificationRight = 0x25d,
	kTextJustificationCenter = 0x25e
};

enum TextPosition {
	kTextPositionMiddle = 0x25e,
	kTextPositionTop = 0x260,
	kTextPositionBotom = 0x261
};

struct CharacterClass {
	uint firstAsciiCode = 0;
	uint lastAsciiCode = 0;
};

class TextActor : public SpatialEntity {
public:
	TextActor() : SpatialEntity(kActorTypeText) {};

	virtual bool isVisible() const override { return _isVisible; }
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

private:
	bool _editable = false;
	uint _loadType = 0;
	bool _isVisible = false;
	Common::String _text;
	uint _maxTextLength = 0;
	uint _fontActorId = 0;
	TextJustification _justification;
	TextPosition _position;
	Common::Array<CharacterClass> _acceptedInput;

	// Method implementations.
	Common::String text() const;
	void setText(Common::String text);
};

} // End of namespace MediaStation

#endif
