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
		uint referencedAssetId = asset->_assetReference;
		if (referencedAssetId != 0) {
			switch (asset->type()) {
			case kAssetTypeImage: {
				Image *image = static_cast<Image *>(asset);
				Image *referencedImage = static_cast<Image *>(getAssetById(referencedAssetId));
				if (referencedImage == nullptr) {
					error("Context::Context(): Asset %d references non-existent asset %d", asset->id(), referencedAssetId);
				}
				image->_bitmap = referencedImage->_bitmap;
				break;
			}

			case kAssetTypeSprite: {
				Sprite *sprite = static_cast<Sprite *>(asset);
				Sprite *referencedSprite = static_cast<Sprite *>(getAssetById(referencedAssetId));
				if (referencedSprite == nullptr) {
					error("Context::Context(): Asset %d references non-existent asset %d", asset->id(), referencedAssetId);
				}
				sprite->_frames = referencedSprite->_frames;
				sprite->_clips = referencedSprite->_clips;
				break;
			}

			default:
				error("Context::Context(): Asset type %d referenced, but reference not implemented yet", asset->type());
			}
		}
	}
}

Context::~Context() {
	delete _palette;
	_palette = nullptr;

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

void Context::readCreateContextData(Chunk &chunk) {
	_fileNumber = chunk.readTypedUint16();

	ContextParametersSectionType sectionType = static_cast<ContextParametersSectionType>(chunk.readTypedUint16());
	while (sectionType != kContextParametersEmptySection) {
		debugC(5, kDebugLoading, "ContextParameters::ContextParameters: sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		switch (sectionType) {
		case kContextParametersName: {
			uint repeatedFileNumber = chunk.readTypedUint16();
			if (repeatedFileNumber != _fileNumber) {
				warning("ContextParameters::ContextParameters(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
			}
			_contextName = chunk.readTypedString();

			uint endingFlag = chunk.readTypedUint16();
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
			readCreateVariableData(chunk);
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

		sectionType = static_cast<ContextParametersSectionType>(chunk.readTypedUint16());
	}
}

Asset *Context::readCreateAssetData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	AssetType type = static_cast<AssetType>(chunk.readTypedUint16());
	uint id = chunk.readTypedUint16();
	debugC(4, kDebugLoading, "_type = 0x%x, _id = 0x%x", static_cast<uint>(type), id);

	Asset *asset = nullptr;
	switch (type) {
	case kAssetTypeImage:
		asset = new Image();
		break;

	case kAssetTypeMovie:
		asset = new Movie();
		break;

	case kAssetTypeSound:
		asset = new Sound();
		break;

	case kAssetTypePalette:
		asset = new Palette();
		break;

	case kAssetTypePath:
		asset = new Path();
		break;

	case kAssetTypeTimer:
		asset = new Timer();
		break;

	case kAssetTypeHotspot:
		asset = new Hotspot();
		break;

	case kAssetTypeSprite:
		asset = new Sprite();
		break;

	case kAssetTypeCanvas:
		asset = new Canvas();
		break;

	case kAssetTypeScreen:
		asset = new Screen();
		_screenAsset = static_cast<Screen *>(asset);
		break;

	case kAssetTypeFont:
		asset = new Font();
		break;

	case kAssetTypeText:
		asset = new Text();
		break;

	default:
		error("No class for asset type 0x%x (@0x%llx)", static_cast<uint>(type), static_cast<long long int>(chunk.pos()));
	}
	asset->setId(id);
	asset->setContextId(contextId);
	asset->initFromParameterStream(chunk);
	return asset;
}

void Context::readCreateVariableData(Chunk &chunk) {
	uint repeatedFileNumber = chunk.readTypedUint16();
	if (repeatedFileNumber != _fileNumber) {
		warning("Context::readCreateVariableData(): Repeated file number didn't match: %d != %d", repeatedFileNumber, _fileNumber);
	}

	uint id = chunk.readTypedUint16();
	if (g_engine->getVariable(id) != nullptr) {
		error("Global variable %d already exists", id);
	}

	ScriptValue *value = new ScriptValue(&chunk);
	_variables.setVal(id, value);
	debugC(5, kDebugScript, "Created global variable %d (type: %s)",
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
		uint16 sectionType = chunk.readTypedUint16();
		debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		bool chunkIsHeader = (sectionType == 0x000d);
		if (!chunkIsHeader) {
			error("Context::readNewStyleHeaderSections(): Expected header chunk, got %s (@0x%llx)", tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
		}

		// Read this header section.
		moreSectionsToRead = readHeaderSection(chunk);
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

bool Context::readHeaderSection(Chunk &chunk) {
	uint16 sectionType = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kContextParametersSection: {
		readCreateContextData(chunk);
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
		byte *buffer = new byte[PALETTE_BYTES];
		chunk.read(buffer, PALETTE_BYTES);
		_palette = new Graphics::Palette(buffer, PALETTE_ENTRIES);
		delete[] buffer;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Read palette");
		// This is likely just an ending flag that we expect to be zero.
		uint endingFlag = chunk.readTypedUint16();
		if (endingFlag != 0) {
			warning("Context::readHeaderSection(): Got non-zero ending flag 0x%x", endingFlag);
		}
		break;
	}

	case kContextAssetHeaderSection: {
		Asset *asset = readCreateAssetData(chunk);
		_assets.setVal(asset->id(), asset);
		g_engine->registerAsset(asset);
		if (asset->_chunkReference != 0) {
			debugC(5, kDebugLoading, "Context::readHeaderSection(): Storing asset with chunk ID \"%s\" (0x%x)", tag2str(asset->_chunkReference), asset->_chunkReference);
			_assetsByChunkReference.setVal(asset->_chunkReference, asset);
		}

		if (asset->type() == kAssetTypeMovie) {
			Movie *movie = static_cast<Movie *>(asset);
			if (movie->_audioChunkReference != 0) {
				_assetsByChunkReference.setVal(movie->_audioChunkReference, asset);
			}
			if (movie->_animationChunkReference != 0) {
				_assetsByChunkReference.setVal(movie->_animationChunkReference, asset);
			}
		}
		// TODO: This datum only appears sometimes.
		uint unk2 = chunk.readTypedUint16();
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Got unknown value at end of asset header section 0x%x", unk2);
		break;
	}

	case kContextFunctionSection: {
		Function *function = new Function(chunk);
		_functions.setVal(function->_id, function);
		if (!g_engine->isFirstGenerationEngine()) {
			uint endingFlag = chunk.readTypedUint16();
			if (endingFlag != 0) {
				warning("Context::readHeaderSection(): Got non-zero ending flag 0x%x in function section", endingFlag);
			}
		}
		break;
	}

	case kContextUnkAtEndSection: {
		int unk1 = chunk.readTypedUint16();
		int unk2 = chunk.readTypedUint16();
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
