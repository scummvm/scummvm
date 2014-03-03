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
#include "mads/compression.h"
#include "mads/graphics.h"
#include "mads/resources.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

ScreenObject::ScreenObject() {
	_category = CAT_NONE;
	_descId = 0;
	_layer = 0;
}

/*------------------------------------------------------------------------*/

ScreenObjects::ScreenObjects(MADSEngine *vm): _vm(vm) {
	_v8333C = false;
	_v832EC = 0;
	_yp = 0;
	_v7FECA = 0;
	_v7FED6 = 0;
	_v8332A = 0;
	_selectedObject = 0;
	_category = CAT_NONE;
	_objectIndex = 0;
	_released = false;
}

void ScreenObjects::add(const Common::Rect &bounds, ScrCategory category, int descId) {
	assert(size() < 100);

	ScreenObject so;
	so._bounds = bounds;
	so._category = category;
	so._descId = descId;
	so._layer = 0x1413;

	push_back(so);
}

void ScreenObjects::check(bool scanFlag) {
	Scene &scene = _vm->_game->_scene;

	if (!_vm->_events->_mouseButtons || _v832EC)
		_v7FECA = false;

	if ((_vm->_events->_vD6 || _v8332A || _yp || _v8333C) && scanFlag) {
		_selectedObject = scanBackwards(_vm->_events->currentPos(), LAYER_GUI);
		if (_selectedObject > 0) {
			_category = (ScrCategory)((*this)[_selectedObject - 1]._category & 7);
			_objectIndex = (*this)[_selectedObject - 1]._descId;
		}

		// Handling for easy mouse
		ScrCategory category = scene._interface._category;
		if (_vm->_easyMouse && !_vm->_events->_vD4 && category != _category
				&& scene._interface._category != CAT_NONE) {
			_released = true;
			if (category >= CAT_ACTION && category <= CAT_6) {
				scene._interface.elementHighlighted();
			} 
		}

		_released = _vm->_events->_mouseReleased;
		if (_vm->_events->_vD2 || (_vm->_easyMouse && !_vm->_events->_vD4))
			scene._interface._category = _category;

		if (!_vm->_events->_mouseButtons || _vm->_easyMouse) {
			if (category >= CAT_ACTION && category <= CAT_6) {
				scene._interface.elementHighlighted();
			}
		}

		if (_vm->_events->_mouseButtons || (_vm->_easyMouse && scene._action._v83338 > 1
				&& scene._interface._category == CAT_INV_LIST) ||
				(_vm->_easyMouse && scene._interface._category == CAT_HOTSPOT)) {
			scene._action.checkActionAtMousePos();
		}
			
		if (_vm->_events->_mouseReleased) {
			scene.leftClick();
			scene._interface._category = CAT_NONE;
		}

		if (_vm->_events->_mouseButtons || _vm->_easyMouse || _yp)
			proc1();

		if (_vm->_events->_mouseButtons || _vm->_easyMouse)
			scene._action.set();

		_v8333C = 0;
	}

	scene._action.refresh();

	// Loop through image inter list
	warning("TODO: iimageInterList loop");
}

int ScreenObjects::scanBackwards(const Common::Point &pt, int layer) {
	for (int i = (int)size() - 1; i >= 0; --i) {
		if ((*this)[i]._bounds.contains(pt) && ((*this)[i]._layer == layer))
			return i + 1;
	}

	// Entry not found
	return 0;
}

void ScreenObjects::proc1() {
	warning("TODO: ScreenObjects::proc1");
}

/*------------------------------------------------------------------------*/

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
	Player &player = _vm->_game->_player;

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

int SpriteSets::add(SpriteAsset *asset, int idx) {
	if (!idx)
		idx = size();

	if (idx >= (int)(size() + 1))
		resize(idx + 1);
	delete (*this)[idx];
	(*this)[idx] = asset;

	return idx;
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

DynamicHotspots::DynamicHotspots(MADSEngine *vm): _vm(vm) {
	_changed = false;
}

void DynamicHotspots::clear() {
	Common::Array<DynamicHotspot>::clear();
	_changed = false;
}

void DynamicHotspots::refresh() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0; idx < size(); ++idx) {
		DynamicHotspot &dh = (*this)[idx];

		switch (scene._screenObjects._v832EC) {
		case 0:
		case 2:
			scene._screenObjects.add(dh._bounds, CAT_12, dh._descId);
			scene._screenObjects._v8333C = true;
		default:
			break;
		}
	}
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

void ARTHeader::load(Common::SeekableReadStream *f) {
	// Read in dimensions of image
	_width = f->readUint16LE();
	_height = f->readUint16LE();

	// Read in palette information
	int palCount = f->readUint16LE();
	for (int i = 0; i < palCount; ++i) {
		RGB6 rgb;
		rgb.load(f);
		_palette.push_back(rgb);
	}
	f->skip(6 * (256 - palCount));

	// Read unknown???
	palCount = f->readUint16LE();
	for (int i = 0; i < palCount; ++i) {
		RGB4 rgb;
		rgb.r = f->readByte();
		rgb.g = f->readByte();
		rgb.b = f->readByte();
		rgb.u = f->readByte();

		_palAnimData.push_back(rgb);
	}
}

/*------------------------------------------------------------------------*/

void SceneNode::load(Common::SeekableReadStream *f) {
	_walkPos.x = f->readSint16LE();
	_walkPos.y = f->readSint16LE();
	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		_indexes[i] = f->readUint16LE();
}

/*------------------------------------------------------------------------*/

InterfaceSurface::InterfaceSurface(MADSEngine *vm): _vm(vm) {
	_category = CAT_NONE;
}

void InterfaceSurface::elementHighlighted() {
	warning("TODO: InterfaceSurface::elementHighlighted");
}

/*------------------------------------------------------------------------*/

void SceneInfo::SpriteInfo::load(Common::SeekableReadStream *f) {
	f->skip(3);
	_spriteSetIndex = f->readByte();
	f->skip(2);
	_position.x = f->readSint16LE();
	_position.y = f->readSint16LE();
	_depth = f->readByte();
	_scale = f->readByte();
}

/*------------------------------------------------------------------------*/

void InterfaceSurface::load(const Common::String &resName) {
	File f(resName);
	MadsPack madsPack(&f);

	// Load in the palette
	Common::SeekableReadStream *palStream = madsPack.getItemStream(0);

	RGB4 *gamePalP = &_vm->_palette->_gamePalette[0];
	byte *palP = &_vm->_palette->_mainPalette[0];

	for (int i = 0; i < 16; ++i, gamePalP++, palP += 3) {
		palP[0] = palStream->readByte();
		palP[1] = palStream->readByte();
		palP[2] = palStream->readByte();
		gamePalP->r |= 1;
		palStream->skip(3);
	}
	delete palStream;

	// set the size for the interface
	setSize(MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);
	Common::SeekableReadStream *pixelsStream = madsPack.getItemStream(1);
	pixelsStream->read(getData(), MADS_SCREEN_WIDTH * MADS_INTERFACE_HEIGHT);
	delete pixelsStream;
}

/*------------------------------------------------------------------------*/

SceneInfo *SceneInfo::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular) {
		return new SceneInfoNebular(vm);
	}
	else {
		return new SceneInfo(vm);
	}
}

void SceneInfo::load(int sceneId, int v1, const Common::String &resName,
		int flags, MSurface &depthSurface, MSurface &bgSurface) {
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
	_sceneId = infoStream->readUint16LE();
	_artFileNum = infoStream->readUint16LE();
	_depthStyle = infoStream->readUint16LE();
	_width = infoStream->readUint16LE();
	_height = infoStream->readUint16LE();

	infoStream->skip(24);

	int nodeCount = infoStream->readUint16LE();
	_yBandsEnd = infoStream->readUint16LE();
	_yBandsStart = infoStream->readUint16LE();
	_maxScale = infoStream->readUint16LE();
	_minScale = infoStream->readUint16LE();
	for (int i = 0; i < DEPTH_BANDS_SIZE; ++i)
		_depthList[i] = infoStream->readUint16LE();
	_field4A = infoStream->readUint16LE();

	// Load the set of objects that are associated with the scene
	for (int i = 0; i < 20; ++i) {
		SceneNode node;
		node.load(infoStream);
		
		if (i < nodeCount)
			_nodes.push_back(node);
	}

	int spriteSetsCount  = infoStream->readUint16LE();
	int spriteInfoCount = infoStream->readUint16LE();

	// Load in sprite sets 
	Common::StringArray setNames;
	for (int i = 0; i < 10; ++i) {
		char name[64];
		infoStream->read(name, 64);

		if (i < spriteSetsCount)
			setNames.push_back(Common::String(name));
	}

	// Load in sprite draw information
	Common::Array<SpriteInfo> spriteInfo;
	for (int i = 0; i < 50; ++i) {
		SpriteInfo info;
		info.load(infoStream);

		if (i < spriteInfoCount)
			spriteInfo.push_back(info);
	}

	delete infoStream;
	infoFile.close();

	int width = _width;
	int height = _height;

	if (!bgSurface.getPixels()) {
		bgSurface.setSize(width, height);
	}

	if (_depthStyle == 2)
		width >>= 2;
	if (!depthSurface.getPixels()) {
		depthSurface.setSize(width, height);
	}

	// Load the depth surface with the scene codes
	loadCodes(depthSurface);

	// Get the ART resource
	if (sceneFlag) {
		resourceName = Resources::formatName(RESPREFIX_RM, _artFileNum, ".ART");
	} else {
		resourceName = "*" + Resources::formatResource(resName, resName);
	}

	// Load in the ART header and palette
	File artFile(resourceName);
	MadsPack artResource(&artFile);
	Common::SeekableReadStream *stream = artResource.getItemStream(0);

	ARTHeader artHeader;
	artHeader.load(stream);
	delete stream;

	// Copy out the palette data
	for (uint i = 0; i < artHeader._palAnimData.size(); ++i)
		_palAnimData.push_back(artHeader._palAnimData[i]);

	if (!(flags & 1)) {
		if (!_vm->_palette->_paletteUsage.empty()) {
			_vm->_palette->_paletteUsage.getKeyEntries(artHeader._palette);
			_vm->_palette->_paletteUsage.prioritize(artHeader._palette);
		}

		_field4C = _vm->_palette->_paletteUsage.process(artHeader._palette, 0xF800);
		if (_field4C > 0) {
			_vm->_palette->_paletteUsage.transform(artHeader._palette);

			for (uint i = 0; i < _palAnimData.size(); ++i) {
				byte g = _palAnimData[i].g;
				_palAnimData[g].b = artHeader._palAnimData[g].u;
			}
		}
	}

	// Read in the background surface data
	assert(_width == bgSurface.w && _height == bgSurface.h);
	stream = artResource.getItemStream(1);
	stream->read(bgSurface.getPixels(), bgSurface.w * bgSurface.h);

	// Close the ART file
	delete stream;
	artFile.close();

	Common::Array<SpriteAsset *> spriteSets;
	Common::Array<int> indexList;

	if (flags & 1) {
		for (uint i = 0; i < setNames.size(); ++i) {
			Common::String setResName;
			if (sceneFlag || resName.hasPrefix("*"))
				setResName += "*";
			setResName += setNames[i];

			SpriteAsset *sprites = new SpriteAsset(_vm, setResName, flags);
			spriteSets.push_back(sprites);
			indexList.push_back(-1); // TODO:: sprites->_field6
		}
	}

	warning("TODO: sub_201E4(indexList, namesCount, &pal data2");

	for (uint i = 0; i < spriteInfo.size(); ++i) {
		SpriteInfo &si = spriteInfo[i];
		SpriteAsset *asset = spriteSets[si._spriteSetIndex];
		assert(asset && _depthStyle != 2);

		asset->drawScaled(asset->getCount(), depthSurface, bgSurface,
			si._scale, si._depth, si._position);
	}

	// Free the sprite sets
	for (int i = (int)spriteSets.size() - 1; i >= 0; --i) {
		warning("TODO: sub_201C8 SPRITE_SET.field_6");
		delete spriteSets[i];
	}
}

void SceneInfo::loadCodes(MSurface &depthSurface) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(0);

	uint16 width = _width;
	uint16 height = _height;
	byte *walkMap = new byte[stream->size()];

	depthSurface.setSize(width, height);
	stream->read(walkMap, f.size());
	delete stream;
	f.close();

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

void SceneInfoNebular::loadCodes(MSurface &depthSurface) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(0);

	byte *destP = depthSurface.getData();
	byte *endP = depthSurface.getBasePtr(0, depthSurface.h);

	byte runLength = stream->readByte();
	while (destP < endP && runLength > 0) {
		byte runValue = stream->readByte();

		// Write out the run length
		Common::fill(destP, destP + runLength, runValue);
		destP += runLength;

		// Get the next run length
		runLength = stream->readByte();
	}

	if (destP < endP)
		Common::fill(destP, endP, 0);
	delete stream;
	f.close();
}

} // End of namespace MADS
