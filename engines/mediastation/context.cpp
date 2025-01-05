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
#include "mediastation/assets/canvas.h"
#include "mediastation/assets/palette.h"
#include "mediastation/assets/image.h"
#include "mediastation/assets/path.h"
#include "mediastation/assets/sound.h"
#include "mediastation/assets/movie.h"
#include "mediastation/assets/sprite.h"
#include "mediastation/assets/hotspot.h"
#include "mediastation/assets/timer.h"

namespace MediaStation {

Context::Context(const Common::Path &path) :
	Datafile(path) {
	// This stuff isn't part of any graphics palette.
	readPreamble();

	// READ THE FIRST SUBFILE.
	Subfile subfile = Subfile(_stream);
	Chunk chunk = subfile.nextChunk();
	// First, read the header sections.
	if (g_engine->isFirstGenerationEngine()) {
		readOldStyleHeaderSections(subfile, chunk);
	} else {
		readNewStyleHeaderSections(subfile, chunk);
	}
	// Then, read any asset data.
	chunk = subfile._currentChunk;
	while (!subfile.atEnd()) {
		readAssetInFirstSubfile(chunk);
		if (!subfile.atEnd()) {
			chunk = subfile.nextChunk();
		}
	}

	// Then, assets in the rest of the subfiles.
	for (uint i = 1; i < _subfileCount; i++) {
		subfile = Subfile(_stream);
		readAssetFromLaterSubfile(subfile);
	}
}

Context::~Context() {
	delete _palette;
	_palette = nullptr;
	delete _parameters;
	_parameters = nullptr;
}

bool Context::readPreamble() {
	uint16 signature = _stream->readUint16LE();
	if (signature != 0x4949) { // "II"
		warning("Datafile::openFile(): Wrong signature for file %s. Got 0x%04X", _path.toString(Common::Path::kNativeSeparator).c_str(), signature);
		close();
		return false;
	}
	_stream->skip(2); // 0x00 0x00

	_unk1 = _stream->readUint32LE();
	debugC(5, kDebugLoading, "Context::openFile(): _unk1 = 0x%x", _unk1);

	_subfileCount = _stream->readUint32LE();
	// The total size of this file, including this header.
	// (Basically the true file size shown on the filesystem.)
	_fileSize = _stream->readUint32LE();
	return true;
}

void Context::readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	error("Context::readOldStyleHeaderSections(): Not implemented yet");
}

void Context::readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	// READ THE PALETTE.
	bool moreSectionsToRead = (chunk._id == MKTAG('i', 'g', 'o', 'd'));
	if (!moreSectionsToRead) {
		warning("Context::readNewStyleHeaderSections(): Got no header sections (@0x%llx)", static_cast<long long int>(chunk.pos()));
	}

	while (moreSectionsToRead) {
		// VERIFY THIS CHUNK IS A HEADER.
		// TODO: What are the situations when it's not?
		uint16 sectionType = Datum(chunk, kDatumTypeUint16_1).u.i;
		debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		bool chunkIsHeader = (sectionType == 0x000d);
		if (!chunkIsHeader) {
			error("Context::readNewStyleHeaderSections(): Expected header chunk, got %s (@0x%llx)", tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
		}

		// READ THIS HEADER SECTION.
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
	Asset *asset = g_engine->_assetsByChunkReference.getValOrDefault(chunk._id);
	if (asset == nullptr) {
		error("Context::readAssetInFirstSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
	}
	debugC(5, kDebugLoading, "\nContext::readAssetInFirstSubfile(): Got asset with chunk ID %s in first subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), static_cast<uint>(asset->type()), static_cast<long long int>(chunk.pos()));
	asset->readChunk(chunk);
}

void Context::readAssetFromLaterSubfile(Subfile &subfile) {
	Chunk chunk = subfile.nextChunk();
	Asset *asset = g_engine->_assetsByChunkReference.getValOrDefault(chunk._id);
	if (asset == nullptr) {
		error("Context::readAssetFromLaterSubfile(): Asset for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
	}
	debugC(5, kDebugLoading, "\nContext::readAssetFromLaterSubfile(): Got asset with chunk ID %s in later subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), asset->type(), static_cast<long long int>(chunk.pos()));
	asset->readSubfile(subfile, chunk);
}

bool Context::readHeaderSection(Subfile &subfile, Chunk &chunk) {
	uint16 sectionType = Datum(chunk, kDatumTypeUint16_1).u.i;
	debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kContextParametersSection: {
		if (_parameters != nullptr) {
			error("Context::readHeaderSection(): Got multiple parameters (@0x%llx)", static_cast<long long int>(chunk.pos()));
		}
		_parameters = new ContextParameters(chunk);
		break;
	}

	case kContextAssetLinkSection: {
		warning("Context::readHeaderSection(): ASSET_LINK not implemented yet");
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
			if (_screenAsset != nullptr) {
				error("Context::readHeaderSection(): Got multiple screen assets in the same context");
			}
			_screenAsset = header;
			break;

		default:
			error("Context::readHeaderSection(): No class for asset type 0x%x (@0x%llx)", static_cast<uint>(header->_type), static_cast<long long int>(chunk.pos()));
		}

		if (g_engine->_assets.contains(header->_id)) {
			error("Context::readHeaderSection(): Asset with ID 0x%d was already defined in this title", header->_id);
		}
		g_engine->_assets.setVal(header->_id, asset);
		if (header->_chunkReference != 0) {
			debugC(5, kDebugLoading, "Context::readHeaderSection(): Storing asset with chunk ID \"%s\" (0x%x)", tag2str(header->_chunkReference), header->_chunkReference);
			g_engine->_assetsByChunkReference.setVal(header->_chunkReference, asset);
		}
		// TODO: Store the movie chunk references better.
		if (header->_audioChunkReference != 0) {
			g_engine->_assetsByChunkReference.setVal(header->_audioChunkReference, asset);
		}
		if (header->_animationChunkReference != 0) {
			g_engine->_assetsByChunkReference.setVal(header->_animationChunkReference, asset);
		}
		// TODO: This datum only appears sometimes.
		uint unk2 = Datum(chunk).u.i;
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Got unknown value at end of asset header section 0x%x", unk2);
		break;
	}

	case kContextFunctionSection: {
		Function *function = new Function(chunk);
		g_engine->_functions.setVal(function->_id, function);
		if (!g_engine->isFirstGenerationEngine()) {
			uint endingFlag = Datum(chunk).u.i;
			if (endingFlag != 0) {
				warning("Context::readHeaderSection(): Got non-zero ending flag 0x%x in function section", endingFlag);
			}
		}
		break;
	}

	case kContextEndSection: {
		error("Context::readHeaderSection(): END Not implemented yet");
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

	default: {
		error("Context::readHeaderSection(): Unknown section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	}
	}

	return true;
}

} // End of namespace MediaStation
