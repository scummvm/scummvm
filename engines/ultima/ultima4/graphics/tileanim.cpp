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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/graphics/image.h"
#include "ultima/ultima4/game/screen.h"
#include "ultima/ultima4/graphics/tileanim.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/graphics/tile.h"

namespace Ultima {
namespace Ultima4 {

using Common::String;
using Std::vector;

TileAnimTransform *TileAnimTransform::create(const ConfigElement &conf) {
	TileAnimTransform *transform;
	static const char *transformTypeEnumStrings[] = { "invert", "pixel", "scroll", "frame", "pixel_color", NULL };

	int type = conf.getEnum("type", transformTypeEnumStrings);

	switch (type) {
	case 0:
		transform = new TileAnimInvertTransform(conf.getInt("x"),
		                                        conf.getInt("y"),
		                                        conf.getInt("width"),
		                                        conf.getInt("height"));
		break;

	case 1: {
		transform = new TileAnimPixelTransform(conf.getInt("x"),
		                                       conf.getInt("y"));

		vector<ConfigElement> children = conf.getChildren();
		for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
			if (i->getName() == "color") {
				RGBA *rgba = loadColorFromConf(*i);
				((TileAnimPixelTransform *)transform)->_colors.push_back(rgba);
			}
		}
	}

	break;

	case 2:
		transform = new TileAnimScrollTransform(conf.getInt("increment"));
		break;

	case 3:
		transform = new TileAnimFrameTransform();
		break;

	case 4: {
		transform = new TileAnimPixelColorTransform(conf.getInt("x"),
		        conf.getInt("y"),
		        conf.getInt("width"),
		        conf.getInt("height"));

		vector<ConfigElement> children = conf.getChildren();
		for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
			if (i->getName() == "color") {
				RGBA *rgba = loadColorFromConf(*i);
				if (i == children.begin())
					((TileAnimPixelColorTransform *)transform)->_start = rgba;
				else ((TileAnimPixelColorTransform *)transform)->_end = rgba;
			}
		}
	}

	default:
		error("Unknown type");
	}

	/**
	 * See if the transform is performed randomely
	 */
	if (conf.exists("random"))
		transform->_random = conf.getInt("random");
	else
		transform->_random = 0;

	return transform;
}

/**
 * Loads a color from a config element
 */
RGBA *TileAnimTransform::loadColorFromConf(const ConfigElement &conf) {
	RGBA *rgba;

	rgba = new RGBA;
	rgba->r = conf.getInt("red");
	rgba->g = conf.getInt("green");
	rgba->b = conf.getInt("blue");
	rgba->a = IM_OPAQUE;

	return rgba;
}

TileAnimInvertTransform::TileAnimInvertTransform(int xp, int yp, int width, int height) {
	this->x = xp;
	this->y = yp;
	this->w = width;
	this->h = height;
}

bool TileAnimInvertTransform::drawsTile() const {
	return false;
}
void TileAnimInvertTransform::draw(Image *dest, Tile *tile, MapTile &mapTile) {
	int scale = tile->getScale();
	tile->getImage()->drawSubRectInvertedOn(dest, x * scale, y * scale, x * scale,
	                                        (tile->getHeight() * mapTile._frame) + (y * scale), w * scale, h * scale);
}

TileAnimPixelTransform::TileAnimPixelTransform(int xp, int yp) {
	this->x = xp;
	this->y = yp;
}

bool TileAnimPixelTransform::drawsTile() const {
	return false;
}
void TileAnimPixelTransform::draw(Image *dest, Tile *tile, MapTile &mapTile) {
	RGBA *color = _colors[xu4_random(_colors.size())];
	int scale = tile->getScale();
	dest->fillRect(x * scale, y * scale, scale, scale, color->r, color->g, color->b, color->a);
}

bool TileAnimScrollTransform::drawsTile() const {
	return true;
}
TileAnimScrollTransform::TileAnimScrollTransform(int i) : _increment(i), _current(0), _lastOffset(0) {}
void TileAnimScrollTransform::draw(Image *dest, Tile *tile, MapTile &mapTile) {
	if (_increment == 0)
		_increment = tile->getScale();

	int offset = screenCurrentCycle * 4 / SCR_CYCLE_PER_SECOND * tile->getScale();
	if (_lastOffset != offset) {
		_lastOffset = offset;
		_current += _increment;
		if (_current >= tile->getHeight())
			_current = 0;
	}

	tile->getImage()->drawSubRectOn(dest, 0, _current, 0, tile->getHeight() * mapTile._frame, tile->getWidth(), tile->getHeight() - _current);
	if (_current != 0)
		tile->getImage()->drawSubRectOn(dest, 0, 0, 0, (tile->getHeight() * mapTile._frame) + tile->getHeight() - _current, tile->getWidth(), _current);

}

/**
 * Advance the frame by one and draw it!
 */
bool TileAnimFrameTransform::drawsTile() const {
	return true;
}
void TileAnimFrameTransform::draw(Image *dest, Tile *tile, MapTile &mapTile) {
	if (++_currentFrame >= tile->getFrames())
		_currentFrame = 0;
	tile->getImage()->drawSubRectOn(dest, 0, 0, 0, _currentFrame * tile->getHeight(), tile->getWidth(), tile->getHeight());


}

TileAnimPixelColorTransform::TileAnimPixelColorTransform(int xp, int yp, int width, int height) {
	this->x = xp;
	this->y = yp;
	this->w = width;
	this->h = height;
}

bool TileAnimPixelColorTransform::drawsTile() const {
	return false;
}
void TileAnimPixelColorTransform::draw(Image *dest, Tile *tile, MapTile &mapTile) {
	RGBA diff = *_end;
	int scale = tile->getScale();
	diff.r -= _start->r;
	diff.g -= _start->g;
	diff.b -= _start->b;

	Image *tileImage = tile->getImage();

	for (int j = y * scale; j < (y * scale) + (h * scale); j++) {
		for (int i = x * scale; i < (x * scale) + (w * scale); i++) {
			RGBA pixelAt;

			tileImage->getPixel(i, j + (mapTile._frame * tile->getHeight()), pixelAt.r, pixelAt.g, pixelAt.b, pixelAt.a);
			if (pixelAt.r >= _start->r && pixelAt.r <= _end->r &&
			        pixelAt.g >= _start->g && pixelAt.g <= _end->g &&
			        pixelAt.b >= _start->b && pixelAt.b <= _end->b) {
				dest->putPixel(i, j, _start->r + xu4_random(diff.r), _start->g + xu4_random(diff.g), _start->b + xu4_random(diff.b), pixelAt.a);
			}
		}
	}
}

/**
 * Creates a new animation context which controls if animation transforms are performed or not
 */
TileAnimContext *TileAnimContext::create(const ConfigElement &conf) {
	TileAnimContext *context;
	static const char *contextTypeEnumStrings[] = { "frame", "dir", NULL };
	static const char *dirEnumStrings[] = { "none", "west", "north", "east", "south", NULL };

	TileAnimContext::Type type = (TileAnimContext::Type)conf.getEnum("type", contextTypeEnumStrings);

	switch (type) {
	case FRAME:
		context = new TileAnimFrameContext(conf.getInt("frame"));
		break;
	case DIR:
		context = new TileAnimPlayerDirContext(Direction(conf.getEnum("dir", dirEnumStrings)));
		break;
	default:
		context = NULL;
		break;
	}

	/**
	 * Add the transforms to the context
	 */
	if (context) {
		vector<ConfigElement> children = conf.getChildren();

		for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
			if (i->getName() == "transform") {
				TileAnimTransform *transform = TileAnimTransform::create(*i);
				context->add(transform);
			}
		}
	}

	return context;
}

/**
 * Adds a tile transform to the context
 */
void TileAnimContext::add(TileAnimTransform *transform) {
	_animTransforms.push_back(transform);
}

/**
 * A context which depends on the tile's current frame for animation
 */
TileAnimFrameContext::TileAnimFrameContext(int f) : _frame(f) {}
bool TileAnimFrameContext::isInContext(Tile *t, MapTile &mapTile, Direction dir) {
	return (mapTile._frame == _frame);
}

/**
 * An animation context which changes the animation based on the player's current facing direction
 */
TileAnimPlayerDirContext::TileAnimPlayerDirContext(Direction d) : _dir(d) {}
bool TileAnimPlayerDirContext::isInContext(Tile *t, MapTile &mapTile, Direction d) {
	return (d == _dir);
}

/**
 * TileAnimSet
 */
TileAnimSet::TileAnimSet(const ConfigElement &conf) {
	_name = conf.getString("name");

	vector<ConfigElement> children = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "tileanim") {
			TileAnim *anim = new TileAnim(*i);
			_tileAnims[anim->_name] = anim;
		}
	}
}

/**
 * Returns the tile animation with the given name from the current set
 */
TileAnim *TileAnimSet::getByName(const Common::String &name) {
	TileAnimMap::iterator i = _tileAnims.find(name);
	if (i == _tileAnims.end())
		return NULL;
	return i->_value;
}

TileAnim::TileAnim(const ConfigElement &conf) : _random(0) {
	_name = conf.getString("name");
	if (conf.exists("random"))
		_random = conf.getInt("random");

	vector<ConfigElement> children = conf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "transform") {
			TileAnimTransform *transform = TileAnimTransform::create(*i);

			_transforms.push_back(transform);
		} else if (i->getName() == "context") {
			TileAnimContext *context = TileAnimContext::create(*i);

			_contexts.push_back(context);
		}
	}
}

void TileAnim::draw(Image *dest, Tile *tile, MapTile &mapTile, Direction dir) {
	Std::vector<TileAnimTransform *>::const_iterator t;
	Std::vector<TileAnimContext *>::const_iterator c;
	bool drawn = false;

	/* nothing to do, draw the tile and return! */
	if ((_random && xu4_random(100) > _random) || (!_transforms.size() && !_contexts.size()) || mapTile._freezeAnimation) {
		tile->getImage()->drawSubRectOn(dest, 0, 0, 0, mapTile._frame * tile->getHeight(), tile->getWidth(), tile->getHeight());
		return;
	}

	/**
	 * Do global transforms
	 */
	for (t = _transforms.begin(); t != _transforms.end(); t++) {
		TileAnimTransform *transform = *t;

		if (!transform->_random || xu4_random(100) < transform->_random) {
			if (!transform->drawsTile() && !drawn)
				tile->getImage()->drawSubRectOn(dest, 0, 0, 0, mapTile._frame * tile->getHeight(), tile->getWidth(), tile->getHeight());
			transform->draw(dest, tile, mapTile);
			drawn = true;
		}
	}

	/**
	 * Do contextual transforms
	 */
	for (c = _contexts.begin(); c != _contexts.end(); c++) {
		if ((*c)->isInContext(tile, mapTile, dir)) {
			TileAnimContext::TileAnimTransformList ctx_transforms = (*c)->getTransforms();
			for (t = ctx_transforms.begin(); t != ctx_transforms.end(); t++) {
				TileAnimTransform *transform = *t;

				if (!transform->_random || xu4_random(100) < transform->_random) {
					if (!transform->drawsTile() && !drawn)
						tile->getImage()->drawSubRectOn(dest, 0, 0, 0, mapTile._frame * tile->getHeight(), tile->getWidth(), tile->getHeight());
					transform->draw(dest, tile, mapTile);
					drawn = true;
				}
			}
		}
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
