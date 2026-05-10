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

#include "mediastation/actors/text.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

void TextActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderLoadType:
		// The original doesn't seem to use this, so we'll throw it away.
		chunk.readTypedByte();
		break;

	case kActorHeaderFontActorId: {
		uint fontActorId = chunk.readTypedUint16();
		_fontActor = static_cast<FontActor *>(g_engine->getImtGod()->getActorByIdAndType(fontActorId, kActorTypeFont));
		break;
	}

	case kActorHeaderInitialText:
		_text = chunk.readTypedString();
		break;

	case kActorHeaderTextMaxLength:
		_maxLength = chunk.readTypedUint16();
		break;

	case kActorHeaderTextJustification:
		_justification = static_cast<TextJustification>(chunk.readTypedUint16());
		break;

	case kActorHeaderTextPosition:
		_position = static_cast<TextPosition>(chunk.readTypedUint16());
		break;

	case kActorHeaderTextAcceptedCharRangeWithOffset: {
		uint firstCharCode = chunk.readTypedUint16();
		uint lastCharCode = chunk.readTypedUint16();
		uint charCodeOffset = chunk.readTypedUint16();
		addAcceptedChars(firstCharCode, lastCharCode, charCodeOffset);
		break;
	}

	case kActorHeaderTextAcceptedCharRange: {
		uint firstCharCode = chunk.readTypedUint16();
		uint lastCharCode = chunk.readTypedUint16();
		addAcceptedChars(firstCharCode, lastCharCode);
		break;
	}

	case kActorHeaderEditable:
		_isEditable = chunk.readTypedByte();
		break;

	case kActorHeaderTextCursorIsVisible:
		_cursorIsVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderTextConstrainToWidth:
		_constrainToWidth = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderTextOverwriteMode:
		_overwriteMode = static_cast<bool>(chunk.readTypedByte());
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

void TextActor::loadIsComplete() {
	setText();
	SpatialEntity::loadIsComplete();
}

ScriptValue TextActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kSpatialShowMethod:
		ARGCOUNTCHECK(0);
		if (!_isVisible) {
			_isVisible = true;
			invalidateLocalBounds();
		}
		break;

	case kSpatialHideMethod:
		ARGCOUNTCHECK(0);
		if (_isVisible) {
			_isVisible = false;
			invalidateLocalBounds();
		}
		break;

	case kTextSetEditableMethod:
		ARGCOUNTCHECK(0);
		_isEditable = true;
		break;

	case kTextSetNonEditableMethod:
		ARGCOUNTCHECK(0);
		_isEditable = false;
		break;

	case kTextGetFontActorMethod: {
		ARGCOUNTCHECK(0);
		returnValue.setToActorId(_fontActor->id());
		break;
	}

	case kTextSetFontActorMethod: {
		ARGCOUNTCHECK(1);
		uint fontActorId = args[0].asActorId();
		_fontActor = static_cast<FontActor *>(g_engine->getImtGod()->getActorByIdAndType(fontActorId, kActorTypeFont));
		invalidateLocalBounds();
		break;
	}

	case kTextGetTextMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToString(_text);
		break;

	case kTextSetTextMethod: {
		ARGCOUNTCHECK(1);
		_text = args[0].asString();
		setText();
		invalidateLocalBounds();
		break;
	}

	case kTextGetMaxLengthMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_maxLength);
		break;

	case kTextSetMaxLengthMethod:
		ARGCOUNTCHECK(1);
		_maxLength = static_cast<uint>(args[0].asFloat());
		invalidateLocalBounds();
		break;

	case kGetLastPressedCharCodeMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_pressedCharCode);
		break;

	case kTextGetCursorPositionMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_cursorPosition);
		break;

	case kTextSetCursorPositionMethod: {
		ARGCOUNTCHECK(1);
		uint newPos = static_cast<uint>(args[0].asFloat());
		if (newPos > _text.size()) {
			newPos = _text.size();
		}
		_cursorPosition = newPos;
		invalidateLocalBounds();
		break;
	}

	case kTextGetJustificationMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToParamToken(static_cast<uint>(_justification));
		break;

	case kTextSetJustificationMethod:
		ARGCOUNTCHECK(1);
		_justification = static_cast<TextJustification>(args[0].asParamToken());
		invalidateLocalBounds();
		break;

	case kTextGetPositionMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToParamToken(static_cast<uint>(_position));
		break;

	case kTextSetPositionMethod:
		ARGCOUNTCHECK(1);
		_position = static_cast<TextPosition>(args[0].asParamToken());
		invalidateLocalBounds();
		break;

	case kTextGetConstrainToWidthMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_constrainToWidth);
		break;

	case kTextSetConstrainToWidthMethod:
		ARGCOUNTCHECK(1);
		_constrainToWidth = static_cast<bool>(args[0].asBool());
		invalidateLocalBounds();
		break;

	case kTextGetCursorIsVisibleMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_cursorIsVisible);
		break;

	case kTextSetCursorIsVisibleMethod:
		ARGCOUNTCHECK(1);
		_cursorIsVisible = static_cast<bool>(args[0].asBool());
		invalidateLocalBounds();
		break;

	case kTextGetOverwriteModeMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_overwriteMode);
		break;

	case kTextSetOverwriteModeMethod:
		ARGCOUNTCHECK(1);
		_overwriteMode = static_cast<bool>(args[0].asBool());
		break;

	case kTextGetTranslatedCharCode: {
		ARGCOUNTCHECK(1);
		uint charId = args[0].asParamToken();
		uint translatedChar = _acceptedChars.getValOrDefault(charId, 0);
		if (translatedChar != 0) {
			returnValue.setToFloat(translatedChar);
		} else {
			// Character not found, so return the input as-is.
			returnValue = args[0];
		}
		break;
	}

	case kTextAddAcceptedCharsMethod: {
		ARGCOUNTMIN(2);
		uint startCharId = static_cast<uint>(args[0].asFloat());
		uint endCharId = static_cast<uint>(args[1].asFloat());
		uint category = 0;
		if (args.size() >= 3) {
			category = static_cast<uint>(args[2].asFloat());
		}

		addAcceptedChars(startCharId, endCharId, category);
		break;
	}

	case kTextIsCharacterAcceptedMethod: {
		ARGCOUNTCHECK(1);
		uint charId = static_cast<uint>(args[0].asFloat());
		bool isAccepted = _acceptedChars.contains(charId);
		returnValue.setToBool(isAccepted);
		break;
	}

	case kTextEnableDisableCharacterMethod: {
		ARGCOUNTCHECK(2);
		uint charId = static_cast<uint>(args[0].asFloat());
		bool shouldEnable = static_cast<bool>(args[1].asBool());
		if (shouldEnable) {
			// Mark character as accepted.
			addAcceptedChars(charId, charId, 0);
		} else {
			// No longer mark character as accepted.
			_acceptedChars.erase(charId);
		}
		break;
	}

	case kTextIsEditableMethod:
		returnValue.setToBool(_isEditable);
		break;

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
	}
	return returnValue;
}

void TextActor::draw(DisplayContext &displayContext) {
	if (_fontActor == nullptr) {
		warning("[%s] %s: No font", debugName(), __func__);
		return;
	}

	Common::Point positionOnScreen(calcStartingXPosition(), calcBaseline());
	for (uint positionInString = 0; positionInString < _text.size(); positionInString++) {
		FontCharacter *fontChar = nullptr;
		char currentChar = _text[positionInString];

		fontChar = _fontActor->lookupCharacter(currentChar);
		if (fontChar != nullptr) {
			// We have the character, so draw it.
			drawCharacter(fontChar, positionOnScreen.x, positionOnScreen.y, displayContext);

			// Also draw the cursor if necessary.
			if (positionInString == _cursorPosition && _cursorIsVisible) {
				drawCursor(positionOnScreen.x, positionOnScreen.y, displayContext);
			}

			positionOnScreen.x += fontChar->_horizontalSpacing + fontChar->width();

		} else if (currentChar == ' ') {
			// Handle space character, which might not be in the font.
			if (positionInString == _cursorPosition && _cursorIsVisible) {
				drawCursor(positionOnScreen.x, positionOnScreen.y, displayContext);
			}

			positionOnScreen.x += _fontActor->_averageCharWidth;

		} else {
			warning("[%s] %s: No font char for %d", debugName(), __func__, currentChar);
		}
	}

	// Draw cursor at end of the text if necessary.
	if (_cursorPosition == _text.size() && _cursorIsVisible) {
		drawCursor(positionOnScreen.x, positionOnScreen.y, displayContext);
	}
}

uint16 TextActor::findActorToAcceptKeyboardEvents(uint16 charCode, uint16 eventMask, MouseActorState &state) {
	uint16 result = 0;
	if (_loadIsComplete && (eventMask & kKeyDownFlag) && _isEditable) {
		// If we have accepted character restrictions, check if character is valid.
		if (!_acceptedChars.empty()) {
			if (!_acceptedChars.contains(charCode)) {
				return 0;
			}
		}

		state.keyDown = this;
		result = kNoFlag;
	}

	return result;
}

void TextActor::keyboardEvent(const KeyboardEvent &event) {
	// TODO: Implement this once we have a title that actually uses it.
	warning("STUB: %s", __func__);
}

bool TextActor::hasScriptResponse(EventType eventType, const ScriptValue &arg) const {
	const Common::Array<ScriptResponse *> &scriptResponses = _scriptResponses.getValOrDefault(eventType);
	for (const ScriptResponse *scriptResponse : scriptResponses) {
		const ScriptValue &argToCheck = scriptResponse->_argumentValue;

		if (arg.getType() != argToCheck.getType()) {
			continue;
		}

		if (arg == argToCheck) {
			return true;
		}
	}
	return false;
}

void TextActor::setText() {
	// Remove double-quotes if they're the first or last characters.
	if (_text.firstChar() == '"') {
		_text.deleteChar(0);
	}
	if (_text.lastChar() == '"') {
		_text.deleteLastChar();
	}

	// Apply character translation if we have any.
	for (uint positionInString = 0; positionInString < _text.size(); positionInString++) {
		char currentChar = _text[positionInString];
		uint translatedChar = _acceptedChars.getValOrDefault(currentChar);
		if (translatedChar != 0) {
			_text.setChar(translatedChar, positionInString);
		}
	}
}

void TextActor::addAcceptedChars(uint firstCharCode, uint lastCharCode, uint charCodeOffset) {
	for (uint charCode = firstCharCode; charCode <= lastCharCode; charCode++) {
		_acceptedChars.setVal(charCode, charCodeOffset);
		if (charCodeOffset != 0) {
			charCodeOffset++;
		}
	}
}

int16 TextActor::calcStartingXPosition() {
	int16 xPos = 0;
	Common::Rect bounds = getBbox();
	if (_justification == kTextJustificationLeft) {
		xPos = bounds.left;

	} else if (_justification == kTextJustificationRight) {
		int16 textPixelLength = calcPixelLength(_text);
		xPos = bounds.right - textPixelLength;

	} else if (_justification == kTextJustificationCenter) {
		int16 textPixelLength = calcPixelLength(_text);
		int16 boundsMidwidth = bounds.left + (bounds.width() / 2);
		xPos = boundsMidwidth - (textPixelLength / 2);
	}
	return xPos;
}

int16 TextActor::calcBaseline() {
	int16 yPos = 0;
	Common::Rect bounds = getBbox();
	if (_position == kTextPositionTop) {
		yPos =  bounds.top + _fontActor->_maxAscent;

	} else if (_position == kTextPositionBottom) {
		yPos = bounds.bottom - _fontActor->_maxDescent;

	} else if (_position == kTextPositionMiddle) {
		int16 boundsMidheight = bounds.top + (bounds.height() / 2);
		yPos = boundsMidheight + (_fontActor->_maxAscent / 2);
	}
	return yPos;
}

void TextActor::drawCharacter(FontCharacter *glyph, int16 x, int16 y, DisplayContext &displayContext) {
	// Draw the glyph at the baseline position (y) minus its ascent.
	Common::Point destPoint(x, y - glyph->ascent());
	g_engine->getDisplayManager()->imageBlit(destPoint, glyph, _dissolveFactor, &displayContext);
}

void TextActor::drawCursor(int16 x, int16 y, DisplayContext &displayContext) {
	FontCharacter *cursorChar = _fontActor->lookupCharacter(CURSOR_CHAR_ID);
	if (cursorChar != nullptr) {
		drawCharacter(cursorChar, x, y, displayContext);
	}
}

int16 TextActor::calcPixelLength(const Common::String &text) {
	// Add the width of the main text.
	int16 totalXOffset = 0;
	for (uint positionInString = 0; positionInString < text.size(); positionInString++) {
		char currentChar = text[positionInString];

		FontCharacter *fontChar = _fontActor->lookupCharacter(currentChar);
		if (fontChar != nullptr) {
			totalXOffset += fontChar->_horizontalSpacing + fontChar->width();

		} else if (currentChar == ' ') {
			// As before, the space character might not have a defined font character.
			totalXOffset += _fontActor->_averageCharWidth;
		}
	}

	// Add cursor width if we are needing to show the cursor.
	if (_cursorPosition == _text.size() && _cursorIsVisible) {
		FontCharacter *cursorChar = _fontActor->lookupCharacter(CURSOR_CHAR_ID);
		if (cursorChar != nullptr) {
			totalXOffset += cursorChar->width();
		}
	}

	return totalXOffset;
}

} // End of namespace MediaStation
