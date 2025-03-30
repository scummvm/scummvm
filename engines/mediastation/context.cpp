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

#include "mediastation/mediastation.h"
#include "mediastation/context.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

#include "mediastation/bitmap.h"
#include "mediastation/mediascript/collection.h"
#include "mediastation/assets/canvas.h"
#include "mediastation/assets/palette.h"
#include "mediastation/assets/image.h"
#include "mediastation/assets/path.h"
#include "mediastation/assets/sound.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"
#include "mediastation/assets/hotspot.h"
#include "mediastation/assets/timer.h"
#include "mediastation/assets/screen.h"
#include "mediastation/assets/font.h"
#include "mediastation/assets/text.h"

namespace MediaStation {

Context::Context(const Common::Path &path) : Datafile(path) {
	uint32 signature = _handle->readUint32BE();
	if (signature != MKTAG('I', 'I', '\0', '\0')) {
		error("Context::Context(): Wrong signature for file %s: 0x%08x", _name.c_str(), signature);
	}

	_unk1 = _handle->readUint32LE();
	_subfileCount = _handle->readUint32LE();
	_fileSize = _handle->readUint32LE();
	debugC(5, kDebugLoading, "Context::Context(): _unk1 = 0x%x", _unk1);

	Subfile subfile = getNextSubfile();
	Chunk chunk = subfile.nextChunk();

	if (g_engine->isFirstGenerationEngine()) {
		readOldStyleHeaderSections(subfile, chunk);
	} else {
		readNewStyleHeaderSections(subfile, chunk);
	}

	chunk = subfile._currentChunk;
	while (!subfile.atEnd()) {
		readAssetInFirstSubfile(chunk);
		if (!subfile.atEnd()) {
			chunk = subfile.nextChunk();
		}
	}

	// Read assets in the rest of the subfiles.
	for (uint i = 1; i < _subfileCount; i++) {
		subfile = getNextSubfile();
		readAssetFromLaterSubfile(subfile);
	}

	// Some sprites and images don't have any image data themselves, they just
	// reference the same image data in another asset. So we need to check for
	// these and create the appropriate references.
	for (auto it = _assets.begin(); it != _assets.end(); ++it) {
		Asset *asset = it->_value;
		uint referencedAssetId = asset->getHeader()->_assetReference;
		if (referencedAssetId != 0) {
			switch (asset->getHeader()->_type) {
			case kAssetTypeImage: {
				Image *image = static_cast<Image *>(asset);
				Image *referencedImage = static_cast<Image *>(getAssetById(referencedAssetId));
				if (referencedImage == nullptr) {
					error("Context::Context(): Asset %d references non-existent asset %d", asset->getHeader()->_id, referencedAssetId);
				}
				image->_bitmap = referencedImage->_bitmap;
				break;
			}

			case kAssetTypeSprite: {
				Sprite *sprite = static_cast<Sprite *>(asset);
				Sprite *referencedSprite = static_cast<Sprite *>(getAssetById(referencedAssetId));
				if (referencedSprite == nullptr) {
					error("Context::Context(): Asset %d references non-existent asset %d", asset->getHeader()->_id, referencedAssetId);
				}
				sprite->_frames = referencedSprite->_frames;
				break;
			}

			default:
				error("Context::Context(): Asset type %d referenced, but reference not implemented yet", asset->getHeader()->_type);
			}
		}
	}
}

Context::~Context() {
	delete _palette;
	_palette = nullptr;

	delete _contextName;
	_contextName = nullptr;

	for (auto it = _assets.begin(); it != _assets.end(); ++it) {
		delete it->_value;
	}
	_assets.clear();
	// The same asset pointers are in here, so don't delete again.
	_assetsByChunkReference.clear();

	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();

	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();
}

Asset *Context::getAssetById(uint assetId) {
	return _assets.getValOrDefault(assetId);
}

Asset *Context::getAssetByChunkReference(uint chunkReference) {
	return _assetsByChunkReference.getValOrDefault(chunkReference);
}

Function *Context::getFunctionById(uint functionId) {
	return _functions.getValOrDefault(functionId);
}

ScriptValue *Context::getVariable(uint variableId) {
	return _variables.getValOrDefault(variableId);
}

void Context::registerActiveAssets() {
	for (auto it = _assets.begin(); it != _assets.end(); ++it) {
		if (it->_value->isActive()) {
			g_engine->addPlayingAsset(it->_value);
		}
	}
}

void Context::readParametersSection(Chunk &chunk) {
	_fileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;

	ContextParametersSectionType sectionType = static_cast<ContextParametersSectionType>(Datum(chunk, kDatumTypeUint16_1).u.i);
	while (sectionType != kContextParametersEmptySection) {
		debugC(5, kDebugLoading, "ContextParameters::ContextParameters: sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		switch (sectionType) {
		case kContextParametersName: {
			uint repeatedFileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;
			if (repeatedFileNumber != _fileNumber) {
				warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
			}
			_contextName = Datum(chunk, kDatumTypeString).u.string;

			uint endingFlag = Datum(chunk, kDatumTypeUint16_1).u.i;
			if (endingFlag != 0) {
				warning("ContextParameters::ContextParameters(): Got non-zero ending flag 0x%x", endingFlag);
			}
			break;
		}

		case kContextParametersFileNumber: {
			error("ContextParameters::ContextParameters(): Section type FILE_NUMBER not implemented yet");
			break;
		}

		case kContextParametersVariable: {
			readVariable(chunk);
			break;
		}

		case kContextParametersBytecode: {
			Function *function = new Function(chunk);
			_functions.setVal(function->_id, function);
			break;
		}

		default:
			error("ContextParameters::ContextParameters(): Unknown section type 0x%x", static_cast<uint>(sectionType));
		}

		sectionType = static_cast<ContextParametersSectionType>(Datum(chunk, kDatumTypeUint16_1).u.i);
	}
}

void Context::readVariable(Chunk &chunk) {
	uint repeatedFileNumber = Datum(chunk, kDatumTypeUint16_1).u.i;
	if (repeatedFileNumber != _fileNumber) {
		warning("Context::readVariable(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
	}

	uint id = Datum(chunk).u.i;
	if (g_engine->getVariable(id) != nullptr) {
		error("Context::readVariable(): Global variable %d already exists", id);
	}

	ScriptValue *value = new ScriptValue(&chunk);
	_variables.setVal(id, value);
	debugC(5, kDebugScript, "Context::readVariable(): Created global variable %d (type: %s)",
		id, scriptValueTypeToStr(value->getType()));
}

void Context::readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	error("Context::readOldStyleHeaderSections(): Not implemented yet");
}

void Context::readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	bool moreSectionsToRead = (chunk._id == MKTAG('i', 'g', 'o', 'd'));
	if (!moreSectionsToRead) {
		warning("Context::readNewStyleHeaderSections(): Got no header sections (@0x%llx)", static_cast<long long int>(chunk.pos()));
	}

	while (moreSectionsToRead) {
		// Verify this chunk is a header.
		// TODO: What are the situations when it's not?
		uint16 sectionType = Datum(chunk, kDatumTypeUint16_1).u.i;
		debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		bool chunkIsHeader = (sectionType == 0x000d);
		if (!chunkIsHeader) {
			error("Context::readNewStyleHeaderSections(): Expected header chunk, got %s (@0x%llx)", tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
		}

		// Read this header section.
		moreSectionsToRead = readHeaderSection(subfile, chunk);
		if (subfile.atEnd()) {
			break;
		} else {
			debugC(5, kDebugLoading, "\nContext::readNewStyleHeaderSections(): Getting next chunk (@0x%llx)", static_cast<long long int>(chunk.pos()));
			chunk = subfile.nextChunk();
			moreSectionsToRead = (chunk._id == MKTAG('i', 'g', 'o', 'd'));
		}
	}
	debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): Finished reading sections (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

void Context::readAssetInFirstSubfile(Chunk &chunk) {
	if (chunk._id == MKTAG('i', 'g', 'o', 'd')) {
		warning("Context::readAssetInFirstSubfile(): Skippping \"igod\" asset link chunk");
		chunk.skip(chunk.bytesRemaining());
		return;
	}

	// TODO: Make sure this is not an asset link.
	Asset *asset = getAssetByChunkReference(chunk._id);
	if (asset == nullptr) {
		// We should only need to look in the global scope when there is an
		// install cache (INSTALL.CXT).
		asset = g_engine->getAssetByChunkReference(chunk._id);
		if (asset == nullptr) {
			error("Context::readAssetInFirstSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
		}
	}
	debugC(5, kDebugLoading, "\nContext::readAssetInFirstSubfile(): Got asset with chunk ID %s in first subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), static_cast<uint>(asset->type()), static_cast<long long int>(chunk.pos()));
	asset->readChunk(chunk);
}

void Context::readAssetFromLaterSubfile(Subfile &subfile) {
	Chunk chunk = subfile.nextChunk();
	Asset *asset = getAssetByChunkReference(chunk._id);
	if (asset == nullptr) {
		// We should only need to look in the global scope when there is an
		// install cache (INSTALL.CXT).
		asset = g_engine->getAssetByChunkReference(chunk._id);
		if (asset == nullptr) {
			error("Context::readAssetFromLaterSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
		}
	}
	debugC(5, kDebugLoading, "\nContext::readAssetFromLaterSubfile(): Got asset with chunk ID %s in later subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), asset->type(), static_cast<long long int>(chunk.pos()));
	asset->readSubfile(subfile, chunk);
}

bool Context::readHeaderSection(Subfile &subfile, Chunk &chunk) {
	uint16 sectionType = Datum(chunk, kDatumTypeUint16_1).u.i;
	debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kContextParametersSection: {
		readParametersSection(chunk);
		break;
	}

	case kContextAssetLinkSection: {
		warning("Context::readHeaderSection(): ASSET_LINK not implemented yet");
		chunk.skip(chunk.bytesRemaining());
		break;
	}

	case kContextPaletteSection: {
		if (_palette != nullptr) {
			error("Context::readHeaderSection(): Got multiple palettes (@0x%llx)", static_cast<long long int>(chunk.pos()));
		}
		// TODO: Avoid the copying here!
		const uint PALETTE_ENTRIES = 256;
		const uint PALETTE_BYTES = PALETTE_ENTRIES * 3;
		byte* buffer = new byte[PALETTE_BYTES];
		chunk.read(buffer, PALETTE_BYTES);
		_palette = new Graphics::Palette(buffer, PALETTE_ENTRIES);
		delete[] buffer;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Read palette");
		// This is likely just an ending flag that we expect to be zero.
		uint endingFlag = Datum(chunk, kDatumTypeUint16_1).u.i;
		if (endingFlag != 0) {
			warning("Context::readHeaderSection(): Got non-zero ending flag 0x%x", endingFlag);
		}
		break;
	}

	case kContextAssetHeaderSection: {
		Asset *asset = nullptr;
		AssetHeader *header = new AssetHeader(chunk);
		switch (header->_type) {
		case kAssetTypeImage:
			asset = new Image(header);
			break;

		case kAssetTypeMovie:
			asset = new Movie(header);
			break;

		case kAssetTypeSound:
			asset = new Sound(header);
			break;

		case kAssetTypePalette:
			asset = new Palette(header);
			break;

		case kAssetTypePath:
			asset = new Path(header);
			break;

		case kAssetTypeTimer:
			asset = new Timer(header);
			break;

		case kAssetTypeHotspot:
			asset = new Hotspot(header);
			break;

		case kAssetTypeSprite:
			asset = new Sprite(header);
			break;

		case kAssetTypeCanvas:
			asset = new Canvas(header);
			break;

		case kAssetTypeScreen:
			asset = new Screen(header);
			_screenAsset = header;
			break;

		case kAssetTypeFont:
			asset = new Font(header);
			break;

		case kAssetTypeText:
			asset = new Text(header);
			break;

		default:
			error("Context::readHeaderSection(): No class for asset type 0x%x (@0x%llx)", static_cast<uint>(header->_type), static_cast<long long int>(chunk.pos()));
		}

		if (g_engine->getAssetById(header->_id)) {
			error("Context::readHeaderSection(): Asset with ID 0x%d was already defined in this title", header->_id);
		}
		_assets.setVal(header->_id, asset);
		if (header->_chunkReference != 0) {
			debugC(5, kDebugLoading, "Context::readHeaderSection(): Storing asset with chunk ID \"%s\" (0x%x)", tag2str(header->_chunkReference), header->_chunkReference);
			_assetsByChunkReference.setVal(header->_chunkReference, asset);
		}
		// TODO: Store the movie chunk references better.
		if (header->_audioChunkReference != 0) {
			_assetsByChunkReference.setVal(header->_audioChunkReference, asset);
		}
		if (header->_animationChunkReference != 0) {
			_assetsByChunkReference.setVal(header->_animationChunkReference, asset);
		}
		// TODO: This datum only appears sometimes.
		uint unk2 = Datum(chunk).u.i;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Got unknown value at end of asset header section 0x%x", unk2);
		break;
	}

	case kContextFunctionSection: {
		Function *function = new Function(chunk);
		_functions.setVal(function->_id, function);
		if (!g_engine->isFirstGenerationEngine()) {
			uint endingFlag = Datum(chunk).u.i;
			if (endingFlag != 0) {
				warning("Context::readHeaderSection(): Got non-zero ending flag 0x%x in function section", endingFlag);
			}
		}
		break;
	}

	case kContextUnkAtEndSection: {
		int unk1 = Datum(chunk).u.i;
		int unk2 = Datum(chunk).u.i;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): unk1 = %d, unk2 = %d", unk1, unk2);
		return false;
	}

	case kContextEmptySection: {
		error("Context::readHeaderSection(): EMPTY Not implemented yet");
		break;
	}

	case kContextPoohSection: {
		error("Context::readHeaderSection(): POOH Not implemented yet");
		break;
	}

	default:
		error("Context::readHeaderSection(): Unknown section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	}

	return true;
}

} // End of namespace MediaStation
