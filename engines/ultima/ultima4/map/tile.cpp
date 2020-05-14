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

#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/tileanim.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

TileId Tile::_nextId = 0;

Tile::Tile(Tileset *tileset)
	: _id(_nextId++)
	, _name()
	, _tileSet(tileset)
	, _w(0)
	, _h(0)
	, _frames(0)
	, _scale(1)
	, _anim(nullptr)
	, _opaque(false)
	, _foreground()
	, _waterForeground()
	, rule(nullptr)
	, _imageName()
	, _looksLike()
	, _image(nullptr)
	, _tiledInDungeon(false)
	, _directions()
	, _animationRule("") {
}

Tile::~Tile() {
	deleteImage();
}

void Tile::loadProperties(const ConfigElement &conf) {
	if (conf.getName() != "tile")
		return;

	_name = conf.getString("name");	// Get the name of the tile

	// Get the animation for the tile, if one is specified
	if (conf.exists("animation")) {
		_animationRule = conf.getString("animation");
	}

	// See if the tile is opaque
	_opaque = conf.getBool("opaque");

	_foreground = conf.getBool("usesReplacementTileAsBackground");
	_waterForeground = conf.getBool("usesWaterReplacementTileAsBackground");

	/* find the rule that applies to the current tile, if there is one.
	   if there is no rule specified, it defaults to the "default" rule */
	if (conf.exists("rule")) {
		rule = g_tileRules->findByName(conf.getString("rule"));
		if (rule == nullptr)
			rule = g_tileRules->findByName("default");
	} else {
		rule = g_tileRules->findByName("default");
	}

	// Get the number of frames the tile has
	_frames = conf.getInt("frames", 1);

	// Get the name of the image that belongs to this tile
	if (conf.exists("image"))
		_imageName = conf.getString("image");
	else
		_imageName = Common::String("tile_") + _name;

	_tiledInDungeon = conf.getBool("tiledInDungeon");

	if (conf.exists("directions")) {
		Common::String dirs = conf.getString("directions");
		if (dirs.size() != (unsigned) _frames)
			error("Error: %ld directions for tile but only %d frames", (long) dirs.size(), _frames);
		for (unsigned i = 0; i < dirs.size(); i++) {
			if (dirs[i] == 'w')
				_directions.push_back(DIR_WEST);
			else if (dirs[i] == 'n')
				_directions.push_back(DIR_NORTH);
			else if (dirs[i] == 'e')
				_directions.push_back(DIR_EAST);
			else if (dirs[i] == 's')
				_directions.push_back(DIR_SOUTH);
			else
				error("Error: unknown direction specified by %c", dirs[i]);
		}
	}
}

Image *Tile::getImage() {
	if (!_image)
		loadImage();
	return _image;
}

void Tile::loadImage() {
	if (!_image) {
		_scale = settings._scale;

		SubImage *subimage = nullptr;

		ImageInfo *info = imageMgr->get(_imageName);
		if (!info) {
			subimage = imageMgr->getSubImage(_imageName);
			if (subimage)
				info = imageMgr->get(subimage->_srcImageName);
		}
		if (!info) { // IF still no info loaded
			warning("Error: couldn't load image for tile '%s'", _name.c_str());
			return;
		}

		/* FIXME: This is a hack to address the fact that there are 4
		   frames for the guard in VGA mode, but only 2 in EGA. Is there
		   a better way to handle this? */
		if (_name == "guard") {
			if (settings._videoType == "EGA")
				_frames = 2;
			else
				_frames = 4;
		}


		if (info->_image)
			info->_image->alphaOff();

		if (info) {
			_w = (subimage ? subimage->width() *_scale : info->_width * _scale / info->_prescale);
			_h = (subimage ? (subimage->height() * _scale) / _frames : (info->_height * _scale / info->_prescale) / _frames);
			_image = Image::create(_w, _h * _frames, false, Image::HARDWARE);


			//info->image->alphaOff();

			// Draw the tile from the image we found to our tile image
			Image *tiles = info->_image;
			assert(tiles);

			if (subimage) {
				tiles->drawSubRectOn(_image, 0, 0,
					subimage->left * _scale, subimage->top * _scale,
					subimage->width() * _scale, subimage->height() * _scale);
			} else {
				tiles->drawOn(_image, 0, 0);
			}
		}

		if (_animationRule.size() > 0) {
			_anim = nullptr;
			if (g_screen->_tileAnims)
				_anim = g_screen->_tileAnims->getByName(_animationRule);
			if (_anim == nullptr)
				warning("Warning: animation style '%s' not found", _animationRule.c_str());
		}

		/* if we have animations, we always used 'animated' to draw from */
		//if (anim)
		//    image->alphaOff();


	}
}

void Tile::deleteImage() {
	if (_image) {
		delete _image;
		_image = nullptr;
	}
	_scale = settings._scale;
}

bool Tile::isDungeonFloor() const {
	Tile *floor = _tileSet->getByName("brick_floor");
	if (_id == floor->_id)
		return true;
	return false;
}

bool Tile::isOpaque() const {
	return g_context->_opacity ? _opaque : false;
}

bool Tile::isForeground() const {
	return (rule->_mask & MASK_FOREGROUND);
}

Direction Tile::directionForFrame(int frame) const {
	if (static_cast<unsigned>(frame) >= _directions.size())
		return DIR_NONE;
	else
		return _directions[frame];
}

int Tile::frameForDirection(Direction d) const {
	for (int i = 0; (unsigned) i < _directions.size() && i < _frames; i++) {
		if (_directions[i] == d)
			return i;
	}
	return -1;
}

} // End of namespace Ultima4
} // End of namespace Ultima
