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
		rgb._colorCount = f->readByte();
		rgb.g = f->readByte();
		rgb._firstColorIndex = f->readByte();
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
		return new Nebular::SceneInfoNebular(vm);
	} else {
		error("Unknown game");
	}
}

void SceneInfo::load(int sceneId, int variant, const Common::String &resName,
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
		WalkNode node;
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
	Common::SeekableReadStream *depthStream = infoPack.getItemStream(variant + 1);
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
				_palAnimData[i]._firstColorIndex = artHeader._palette[g]._palIndex;
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
