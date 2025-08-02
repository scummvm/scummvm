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

namespace MediaStation {

void TextActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderEditable:
		_editable = chunk.readTypedByte();
		break;

	case kActorHeaderLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kActorHeaderFontId:
		_fontActorId = chunk.readTypedUint16();
		break;

	case kActorHeaderTextMaxLength:
		_maxTextLength = chunk.readTypedUint16();
		break;

	case kActorHeaderInitialText:
		_text = chunk.readTypedString();
		break;

	case kActorHeaderTextJustification:
		_justification = static_cast<TextJustification>(chunk.readTypedUint16());
		break;

	case kActorHeaderTextPosition:
		_position = static_cast<TextPosition>(chunk.readTypedUint16());
		break;

	case kActorHeaderTextCharacterClass: {
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

ScriptValue TextActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTextMethod: {
		assert(args.empty());
		error("%s: Text() method not implemented yet", __func__);
	}

	case kSetTextMethod: {
		assert(args.size() == 1);
		error("%s: getText() method not implemented yet", __func__);
	}

	case kSpatialShowMethod: {
		assert(args.empty());
		_isVisible = true;
		warning("%s: spatialShow method not implemented yet", __func__);
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		_isVisible = false;
		warning("%s: spatialHide method not implemented yet", __func__);
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

Common::String TextActor::text() const {
	return _text;
}

void TextActor::setText(Common::String text) {
	error("%s: Setting text not implemented yet", __func__);
}

} // End of namespace MediaStation
