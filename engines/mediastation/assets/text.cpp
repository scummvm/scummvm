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

#include "mediastation/assets/text.h"

namespace MediaStation {

void Text::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderEditable:
		_editable = chunk.readTypedByte();
		break;

	case kAssetHeaderLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kAssetHeaderFontId:
		_fontAssetId = chunk.readTypedUint16();
		break;

	case kAssetHeaderTextMaxLength:
		_maxTextLength = chunk.readTypedUint16();
		break;

	case kAssetHeaderInitialText:
		_text = chunk.readTypedString();
		break;

	case kAssetHeaderTextJustification:
		_justification = static_cast<TextJustification>(chunk.readTypedUint16());
		break;

	case kAssetHeaderTextPosition:
		_position = static_cast<TextPosition>(chunk.readTypedUint16());
		break;

	case kAssetHeaderTextCharacterClass: {
		CharacterClass characterClass;
		characterClass.firstAsciiCode = chunk.readTypedUint16();
		characterClass.lastAsciiCode = chunk.readTypedUint16();
		_acceptedInput.push_back(characterClass);
		break;
	}

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue Text::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTextMethod: {
		assert(args.empty());
		error("Text::callMethod(): Text() method not implemented yet");
	}

	case kSetTextMethod: {
		assert(args.size() == 1);
		error("Text::callMethod(): getText() method not implemented yet");
	}

	case kSpatialShowMethod: {
		assert(args.empty());
		_isVisible = true;
		warning("Text::callMethod(): spatialShow method not implemented yet");
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		_isVisible = false;
		warning("Text::callMethod(): spatialHide method not implemented yet");
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

Common::String Text::text() const {
	return _text;
}

void Text::setText(Common::String text) {
	error("Text::setText(): Setting text not implemented yet");
}

} // End of namespace MediaStation
