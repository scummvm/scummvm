/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/scene_data.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/screen.h"
#include "mads/resources.h"
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

KernelMessage::KernelMessage() {
	_flags = 0;
	_sequenceIndex = 0;
	_color1 = 0;
	_color2 = 0;
	_msgOffset = 0;
	_numTicks = 0;
	_frameTimer2 = 0;
	_frameTimer = 0;
	_timeout = 0;
	_trigger = 0;
	_abortMode = SEQUENCE_TRIGGER_PARSER;
	_actionDetails._verbId = VERB_NONE;
	_actionDetails._objectNameId = 0;
	_actionDetails._indirectObjectId = 0;
	_textDisplayIndex = 0;
}

/*------------------------------------------------------------------------*/

void ARTHeader::load(Common::SeekableReadStream *f, bool isV2) {
	if (!isV2) {
		// Read in dimensions of image
		_width = f->readUint16LE();
		_height = f->readUint16LE();
	}

	// Read in palette information
	int palCount = f->readUint16LE();
	for (int i = 0; i < palCount; ++i) {
		RGB6 rgb;
		rgb.load(f);
		_palette.push_back(rgb);
	}
	f->skip(6 * (256 - palCount));

	// Read palette animations
	int cycleCount = f->readUint16LE();
	for (int i = 0; i < cycleCount; ++i) {
		PaletteCycle cycle;
		cycle._colorCount = f->readByte();
		cycle._firstListColor = f->readByte();
		cycle._firstColorIndex = f->readByte();
		cycle._ticks = f->readByte();

		_paletteCycles.push_back(cycle);
	}
}

/*------------------------------------------------------------------------*/

void SceneInfo::SpriteInfo::load(Common::SeekableReadStream *f) {
	f->skip(3);
	_spriteSetIndex = f->readByte();
	_frameNumber = f->readSByte();
	f->skip(1);
	_position.x = f->readSint16LE();
	_position.y = f->readSint16LE();
	_depth = f->readByte();
	_scale = f->readByte();
}

/*------------------------------------------------------------------------*/

SceneInfo::SceneInfo(MADSEngine *vm) : _vm(vm) {
	_sceneId = 0;
	_artFileNum = 0;
	_depthStyle = 0;
	_width = 0;
	_height = 0;
	_yBandsEnd = 0;
	_yBandsStart = 0;
	_maxScale = 0;
	_minScale = 0;
	_field4A = 0;
	_usageIndex = 0;
	for (int i = 0; i < DEPTH_BANDS_SIZE; ++i)
		_depthList[i] = 0;
}

SceneInfo *SceneInfo::init(MADSEngine *vm) {
	switch (vm->getGameID()) {
	case GType_RexNebular:
		return new Nebular::SceneInfoNebular(vm);
	case GType_Dragonsphere:
		return new Dragonsphere::SceneInfoDragonsphere(vm);
	case GType_Phantom:
		return new Phantom::SceneInfoPhantom(vm);
	default:
		error("SceneInfo: Unknown game");
	}

	return nullptr;
}

void SceneInfo::load(int sceneId, int variant, const Common::String &resName,
		int flags, DepthSurface &depthSurface, BaseSurface &bgSurface) {
	bool sceneFlag = sceneId >= 0;

	// Figure out the resource to use
	Common::String resourceName;
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".DAT");
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Open the scene info resource for access
	File infoFile(resourceName);
	MadsPack infoPack(&infoFile);

	// Read in basic data
	Common::SeekableReadStream *infoStream = infoPack.getItemStream(0);

	/*
	byte *data = new byte[infoStream->size()];
	infoStream->read(data, infoStream->size());
	Common::hexdump(data, infoStream->size());
	infoStream->seek(0);
	delete[] data;
	*/

	if (_vm->getGameID() == GType_RexNebular) {
		_sceneId = infoStream->readUint16LE();
	} else {
		infoStream->skip(6);	// actual scene ID (string)
		_sceneId = sceneId;
	}

	int nodeCount = 20;

	if (_vm->getGameID() == GType_RexNebular) {
		_artFileNum = infoStream->readUint16LE();
		_depthStyle = infoStream->readUint16LE();
		_width = infoStream->readUint16LE();
		_height = infoStream->readUint16LE();

		infoStream->skip(24);
	} else {
		_artFileNum = sceneId;
		_depthStyle = 0;
		_width = 320;
		_height = 156;

		infoStream->skip(98);
	}

	nodeCount = infoStream->readUint16LE();
	_yBandsEnd = infoStream->readUint16LE();
	_yBandsStart = infoStream->readUint16LE();
	_maxScale = infoStream->readUint16LE();
	_minScale = infoStream->readUint16LE();
	for (int i = 0; i < DEPTH_BANDS_SIZE; ++i)
		_depthList[i] = infoStream->readUint16LE();
	_field4A = infoStream->readUint16LE();

	// HACK for V2 games
	if (_vm->getGameID() != GType_RexNebular) {
		_minScale = _maxScale = 100;
		memset(_depthList, 0, DEPTH_BANDS_SIZE * sizeof(int));
	}

	// Load the scene's walk nodes
	for (int i = 0; i < 20; ++i) {
		WalkNode node;
		node.load(infoStream);

		if (i < nodeCount) {
			_nodes.push_back(node);
			//debug("Node %d: %d,%d", i, node._walkPos.x, node._walkPos.y);
		}
	}

	Common::StringArray setNames;
	Common::Array<SpriteInfo> spriteInfo;

	if (_vm->getGameID() == GType_RexNebular) {
		int spriteSetsCount = infoStream->readUint16LE();
		int spriteInfoCount = infoStream->readUint16LE();

		// Load in sprite sets
		for (int i = 0; i < 10; ++i) {
			char name[64];
			infoStream->read(name, 64);

			if (i < spriteSetsCount)
				setNames.push_back(Common::String(name));
		}

		// Load in sprite draw information
		for (int i = 0; i < 50; ++i) {
			SpriteInfo info;
			info.load(infoStream);

			if (i < spriteInfoCount)
				spriteInfo.push_back(info);
		}
	} else {
		uint16 shadowColors = infoStream->readUint16LE();
		uint16 shadowR = infoStream->readUint16LE();
		uint16 shadowG = infoStream->readUint16LE();
		uint16 shadowB = infoStream->readUint16LE();
		debug("Shadow colors: %d (%d, %d, %d)", shadowColors, shadowR, shadowG, shadowB);
	}
	delete infoStream;

	int width = _width;
	int height = _height;

	if (!bgSurface.getPixels() || (bgSurface.w != width) || (bgSurface.h != height)) {
		bgSurface.create(width, height);
	}

	if (_depthStyle == 2)
		width >>= 2;
	if (!depthSurface.getPixels()) {
		depthSurface.create(width, height);
	}

	loadCodes(depthSurface, variant);
	depthSurface._depthStyle = _depthStyle;
	infoFile.close();

	if (_vm->getGameID() == GType_RexNebular) {
		loadMadsV1Background(_artFileNum, resName, flags, bgSurface);
		loadPalette(_sceneId, _artFileNum, resName, flags, bgSurface);
	} else {
		loadMadsV2Background(_sceneId, resName, flags, bgSurface);
		loadPalette(_sceneId, _sceneId, resName, flags, bgSurface);
	}

	Common::Array<SpriteAsset *> spriteSets;
	Common::Array<int> usageList;

	// TODO: The following isn't quite right for V2 games
	if (_vm->getGameID() == GType_RexNebular) {
		for (uint i = 0; i < setNames.size(); ++i) {
			Common::String setResName;
			if (sceneFlag || resName.hasPrefix("*"))
				setResName += "*";
			setResName += setNames[i];

			SpriteAsset *sprites = new SpriteAsset(_vm, setResName, flags);
			spriteSets.push_back(sprites);
			usageList.push_back(sprites->_usageIndex);
		}
	}

	_vm->_palette->_paletteUsage.updateUsage(usageList, _usageIndex);

	for (uint i = 0; i < spriteInfo.size(); ++i) {
		SpriteInfo &si = spriteInfo[i];
		SpriteAsset *asset = spriteSets[si._spriteSetIndex];
		assert(asset && _depthStyle != 2);

		MSprite *spr = asset->getFrame(si._frameNumber);
		bgSurface.copyFrom(*spr, si._position, si._depth, &depthSurface,
			si._scale, false, spr->getTransparencyIndex());
	}

	// Free the sprite sets
	for (int i = (int)spriteSets.size() - 1; i >= 0; --i) {
		_vm->_palette->_paletteUsage.resetPalFlags(spriteSets[i]->_usageIndex);
		delete spriteSets[i];
	}
}

void SceneInfo::loadPalette(int sceneId, int artFileNum, const Common::String &resName, int flags, BaseSurface &bgSurface) {
	bool sceneFlag = sceneId >= 0;
	Common::String resourceName;
	bool isV2 = (_vm->getGameID() != GType_RexNebular);
	Common::String extension = !isV2 ? ".ART" : ".TT";
	int paletteStream = !isV2 ? 0 : 2;

	// Get the ART resource
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, artFileNum, extension);
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Load in the ART header and palette
	File artFile(resourceName);
	MadsPack artResource(&artFile);
	Common::SeekableReadStream *stream = artResource.getItemStream(paletteStream);

	ARTHeader artHeader;
	artHeader.load(stream, isV2);
	delete stream;

	// Copy out the palette animation data
	_paletteCycles.clear();
	for (uint i = 0; i < artHeader._paletteCycles.size(); ++i)
		_paletteCycles.push_back(artHeader._paletteCycles[i]);

	if (!(flags & 1)) {
		if (!_vm->_palette->_paletteUsage.empty()) {
			_vm->_palette->_paletteUsage.getKeyEntries(artHeader._palette);
			_vm->_palette->_paletteUsage.prioritize(artHeader._palette);
		}

		_usageIndex = _vm->_palette->_paletteUsage.process(artHeader._palette,
			(flags & 0xF800) | 0x8000);
		if (_usageIndex > 0) {
			_vm->_palette->_paletteUsage.transform(artHeader._palette);

			for (uint i = 0; i < _paletteCycles.size(); ++i) {
				byte listColor = _paletteCycles[i]._firstListColor;
				_paletteCycles[i]._firstColorIndex = artHeader._palette[listColor]._palIndex;
			}
		}
	}

	if (!(flags & 1)) {
		// Translate the background to use the correct palette indexes
		bgSurface.translate(artHeader._palette);
	}
}

void SceneInfo::loadMadsV1Background(int sceneId, const Common::String &resName, int flags, BaseSurface &bgSurface) {
	bool sceneFlag = sceneId >= 0;
	Common::String resourceName;
	Common::SeekableReadStream *stream;

	// Get the ART resource
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".ART");
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Load in the ART data
	File artFile(resourceName);
	MadsPack artResource(&artFile);

	// Read inhh the background surface data
	assert(_width  && _height == bgSurface.h);
	stream = artResource.getItemStream(1);
	stream->read(bgSurface.getPixels(), bgSurface.w * bgSurface.h);
	delete stream;

	if (flags & SCENEFLAG_TRANSLATE) {
		// Load in the palette and translate it
		Common::SeekableReadStream *palStream = artResource.getItemStream(0);
		Common::Array<RGB6> palette;

		_width = palStream->readUint16LE();
		_height = palStream->readUint16LE();

		int numColors = palStream->readUint16LE();
		assert(numColors <= 252);
		palette.resize(numColors);
		for (int i = 0; i < numColors; ++i)
			palette[i].load(palStream);
		delete palStream;

		// Translate the surface
		_vm->_palette->_paletteUsage.process(palette, 0);
		bgSurface.translate(palette);
	}

	// Close the ART file
	artFile.close();
}

void SceneInfo::loadMadsV2Background(int sceneId, const Common::String &resName, int flags, BaseSurface &bgSurface) {
	Common::String tileMapResourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".MM");
	File tileMapFile(tileMapResourceName);
	MadsPack tileMapPack(&tileMapFile);
	Common::SeekableReadStream *mapStream = tileMapPack.getItemStream(0);

	// Get the details of the tiles and map
	mapStream->readUint32LE();
	int tileCountX = mapStream->readUint16LE();
	int tileCountY = mapStream->readUint16LE();
	int tileWidthMap = mapStream->readUint16LE();
	int tileHeightMap = mapStream->readUint16LE();
	int screenWidth = mapStream->readUint16LE();
	int screenHeight = mapStream->readUint16LE();
	int tileCountMap = tileCountX * tileCountY;
	delete mapStream;

	// Obtain tile map information
	typedef Common::List<Common::SharedPtr<MSurface> > TileSetList;
	typedef TileSetList::iterator TileSetIterator;
	TileSetList tileSet;
	uint16 *tileMap = new uint16[tileCountMap];
	mapStream = tileMapPack.getItemStream(1);
	for (int i = 0; i < tileCountMap; ++i)
		tileMap[i] = mapStream->readUint16LE();
	delete mapStream;
	tileMapFile.close();

	// --------------------------------------------------------------------------------

	// Tile data, which needs to be kept compressed, as the tile map offsets refer to
	// the compressed data. Each tile is then uncompressed separately
	Common::String tileDataResourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".TT");
	File tileDataFile(tileDataResourceName);
	MadsPack tileDataPack(&tileDataFile);
	Common::SeekableReadStream *tileDataUncomp = tileDataPack.getItemStream(0);

	// Validate that the data matches between the tiles and tile map file and is valid
	int tileCount = tileDataUncomp->readUint16LE();
	int tileWidth = tileDataUncomp->readUint16LE();
	int tileHeight = tileDataUncomp->readUint16LE();
	delete tileDataUncomp;
	assert(tileCountMap == tileCount);
	assert(tileWidth == tileWidthMap);
	assert(tileHeight == tileHeightMap);
	assert(screenWidth == _width);
	assert(screenHeight <= _height);

	// Resize the background surface to hold all of the tiles
	uint16 newWidth = bgSurface.w;
	uint16 newHeight = bgSurface.h;

	if (tileWidth < screenWidth && bgSurface.w != tileCount * tileWidth)
		newWidth = tileCount * tileWidth;
	if (tileHeight < screenHeight && bgSurface.h != tileCount * tileHeight)
		newHeight = tileCount * tileHeight;

	if (bgSurface.w != newWidth || bgSurface.h != newHeight)
		bgSurface.create(newWidth, newHeight);

	// --------------------------------------------------------------------------------

	// Get tile data

	tileDataUncomp = tileDataPack.getItemStream(1);
	FabDecompressor fab;
	uint32 compressedTileDataSize = 0;

	for (int i = 0; i < tileCount; i++) {
		tileDataUncomp->seek(i * 4, SEEK_SET);
		uint32 tileOfs = tileDataUncomp->readUint32LE();
		MSurface* newTile = new MSurface(tileWidth, tileHeight);

		if (i == tileCount - 1)
			compressedTileDataSize = tileDataFile.size() - tileOfs;
		else
			compressedTileDataSize = tileDataUncomp->readUint32LE() - tileOfs;

		//debugCN(kDebugGraphics, "Tile: %i, compressed size: %i\n", i, compressedTileDataSize);

		newTile->clear();

		byte *compressedTileData = new byte[compressedTileDataSize];

		tileDataFile.seek(tileDataPack.getDataOffset() + tileOfs, SEEK_SET);
		tileDataFile.read(compressedTileData, compressedTileDataSize);

		fab.decompress(compressedTileData, compressedTileDataSize, (byte*)newTile->getPixels(), tileWidth * tileHeight);
		tileSet.push_back(TileSetList::value_type(newTile));
		delete[] compressedTileData;
	}

	delete tileDataUncomp;

	// --------------------------------------------------------------------------------

	// Loop through the mapping data to place the tiles on the screen

	uint16 *tIndex = &tileMap[0];
	for (int y = 0; y < tileCountY; y++) {
		for (int x = 0; x < tileCountX; x++) {
			int tileIndex = *tIndex++;
			assert(tileIndex < tileCount);
			TileSetIterator tile = tileSet.begin();
			for (int i = 0; i < tileIndex; i++)
				++tile;

			bgSurface.blitFrom(*(*tile).get(), Common::Point(x * tileWidth, y * tileHeight));
			((*tile).get())->free();
		}
	}
	tileSet.clear();
	tileDataFile.close();

	delete[] tileMap;
}

/*------------------------------------------------------------------------*/

SceneLogic::SceneLogic(MADSEngine *vm) : _vm(vm) {
	_scene = &_vm->_game->_scene;
}

} // End of namespace MADS
