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
#include "mads/screen.h"
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
	_v7FECA = 0;
	_v7FED6 = 0;
	_v8332A = 0;
	_category = CAT_NONE;
	_objectIndex = 0;
	_released = false;
}

void ScreenObjects::add(const Common::Rect &bounds, Layer layer, ScrCategory category, int descId) {
	assert(size() < 100);

	ScreenObject so;
	so._bounds = bounds;
	so._category = category;
	so._descId = descId;
	so._layer = layer;

	push_back(so);
}

void ScreenObjects::check(bool scanFlag) {
	Scene &scene = _vm->_game->_scene;

	if (!_vm->_events->_mouseButtons || _v832EC)
		_v7FECA = false;

	if ((_vm->_events->_vD6 || _v8332A || _vm->_game->_scene._userInterface._scrollerY || _v8333C) && scanFlag) {
		scene._userInterface._selectedInvIndex = scanBackwards(_vm->_events->currentPos(), LAYER_GUI);
		if (scene._userInterface._selectedInvIndex > 0) {
			_category = (ScrCategory)((*this)[scene._userInterface._selectedInvIndex - 1]._category & 7);
			_objectIndex = (*this)[scene._userInterface._selectedInvIndex - 1]._descId;
		}

		// Handling for easy mouse
		ScrCategory category = scene._userInterface._category;
		if (_vm->_easyMouse && !_vm->_events->_vD4 && category != _category
				&& scene._userInterface._category != CAT_NONE) {
			_released = true;
			if (category >= CAT_ACTION && category <= CAT_TALK_ENTRY) {
				scene._userInterface.elementHighlighted();
			} 
		}

		_released = _vm->_events->_mouseReleased;
		if (_vm->_events->_vD2 || (_vm->_easyMouse && !_vm->_events->_vD4))
			scene._userInterface._category = _category;

		if (!_vm->_events->_mouseButtons || _vm->_easyMouse) {
			if (category >= CAT_ACTION && category <= CAT_TALK_ENTRY) {
				scene._userInterface.elementHighlighted();
			}
		}

		if (_vm->_events->_mouseButtons || (_vm->_easyMouse && scene._action._v83338 > 1
				&& scene._userInterface._category == CAT_INV_LIST) ||
				(_vm->_easyMouse && scene._userInterface._category == CAT_HOTSPOT)) {
			scene._action.checkActionAtMousePos();
		}
			
		if (_vm->_events->_mouseReleased) {
			scene.leftClick();
			scene._userInterface._category = CAT_NONE;
		}

		if (_vm->_events->_mouseButtons || _vm->_easyMouse || scene._userInterface._scrollerY)
			proc1();

		if (_vm->_events->_mouseButtons || _vm->_easyMouse)
			scene._action.set();

		_v8333C = 0;
	}

	scene._action.refresh();

	// Loop through image inter list
	warning("TODO: imageInterList loop");
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

void SceneNode::load(Common::SeekableReadStream *f) {
	_walkPos.x = f->readSint16LE();
	_walkPos.y = f->readSint16LE();
	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		_indexes[i] = f->readUint16LE();
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

void SceneInfo::setRouteNode(int nodeIndex, const Common::Point &pt, MSurface &depthSurface) {
	int flags, hypotenuse;

	_nodes[nodeIndex]._walkPos = pt;

	// Recalculate inter-node lengths
	for (uint idx = 0; idx < _nodes.size(); ++idx) {
		int entry;
		if (idx == (uint)nodeIndex) {
			entry = 0x3FFF;
		}
		else {
			// Process the node
			flags = getRouteFlags(pt, _nodes[idx]._walkPos, depthSurface);

			int xDiff = ABS(_nodes[idx]._walkPos.x - pt.x);
			int yDiff = ABS(_nodes[idx]._walkPos.y - pt.y);
			hypotenuse = sqrt((double)(xDiff * xDiff + yDiff * yDiff));

			if (hypotenuse >= 0x3FFF)
				// Shouldn't ever be this large
				hypotenuse = 0x3FFF;

			entry = hypotenuse | flags;
			_nodes[idx]._indexes[nodeIndex] = entry;
			_nodes[nodeIndex]._indexes[idx] = entry;
		}
	}
}

int SceneInfo::getRouteFlags(const Common::Point &src, const Common::Point &dest, 
		MSurface &depthSurface) {
	int result = 0x8000;
	bool flag = false;

	int xDiff = ABS(dest.x - src.x);
	int yDiff = ABS(dest.y - src.y);
	int xDirection = dest.x >= src.x ? 1 : -1;
	int yDirection = dest.y >= src.y ? depthSurface.w : -depthSurface.w;
	int majorDiff = 0;
	if (dest.x < src.x)
		majorDiff = MAX(xDiff, yDiff);
	++xDiff;
	++yDiff;

	byte *srcP = depthSurface.getBasePtr(src.x, src.y);

	int totalCtr = majorDiff;
	for (int xCtr = 0; xCtr < xDiff; ++xCtr, srcP += xDirection) {
		totalCtr += yDiff;

		if ((*srcP & 0x80) == 0)
			flag = false;
		else if (!flag) {
			flag = true;
			result -= 0x4000;
			if (result == 0)
				break;
		}

		while (totalCtr >= xDiff) {
			totalCtr -= xDiff;

			if ((*srcP & 0x80) == 0)
				flag = false;
			else if (!flag) {
				flag = true;
				result -= 0x4000;
				if (result == 0)
					break;
			}

			srcP += yDirection;
		}
		if (result == 0)
			break;
	}

	return result;
}

/*------------------------------------------------------------------------*/

SceneInfo *SceneInfo::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular) {
		return new Nebular::SceneInfoNebular(vm);
	} else {
		error("Unknown game");
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
	Common::SeekableReadStream *depthStream = infoPack.getItemStream(1);
	loadCodes(depthSurface, depthStream);
	delete depthStream;

	infoFile.close();

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

		_usageIndex = _vm->_palette->_paletteUsage.process(artHeader._palette, 
			(flags & 0xF800) | 0x8000);
		if (_usageIndex > 0) {
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

	if (!(flags & 1)) {
		// Translate the background to use the correct palette indexes
		bgSurface.translate(artHeader._palette);
	}

	// Close the ART file
	delete stream;
	artFile.close();

	Common::Array<SpriteAsset *> spriteSets;
	Common::Array<int> usageList;

	if (flags & 1) {
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

		asset->drawScaled(asset->getCount(), depthSurface, bgSurface,
			si._scale, si._depth, si._position);
	}

	// Free the sprite sets
	for (int i = (int)spriteSets.size() - 1; i >= 0; --i) {
		warning("TODO: sub_201C8 SPRITE_SET.field_6");
		delete spriteSets[i];
	}
}

/*------------------------------------------------------------------------*/

SceneLogic::SceneLogic(MADSEngine *vm) : _vm(vm) {
	_scene = &_vm->_game->_scene;
}

} // End of namespace MADS
