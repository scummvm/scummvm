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
#include "mediastation/actors/font.h"
#include "mediastation/graphics.h"
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
	kTextPositionBottom = 0x261
};

class TextActor : public SpatialEntity {
public:
	TextActor() : SpatialEntity(kActorTypeText) {};

	virtual void loadIsComplete() override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void draw(DisplayContext &displayContext) override;
	virtual uint16 findActorToAcceptKeyboardEvents(uint16 charCode, uint16 eventMask, MouseActorState &state) override;
	virtual void keyboardEvent(const KeyboardEvent &event) override;

private:
	static const uint CURSOR_CHAR_ID = 0x104;
	bool _isEditable = false;
	Common::String _text;
	uint _maxLength = 0;
	FontActor *_fontActor = nullptr;
	TextJustification _justification = kTextJustificationLeft;
	TextPosition _position = kTextPositionTop;
	Common::HashMap<uint, uint> _acceptedChars;
	uint _cursorPosition = 0;
	uint _pressedCharCode = 0;
	bool _cursorIsVisible = false;
	bool _constrainToWidth = false;
	bool _overwriteMode = false;

	void setText();
	void addAcceptedChars(uint firstCharCode, uint lastCharCode, uint charCodeOffset = 0);
	bool hasScriptResponse(EventType eventType, const ScriptValue &arg) const;

	int16 calcStartingXPosition();
	int16 calcBaseline();
	void drawCharacter(FontCharacter *glyph, int16 x, int16 y, DisplayContext &displayContext);
	void drawCursor(int16 x, int16 y, DisplayContext &displayContext);
	int16 calcPixelLength(const Common::String &text);
};

} // End of namespace MediaStation

#endif
