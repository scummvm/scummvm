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

MADSEngine *DirtyArea::_vm = nullptr;

void DirtyArea::setArea(int width, int height, int maxWidth, int maxHeight) {
	if (_bounds.left % 2) {
		--_bounds.left;
		++width;
	}

	if (_bounds.left < 0)
		_bounds.left = 0;
	else if (_bounds.left > maxWidth)
		_bounds.left = maxWidth;
	int right = _bounds.left + width;
	if (right < 0)
		right = 0;
	if (right > maxWidth)
		right = maxWidth;

	_bounds.right = right;
	_bounds2.left = _bounds.width() / 2;
	_bounds2.right = _bounds.left + (_bounds.width() + 1) / 2 - 1;

	if (_bounds.top < 0)
		_bounds.top = 0;
	else if (_bounds.top > maxHeight)
		_bounds.top = maxHeight;
	int bottom = _bounds.top + height;
	if (bottom < 0)
		bottom = 0;
	if (bottom > maxHeight)
		bottom = maxHeight;

	_bounds.bottom = bottom;
	_bounds2.top = _bounds.height() / 2;
	_bounds2.bottom = _bounds.top + (_bounds.height() + 1) / 2 - 1;

	_active = true;
}


void DirtyArea::setSpriteSlot(const SpriteSlot *spriteSlot) {
	int width, height;
	Scene &scene = _vm->_game->_scene;

	if (spriteSlot->_spriteType == ST_FULL_SCREEN_REFRESH) {
		// Special entry to refresh the entire screen
		_bounds.left = 0;
		_bounds.top = 0;
		width = MADS_SCREEN_WIDTH;
		height = MADS_SCENE_HEIGHT;
	} else {
		// Standard sprite slots
		_bounds.left = spriteSlot->_position.x - scene._posAdjust.x;
		_bounds.top = spriteSlot->_position.y - scene._posAdjust.y;

		SpriteAsset &spriteSet = scene._spriteSlots.getSprite(spriteSlot->_spritesIndex);
		MSprite *frame = spriteSet.getFrame(((spriteSlot->_frameNumber & 0x7fff) - 1) & 0x7f);

		if (spriteSlot->_scale == -1) {
			width = frame->w;
			height = frame->h;
		}
		else {
			width = frame->w * spriteSlot->_scale / 100;
			height = frame->h * spriteSlot->_scale / 100;

			_bounds.left -= width / 2;
			_bounds.top += -(height - 1);
		}
	}

	setArea(width, height, MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
}

void DirtyArea::setTextDisplay(const TextDisplay *textDisplay) {
	_bounds.left = textDisplay->_bounds.left;
	_bounds.top = textDisplay->_bounds.top;

	setArea(textDisplay->_bounds.width(), textDisplay->_bounds.height(),
		MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
}

/*------------------------------------------------------------------------*/

DirtyAreas::DirtyAreas(MADSEngine *vm) : _vm(vm) {
	DirtyArea::_vm = vm;

	for (int i = 0; i < DIRTY_AREAS_SIZE; ++i) {
		DirtyArea rec;
		rec._active = false;
		push_back(rec);
	}
}

void DirtyAreas::merge(int startIndex, int count) {
	error("TODO: DirtyAreas::merge");
	if (startIndex >= count)
		return;

	for (int outerCtr = startIndex - 1, idx = 0; idx < count; ++outerCtr, ++idx) {
		if (!(*this)[outerCtr]._active)
			continue;

		for (int innerCtr = outerCtr + 1; innerCtr < count; ++innerCtr) {
			if (!(*this)[innerCtr]._active || !intersects(outerCtr, innerCtr))
				continue;

			if ((*this)[outerCtr]._textActive && (*this)[innerCtr]._textActive)
				mergeAreas(outerCtr, innerCtr);
		}
	}
}

/**
* Returns true if two dirty areas intersect
*/
bool DirtyAreas::intersects(int idx1, int idx2) {
	return (*this)[idx1]._bounds2.intersects((*this)[idx2]._bounds2);
}

void DirtyAreas::mergeAreas(int idx1, int idx2) {
	DirtyArea &da1 = (*this)[idx1];
	DirtyArea &da2 = (*this)[idx2];

	da1._bounds.extend(da2._bounds);

	da1._bounds2.left = da1._bounds.width() / 2;
	da1._bounds2.right = da1._bounds.left + (da1._bounds.width() + 1) / 2 - 1;
	da1._bounds2.top = da1._bounds.height() / 2;
	da1._bounds2.bottom = da1._bounds.top + (da1._bounds.height() + 1) / 2 - 1;

	da2._active = false;
	da1._textActive = true;
}

void DirtyAreas::copy(MSurface *dest, MSurface *src, const Common::Point &posAdjust) {
	for (uint i = 0; i < size(); ++i) {
		const Common::Rect &srcBounds = (*this)[i]._bounds;

		Common::Rect bounds(srcBounds.left + posAdjust.x, srcBounds.top + posAdjust.y,
			srcBounds.right + posAdjust.x, srcBounds.bottom + posAdjust.y);

		if ((*this)[i]._active && (*this)[i]._bounds.isValidRect()) {
			src->copyTo(dest, bounds, Common::Point((*this)[i]._bounds.left,
				(*this)[i]._bounds.top));
		}
	}
}

void DirtyAreas::reset() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._active = false;
}

/*------------------------------------------------------------------------*/

DynamicHotspot::DynamicHotspot() {
	_seqIndex = 0;
	_facing = 0;
	_descId = 0;
	_field14 = 0;
	_articleNumber = 0;
	_cursor = CURSOR_NONE;
}

/*------------------------------------------------------------------------*/

DynamicHotspots::DynamicHotspots(MADSEngine *vm): _vm(vm) {
	for (int i = 0; i < DYNAMIC_HOTSPOTS_SIZE; ++i) {
		DynamicHotspot rec;
		rec._active = false;
		_entries.push_back(rec);
	}

	_changed = true;
	_count = 0;
}

int DynamicHotspots::add(int descId, int field14, int seqIndex, const Common::Rect &bounds) {
	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && _entries[idx]._active)
		++idx;
	if (idx == _entries.size())
		error("DynamicHotspots overflow");

	_entries[idx]._active = true;
	_entries[idx]._descId = descId;
	_entries[idx]._seqIndex = seqIndex;
	_entries[idx]._bounds = bounds;
	_entries[idx]._feetPos.x = -3;
	_entries[idx]._feetPos.y = 0;
	_entries[idx]._facing = 5;
	_entries[idx]._field14 = field14;
	_entries[idx]._articleNumber = 6;
	_entries[idx]._cursor = CURSOR_NONE;

	++_count;
	_changed = true;

	if (seqIndex >= 0)
		_vm->_game->_scene._sequences[seqIndex]._dynamicHotspotIndex = idx;

	return idx;
}

int DynamicHotspots::setPosition(int index, int xp, int yp, int facing) {
	if (index >= 0) {
		_entries[index]._feetPos.x = xp;
		_entries[index]._feetPos.y = yp;
		_entries[index]._facing = facing;
	}

	return index;
}

int DynamicHotspots::setCursor(int index, CursorType cursor) {
	if (index >= 0)
		_entries[index]._cursor = cursor;

	return index;
}

void DynamicHotspots::remove(int index) {
	Scene &scene = _vm->_game->_scene;

	if (_entries[index]._active) {
		if (_entries[index]._seqIndex >= 0)
			scene._sequences[_entries[index]._seqIndex]._dynamicHotspotIndex = -1;
		_entries[index]._active = false;

		--_count;
		_changed = true;
	}
}

void DynamicHotspots::clear() {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i]._active = false;

	_changed = false;
	_count = 0;
}

void DynamicHotspots::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	_count = 0;
	_changed = false;
}

void DynamicHotspots::refresh() {
	error("DynamicHotspots::refresh");
}

/*------------------------------------------------------------------------*/

KernelMessage::KernelMessage() {
	_flags = 0;
	_sequenceIndex = 0;
	_asciiChar = '\0';
	_asciiChar2 = '\0';
	_color1 = 0;
	_color2 = 0;
	_msgOffset = 0;
	_numTicks = 0;
	_frameTimer2 = 0;
	_frameTimer = 0;
	_timeout = 0;
	_abortTimers = 0;
	_abortMode = ABORTMODE_0;
	_actionDetails._verbId = 0;
	_actionDetails._objectNameId = 0;
	_actionDetails._indirectObjectId = 0;
}

/*------------------------------------------------------------------------*/

Hotspot::Hotspot() {
	_facing = 0;
	_articleNumber = 0;
	_cursor = CURSOR_NONE;
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
	_cursor = (CursorType)f.readByte();
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
