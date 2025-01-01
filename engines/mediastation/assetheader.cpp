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
	debugC(4, kDebugLoading, "AssetHeader::AssetHeader(): _type = 0x%x, _id = 0x%x (@0x%llx)", _type, _id, static_cast<long long int>(chunk.pos()));

	AssetHeader::SectionType sectionType = getSectionType(chunk);
	bool moreSectionsToRead = (AssetHeader::SectionType::EMPTY != sectionType);
	while (moreSectionsToRead) {
		readSection(sectionType, chunk);
		sectionType = getSectionType(chunk);
		moreSectionsToRead = (AssetHeader::SectionType::EMPTY != sectionType);
	}
}

AssetHeader::~AssetHeader() {
	delete _boundingBox;
	delete _mouseActiveArea;
	delete _palette;
	delete _name;
	delete _startPoint;
	delete _endPoint;
}

void AssetHeader::readSection(AssetHeader::SectionType sectionType, Chunk& chunk) {
	debugC(5, kDebugLoading, "AssetHeader::AssetHeader(): sectionType = 0x%x (@0x%llx)", sectionType, static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case AssetHeader::SectionType::EMPTY: {
		break;
	}

	case AssetHeader::SectionType::EVENT_HANDLER: {
		EventHandler *eventHandler = new EventHandler(chunk);
		switch (eventHandler->_type) {
		case EventHandler::Type::Time: {
			_timeHandlers.push_back(eventHandler);
			break;
		}

		case EventHandler::Type::KeyDown: {
			_keyDownHandlers.push_back(eventHandler);
			break;
		}

		case EventHandler::Type::Input: {
			_inputHandlers.push_back(eventHandler);
			break;
		}

		case EventHandler::Type::LoadComplete: {
			_loadCompleteHandlers.push_back(eventHandler);
			break;
		}

		default: {
			if (eventHandler->_argumentType != EventHandler::ArgumentType::Null && eventHandler->_argumentType != EventHandler::ArgumentType::Unk1) {
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

	case AssetHeader::SectionType::STAGE_ID: {
		_stageId = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::ASSET_ID: {
		// We already have this asset's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateAssetId = Datum(chunk).u.i;
		if (duplicateAssetId != _id) {
			warning("AssetHeader::readSection(): AssetHeader ID %d does not match original asset ID %d", duplicateAssetId, _id);
		}
		break;
	}

	case AssetHeader::SectionType::CHUNK_REFERENCE: {
		// These are references to the chunk(s) that hold the data for this asset.
		// The references and the chunks have the following format "a501".
		// There is no guarantee where these chunk(s) might actually be located:
		//  - They might be in the same RIFF subfile as this header,
		//  - They might be in a different RIFF subfile in the same CXT file,
		//  - They might be in a different CXT file entirely.
		_chunkReference = Datum(chunk, DatumType::REFERENCE).u.i;
		break;
	}

	case AssetHeader::SectionType::MOVIE_AUDIO_CHUNK_REFERENCE: {
		_audioChunkReference = Datum(chunk, DatumType::REFERENCE).u.i;
		break;
	}

	case AssetHeader::SectionType::MOVIE_ANIMATION_CHUNK_REFERENCE: {
		_animationChunkReference = Datum(chunk, DatumType::REFERENCE).u.i;
		break;
	}

	case AssetHeader::SectionType::BOUNDING_BOX: {
		_boundingBox = Datum(chunk, DatumType::BOUNDING_BOX).u.bbox;
		break;
	}

	case AssetHeader::SectionType::MOUSE_ACTIVE_AREA: {
		_mouseActiveArea = Datum(chunk, DatumType::POLYGON).u.polygon;
		break;
	}

	case AssetHeader::SectionType::Z_INDEX: {
		_zIndex = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::ASSET_REFERENCE: {
		_assetReference = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::STARTUP: {
		_startup = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::TRANSPARENCY: {
		_transparency = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::HAS_OWN_SUBFILE: {
		_hasOwnSubfile = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::CURSOR_RESOURCE_ID: {
		_cursorResourceId = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::FRAME_RATE: {
		_frameRate = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::LOAD_TYPE: {
		_loadType = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::SOUND_INFO: {
		_totalChunks = Datum(chunk).u.i;
		_rate = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::MOVIE_LOAD_TYPE: {
		_loadType = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::GET_OFFSTAGE_EVENTS: {
		_getOffstageEvents = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::PALETTE: {
		// TODO: Avoid the copying here!
		const uint PALETTE_ENTRIES = 256;
		const uint PALETTE_BYTES = PALETTE_ENTRIES * 3;
		byte* buffer = new byte[PALETTE_BYTES];
		chunk.read(buffer, PALETTE_BYTES);
		_palette = new Graphics::Palette(buffer, PALETTE_ENTRIES);
		delete[] buffer;
		break;
	}

	case AssetHeader::SectionType::DISSOLVE_FACTOR: {
		_dissolveFactor = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::SOUND_ENCODING_1:
	case AssetHeader::SectionType::SOUND_ENCODING_2: {
		_soundEncoding = SoundEncoding(Datum(chunk).u.i);
		break;
	}

	case AssetHeader::SectionType::SPRITE_CHUNK_COUNT: {
		_chunkCount = Datum(chunk).u.i;
		break;
	}

	case AssetHeader::SectionType::START_POINT: {
		_startPoint = Datum(chunk, DatumType::POINT_2).u.point;
		break;
	}

	case AssetHeader::SectionType::END_POINT: {
		_endPoint = Datum(chunk, DatumType::POINT_2).u.point;
		break;
	}

	case AssetHeader::SectionType::STEP_RATE: {
		_stepRate = (uint32)(Datum(chunk, DatumType::FLOAT64_2).u.f);
		break;
	}

	case AssetHeader::SectionType::DURATION: {
		// These are stored in the file as fractional seconds,
		// but we want milliseconds.
		_duration = (uint32)(Datum(chunk).u.f * 1000);
		break;
	}

	default: {
		error("AssetHeader::readSection(): Unknown section type 0x%x (@0x%llx)", sectionType, static_cast<long long int>(chunk.pos()));
		break;
	}
	}
}

AssetHeader::SectionType AssetHeader::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	AssetHeader::SectionType sectionType = static_cast<AssetHeader::SectionType>(datum.u.i);
	return sectionType;
}

} // end of namespace MediaStation
