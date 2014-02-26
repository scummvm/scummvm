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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/scene_data.h"
#include "mads/mads.h"
#include "mads/resources.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

SpriteSlot::SpriteSlot() {
	_spriteType = ST_NONE;
	_seqIndex = 0;
	_spritesIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

SpriteSlot::SpriteSlot(SpriteType type, int seqIndex) {
	_spriteType = type;
	_seqIndex = seqIndex;
	_spritesIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

/*------------------------------------------------------------------------*/

void SpriteSlots::clear(bool flag) {
	_vm->_game->_scene._textDisplay.clear();

	if (flag)
		_vm->_game->_scene._sprites.clear();

	Common::Array<SpriteSlot>::clear();
	push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

/**
 * Releases any sprites used by the player
 */
void SpriteSlots::releasePlayerSprites() {
	Player &player = _vm->_game->player();

	if (player._spritesLoaded && player._numSprites > 0) {
		int spriteEnd = player._spritesStart + player._numSprites - 1;
		do {
			deleteEntry(spriteEnd);
		} while (--spriteEnd >= player._spritesStart);
	}
}

void SpriteSlots::deleteEntry(int index) {
	remove_at(index);
}


/*------------------------------------------------------------------------*/

TextDisplay::TextDisplay() {
	_active = false;
	_spacing = 0;
	_expire = 0;
	_col1 = _col2 = 0;
}

/*------------------------------------------------------------------------*/

DynamicHotspot::DynamicHotspot() {
	_seqIndex = 0;
	_facing = 0;
	_descId = 0;
	_field14 = 0;
	_articleNumber = 0;
	_cursor = 0;
}

/*------------------------------------------------------------------------*/

SequenceEntry::SequenceEntry() {
	_spritesIndex = 0;
	_flipped =0;
	_frameIndex = 0;
	_frameStart = 0;
	_numSprites = 0;
	_animType = 0;
	_frameInc = 0;
	_depth = 0;
	_scale = 0;
	_dynamicHotspotIndex = -1;
	_triggerCountdown = 0;
	_doneFlag = 0;
	_entries._count = 0;
	_abortMode = 0;
	_actionNouns[0] = _actionNouns[1] = _actionNouns[2] = 0;
	_numTicks = 0;
	_extraTicks = 0;
	_timeout = 0;
}

KernelMessage::KernelMessage() {
	_flags = 0;
	_seqInex = 0;
	_asciiChar = '\0';
	_asciiChar2 = '\0';
	_colors = 0;
	Common::Point _posiition;
	_msgOffset = 0;
	_numTicks = 0;
	_frameTimer2 = 0;
	_frameTimer = 0;
	_timeout = 0;
	_field1C = 0;
	_abortMode = 0;
	_nounList[0] = _nounList[1] = _nounList[2] = 0;
}

/*------------------------------------------------------------------------*/

Hotspot::Hotspot() {
	_facing = 0;
	_articleNumber = 0;
	_cursor = 0;
	_vocabId = 0;
	_verbId = 0;
}

Hotspot::Hotspot(Common::SeekableReadStream &f) {
	_bounds.left = f.readSint16LE();
	_bounds.top = f.readSint16LE();
	_bounds.right = f.readSint16LE();
	_bounds.bottom = f.readSint16LE();
	_feetPos.x = f.readSint16LE();
	_feetPos.y = f.readSint16LE();
	_facing = f.readByte();
	_articleNumber = f.readByte();
	f.skip(1);
	_cursor = f.readByte();
	_vocabId = f.readUint16LE();
	_verbId = f.readUint16LE();
}

/*------------------------------------------------------------------------*/

void ARTHeader::load(Common::SeekableReadStream &f) {
	_width = f.readUint16LE();
	_height = f.readUint16LE();

	_palCount = f.readUint16LE();
	for (int i = 0; i < 256; ++i) {
		RGB6 rgb;
		rgb.r = f.readByte();
		rgb.g = f.readByte();
		rgb.b = f.readByte();
		f.read(&rgb.unused[0], 3);

		_palette.push_back(rgb);
	}

	int palCount = f.readUint16LE();
	for (int i = 0; i < palCount; ++i) {
		RGB4 rgb;
		rgb.r = f.readByte();
		rgb.g = f.readByte();
		rgb.b = f.readByte();
		rgb.u = f.readByte();

		_palData.push_back(rgb);
	}
}

/*------------------------------------------------------------------------*/

SceneInfo *SceneInfo::load(MADSEngine *vm, int sceneId, int v1, const Common::String &resName,
		int v3, MSurface &depthSurface, MSurface &bgSurface) {
	return new SceneInfo(vm, sceneId, v1, resName, v3, depthSurface, bgSurface);
}

SceneInfo::SceneInfo(MADSEngine *vm, int sceneId, int v1, const Common::String &resName,
		int flags, MSurface &depthSurface, MSurface &bgSurface) {
	bool flag = true;
	bool sceneFlag = sceneId >= 0;
	bool ssFlag = false, wsFlag = false;
	int handle = 0;
	
	SpriteAsset *spriteSets[10];
	int xpList[10];
	Common::fill(&spriteSets[0], &spriteSets[10], (SpriteAsset *)nullptr);
	Common::fill(&xpList[0], &xpList[10], -1);

	// Figure out the resource to use
	Common::String resourceName;
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".DAT");
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Open the scene info resource for access
	File infoFile(resName);

	// Read in basic data
	_sceneId = infoFile.readUint16LE();
	_artFileNum = infoFile.readUint16LE();
	_depthStyle = infoFile.readUint16LE();
	_width = infoFile.readUint16LE();
	_height = infoFile.readUint16LE();
	infoFile.skip(24);
	_nodeCount = infoFile.readUint16LE();
	_yBandsEnd = infoFile.readUint16LE();
	_yBandsStart = infoFile.readUint16LE();
	_maxScale = infoFile.readUint16LE();
	_minScale = infoFile.readUint16LE();
	for (int i = 0; i < 15; ++i)
		_depthList[i] = infoFile.readUint16LE();
	_field4A = infoFile.readUint16LE();

	// Load the set of objects that are associated with the scene
	for (int i = 0; i < 20; ++i) {
		InventoryObject obj;
		obj.load(infoFile);
		_objects.push_back(obj);
	}

	int setCount  = infoFile.readUint16LE();
	int field40E = infoFile.readUint16LE();

	for (int i = 0; i < 20; ++i) {
		char name[64];
		infoFile.read(name, 64);
		_setNames.push_back(Common::String(name));
	}

	infoFile.close();
	int width = _width;
	int height = _height;

	if (!bgSurface.getPixels()) {
		bgSurface.setSize(width, height);
		ssFlag = true;
	}

	if (_depthStyle == 2)
		width >>= 2;
	if (!depthSurface.getPixels()) {
		depthSurface.setSize(width, height);
		wsFlag = true;
	}

	// Load the depth surface with the scene codes
	loadCodes(depthSurface);

	// Get the ART resource
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, sceneId, ".ART");
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Load in the ART header and palette
	File artFile(resourceName);
	ARTHeader artHeader;
	artHeader.load(artFile);
	artFile.close();

	// Copy out the palette data
	for (int i = 0; i < artHeader._palData.size(); ++i)
		_palette.push_back(artHeader._palData[i]);
/*
	if (!(flags & 1)) {
		if (_vm->_game->_scene->_scenePalette) {
			//_vm->_game->_scene->_scenePalette->clean(&artHeader._palCount);
			//_vm->_game->_scene->_scenePalette->process(&artHeader._palCount)
		}
	}
	*/
	warning("TODO");
}

void SceneInfo::loadCodes(MSurface &depthSurface) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));

	uint16 width = _width;
	uint16 height = _height;
	byte *walkMap = new byte[f.size()];

	depthSurface.setSize(width, height);
	f.read(walkMap, f.size());

	byte *ptr = (byte *)depthSurface.getPixels();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int ofs = x + (y * width);
			if ((walkMap[ofs / 8] << (ofs % 8)) & 0x80)
				*ptr++ = 1;		// walkable
			else
				*ptr++ = 0;
		}
	}

	delete[] walkMap;
}

/*------------------------------------------------------------------------*/

void ScenePalette::clean(int *palCount) {
	warning("TODO: ScenePalette::clean");
}

void ScenePalette::process(int *palCount) {
	warning("TODO: ScenePalette::process");
}

} // End of namespace MADS
