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

#include "mediastation/assetheader.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

AssetHeader::AssetHeader(Chunk &chunk) {
	_fileNumber = chunk.readTypedUint16();
	_type = static_cast<AssetType>(chunk.readTypedUint16());
	_id = chunk.readTypedUint16();
	debugC(4, kDebugLoading, "AssetHeader::AssetHeader(): _type = 0x%x, _id = 0x%x (@0x%llx)", static_cast<uint>(_type), _id, static_cast<long long int>(chunk.pos()));

	AssetHeaderSectionType sectionType = static_cast<AssetHeaderSectionType>(chunk.readTypedUint16());
	bool moreSectionsToRead = (kAssetHeaderEmptySection != sectionType);
	while (moreSectionsToRead) {
		readSection(sectionType, chunk);
		sectionType = static_cast<AssetHeaderSectionType>(chunk.readTypedUint16());
		moreSectionsToRead = (kAssetHeaderEmptySection != sectionType);
	}
}

AssetHeader::~AssetHeader() {
	for (auto it = _eventHandlers.begin(); it != _eventHandlers.end(); ++it) {
		for (EventHandler *eventHandler : it->_value) {
			delete eventHandler;
		}
	}
	_eventHandlers.clear();

	delete _palette;
	_palette = nullptr;
}

void AssetHeader::readSection(AssetHeaderSectionType sectionType, Chunk& chunk) {
	debugC(5, kDebugLoading, "AssetHeader::AssetHeader(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kAssetHeaderEmptySection: {
		break;
	}

	case kAssetHeaderEventHandler: {
		EventHandler *eventHandler = new EventHandler(chunk);
		Common::Array<EventHandler *> &eventHandlersForType = _eventHandlers.getOrCreateVal(eventHandler->_type);

		// This is not a hashmap because we don't want to have to hash ScriptValues.
		for (EventHandler *existingEventHandler : eventHandlersForType) {
			if (existingEventHandler->_argumentValue == eventHandler->_argumentValue) {
				error("AssetHeader::readSection(): Event handler for %s (%s) already exists",
					eventTypeToStr(eventHandler->_type), eventHandler->getDebugHeader().c_str());
			}
		}
		eventHandlersForType.push_back(eventHandler);
		break;
	}

	case kAssetHeaderStageId: {
		_stageId = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderAssetId: {
		// We already have this asset's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateAssetId = chunk.readTypedUint16();
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
		_chunkReference = chunk.readTypedChunkReference();
		break;
	}

	case kAssetHeaderMovieAudioChunkReference: {
		_audioChunkReference = chunk.readTypedChunkReference();
		break;
	}

	case kAssetHeaderMovieAnimationChunkReference: {
		_animationChunkReference = chunk.readTypedChunkReference();
		break;
	}

	case kAssetHeaderBoundingBox: {
		_boundingBox = chunk.readTypedRect();
		break;
	}

	case kAssetHeaderMouseActiveArea: {
		uint16 total_points = chunk.readTypedUint16();
		for (int i = 0; i < total_points; i++) {
			Common::Point point = chunk.readTypedPoint();
			_mouseActiveArea.push_back(point);
		}
		break;
	}

	case kAssetHeaderZIndex: {
		_zIndex = chunk.readTypedGraphicUnit();
		break;
	}

	case kAssetHeaderAssetReference: {
		_assetReference = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderStartup: {
		_startup = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderTransparency: {
		_transparency = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderHasOwnSubfile: {
		_hasOwnSubfile = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderCursorResourceId: {
		_cursorResourceId = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderFrameRate: {
		_frameRate = static_cast<uint32>(chunk.readTypedDouble());
		break;
	}

	case kAssetHeaderLoadType: {
		_loadType = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderSoundInfo: {
		_chunkCount = chunk.readTypedUint16();
		_rate = chunk.readTypedUint32();
		break;
	}

	case kAssetHeaderMovieLoadType: {
		_loadType = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderGetOffstageEvents: {
		_getOffstageEvents = chunk.readTypedByte();
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
		_dissolveFactor = chunk.readTypedDouble();
		break;
	}

	case kAssetHeaderSoundEncoding1:
	case kAssetHeaderSoundEncoding2: {
		_soundEncoding = static_cast<SoundEncoding>(chunk.readTypedUint16());
		break;
	}

	case kAssetHeaderSpriteChunkCount: {
		_chunkCount = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderStartPoint: {
		_startPoint = chunk.readTypedPoint();
		break;
	}

	case kAssetHeaderEndPoint: {
		_endPoint = chunk.readTypedPoint();
		break;
	}

	case kAssetHeaderStepRate: {
		double _stepRateFloat = chunk.readTypedDouble();
		// This should always be an integer anyway,
		// so we'll cast away any fractional part.
		_stepRate = static_cast<uint32>(_stepRateFloat);
		break;
	}

	case kAssetHeaderDuration: {
		// These are stored in the file as fractional seconds,
		// but we want milliseconds.
		_duration = (uint32)(chunk.readTypedTime() * 1000);
		break;
	}

	case kAssetHeaderX: {
		_x = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderY: {
		_y = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderEditable: {
		_editable = chunk.readTypedByte();
		break;
	}

	case kAssetHeaderFontId: {
		_fontAssetId = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderTextMaxLength: {
		_maxTextLength = chunk.readTypedUint16();
		break;
	}

	case kAssetHeaderInitialText: {
		_text = chunk.readTypedString();
		break;
	}

	case kAssetHeaderTextJustification: {
		_justification = static_cast<TextJustification>(chunk.readTypedUint16());
		break;
	}

	case kAssetHeaderTextPosition: {
		_position = static_cast<TextPosition>(chunk.readTypedUint16());
		break;
	}

	case kAssetHeaderTextCharacterClass: {
		CharacterClass characterClass;
		characterClass.firstAsciiCode = chunk.readTypedUint16();
		characterClass.lastAsciiCode = chunk.readTypedUint16();
		_acceptedInput.push_back(characterClass);
		break;
	}

	case kAssetHeaderSpriteFrameMapping: {
		uint32 externalFrameId = chunk.readTypedUint16();
		uint32 internalFrameId = chunk.readTypedUint16();
		uint32 unk1 = chunk.readTypedUint16();
		if (unk1 != internalFrameId) {
			warning("AssetHeader::readSection(): Repeated internalFrameId doesn't match");
		}
		_spriteFrameMapping.setVal(externalFrameId, internalFrameId);
		break;
	}

    case kAssetHeaderPathTotalSteps: {
		_totalSteps = chunk.readTypedUint16();
		break;
	}

	default:
		error("AssetHeader::readSection(): Unknown section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	}
}

} // end of namespace MediaStation
