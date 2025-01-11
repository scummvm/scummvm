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

#include "mediastation/datum.h"
#include "mediastation/assetheader.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

AssetHeader::AssetHeader(Chunk &chunk) {
	// I arbitrarily chose the bitmap as the default union member,
	// but they are all pointers so it doesn't matter.
	_fileNumber = Datum(chunk).u.i;
	// TODO: Cast to an asset type.
	_type = AssetType(Datum(chunk).u.i);
	_id = Datum(chunk).u.i;
	debugC(4, kDebugLoading, "AssetHeader::AssetHeader(): _type = 0x%x, _id = 0x%x (@0x%llx)", static_cast<uint>(_type), _id, static_cast<long long int>(chunk.pos()));

	AssetHeaderSectionType sectionType = getSectionType(chunk);
	bool moreSectionsToRead = (kAssetHeaderEmptySection != sectionType);
	while (moreSectionsToRead) {
		readSection(sectionType, chunk);
		sectionType = getSectionType(chunk);
		moreSectionsToRead = (kAssetHeaderEmptySection != sectionType);
	}
}

AssetHeader::~AssetHeader() {
	delete _boundingBox;
	_mouseActiveArea.clear();
	delete _palette;
	delete _name;
	delete _startPoint;
	delete _endPoint;
	delete _text;
}

void AssetHeader::readSection(AssetHeaderSectionType sectionType, Chunk& chunk) {
	debugC(5, kDebugLoading, "AssetHeader::AssetHeader(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kAssetHeaderEmptySection: {
		break;
	}

	case kAssetHeaderEventHandler: {
		EventHandler *eventHandler = new EventHandler(chunk);
		switch (eventHandler->_type) {
		case kTimerEvent: {
			_timeHandlers.push_back(eventHandler);
			break;
		}

		case kKeyDownEvent: {
			if (eventHandler->_argumentType != kAsciiCodeEventHandlerArgument) {
				error("Keydown event handler doesn't have correct argument type");
			}
			if (eventHandler->_argumentValue.t != kDatumTypeFloat64_2) {
				error("Keydown event handler doesn't have correct argument value type");
			}
			uint asciiCode = static_cast<uint>(eventHandler->_argumentValue.u.f);
			_keyDownHandlers.setVal(asciiCode, eventHandler);
			break;
		}

		case kInputEvent: {
			_inputHandlers.push_back(eventHandler);
			break;
		}

		case kLoadCompleteEvent: {
			_loadCompleteHandlers.push_back(eventHandler);
			break;
		}

		default: {
			if (eventHandler->_argumentType != kNullEventHandlerArgument && \
				eventHandler->_argumentType != kUnk1EventHandlerArgument) {
				error("AssetHeader::readSection(): Event handler of type %d has a non-null argument type %d", eventHandler->_type, eventHandler->_argumentType);
			}

			if (_eventHandlers.contains(eventHandler->_type)) {
				error("AssetHeader::readSection(): Event handler type %d already exists", eventHandler->_type);
			} else {
				_eventHandlers.setVal(eventHandler->_type, eventHandler);
			}
			break;
		}
		}
		break;
	}

	case kAssetHeaderStageId: {
		_stageId = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderAssetId: {
		// We already have this asset's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateAssetId = Datum(chunk).u.i;
		if (duplicateAssetId != _id) {
			warning("AssetHeader::readSection(): AssetHeader ID %d does not match original asset ID %d", duplicateAssetId, _id);
		}
		break;
	}

	case kAssetHeaderChunkReference: {
		// These are references to the chunk(s) that hold the data for this asset.
		// The references and the chunks have the following format "a501".
		// There is no guarantee where these chunk(s) might actually be located:
		//  - They might be in the same RIFF subfile as this header,
		//  - They might be in a different RIFF subfile in the same CXT file,
		//  - They might be in a different CXT file entirely.
		_chunkReference = Datum(chunk, kDatumTypeReference).u.i;
		break;
	}

	case kAssetHeaderMovieAudioChunkReference: {
		_audioChunkReference = Datum(chunk, kDatumTypeReference).u.i;
		break;
	}

	case kAssetHeaderMovieAnimationChunkReference: {
		_animationChunkReference = Datum(chunk, kDatumTypeReference).u.i;
		break;
	}

	case kAssetHeaderBoundingBox: {
		_boundingBox = Datum(chunk, kDatumTypeBoundingBox).u.bbox;
		break;
	}

	case kAssetHeaderMouseActiveArea: {
		uint16 total_points = Datum(chunk, kDatumTypeUint16_1).u.i;
		for (int i = 0; i < total_points; i++) {
			Common::Point *point = Datum(chunk, kDatumTypePoint2).u.point;
			_mouseActiveArea.push_back(point);
		}
		break;
	}

	case kAssetHeaderZIndex: {
		_zIndex = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderAssetReference: {
		_assetReference = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderStartup: {
		_startup = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderTransparency: {
		_transparency = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderHasOwnSubfile: {
		_hasOwnSubfile = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderCursorResourceId: {
		_cursorResourceId = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderFrameRate: {
		_frameRate = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderLoadType: {
		_loadType = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderSoundInfo: {
		_chunkCount = Datum(chunk).u.i;
		_rate = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderMovieLoadType: {
		_loadType = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderGetOffstageEvents: {
		_getOffstageEvents = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderPalette: {
		// TODO: Avoid the copying here!
		const uint PALETTE_ENTRIES = 256;
		const uint PALETTE_BYTES = PALETTE_ENTRIES * 3;
		byte* buffer = new byte[PALETTE_BYTES];
		chunk.read(buffer, PALETTE_BYTES);
		_palette = new Graphics::Palette(buffer, PALETTE_ENTRIES);
		delete[] buffer;
		break;
	}

	case kAssetHeaderDissolveFactor: {
		_dissolveFactor = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderSoundEncoding1:
	case kAssetHeaderSoundEncoding2: {
		_soundEncoding = SoundEncoding(Datum(chunk).u.i);
		break;
	}

	case kAssetHeaderSpriteChunkCount: {
		_chunkCount = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderStartPoint: {
		_startPoint = Datum(chunk, kDatumTypePoint2).u.point;
		break;
	}

	case kAssetHeaderEndPoint: {
		_endPoint = Datum(chunk, kDatumTypePoint2).u.point;
		break;
	}

	case kAssetHeaderStepRate: {
		_stepRate = (uint32)(Datum(chunk, kDatumTypeFloat64_2).u.f);
		break;
	}

	case kAssetHeaderDuration: {
		// These are stored in the file as fractional seconds,
		// but we want milliseconds.
		_duration = (uint32)(Datum(chunk).u.f * 1000);
		break;
	}

	case kAssetHeaderX: {
		_x = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderY: {
		_y = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderEditable: {
		_editable = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderFontId: {
		_fontAssetId = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderTextMaxLength: {
		_maxTextLength = Datum(chunk).u.i;
		break;
	}

	case kAssetHeaderInitialText: {
		_text = Datum(chunk).u.string;
		break;
	}

	case kAssetHeaderTextJustification: {
		_justification = static_cast<TextJustification>(Datum(chunk).u.i);
		break;
	}

	case kAssetHeaderTextPosition: {
		_position = static_cast<TextPosition>(Datum(chunk).u.i);
		break;
	}

	case kAssetHeaderTextCharacterClass: {
		CharacterClass characterClass;
		characterClass.firstAsciiCode = Datum(chunk).u.i;
		characterClass.lastAsciiCode = Datum(chunk).u.i;
		_acceptedInput.push_back(characterClass);
		break;
	}

	default: {
		error("AssetHeader::readSection(): Unknown section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		break;
	}
	}
}

AssetHeaderSectionType AssetHeader::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	AssetHeaderSectionType sectionType = static_cast<AssetHeaderSectionType>(datum.u.i);
	return sectionType;
}

} // end of namespace MediaStation
