/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MYST3_ROOM_H
#define MYST3_ROOM_H

#include "engines/myst3/archive.h"
#include "engines/myst3/gfx.h"

#include "common/array.h"
#include "common/rect.h"

#include "graphics/surface.h"

namespace Myst3 {

class Texture;
class Myst3Engine;
class Subtitles;
class Effect;

class Face {
public:
	Graphics::Surface *_bitmap;
	Graphics::Surface *_finalBitmap;
	Texture *_texture;

	Face(Myst3Engine *vm);
	~Face();

	void setTextureFromJPEG(const ResourceDescription *jpegDesc);

	void addTextureDirtyRect(const Common::Rect &rect);
	bool isTextureDirty() { return _textureDirty; }

	void uploadTexture();

private:
	bool _textureDirty;
	Common::Rect _textureDirtyRect;

	Myst3Engine *_vm;
};

class SpotItemFace {
public:
	SpotItemFace(Face *face, uint16 posX, uint16 posY);
	~SpotItemFace();

	void initBlack(uint16 width, uint16 height);
	void loadData(const ResourceDescription *jpegDesc);
	void updateData(const Graphics::Surface *surface);
	void clear();

	void draw();
	void undraw();
	void fadeDraw();

	bool isDrawn() { return _drawn; }
	void setDrawn(bool drawn) { _drawn = drawn; }
	uint16 getFadeValue() { return _fadeValue; }
	void setFadeValue(uint16 value) { _fadeValue = value; }

	Common::Rect getFaceRect() const;

private:
	Face *_face;
	bool _drawn;
	uint16 _fadeValue;
	uint16 _posX;
	uint16 _posY;

	Graphics::Surface *_bitmap;
	Graphics::Surface *_notDrawnBitmap;

	void initNotDrawn(uint16 width, uint16 height);
};

class SpotItem {
public:
	SpotItem(Myst3Engine *vm);
	~SpotItem();

	void setCondition(int16 condition) { _condition = condition; }
	void setFade(bool fade) { _enableFade = fade; }
	void setFadeVar(uint16 var) { _fadeVar = var; }
	void addFace(SpotItemFace *face) { _faces.push_back(face); }

	void updateUndraw();
	void updateDraw();
private:
	Myst3Engine *_vm;

	int16 _condition;
	uint16 _fadeVar;
	bool _enableFade;

	Common::Array<SpotItemFace *> _faces;
};

class SunSpot {
public:
	uint16 pitch;
	uint16 heading;
	float intensity;
	uint32 color;
	uint16 var;
	bool variableIntensity;
	float radius;
};

class Node : public Drawable {
public:
	Node(Myst3Engine *vm, uint16 id);
	~Node() override;

	void initEffects();
	void resetEffects();

	void update();
	void drawOverlay() override;

	void loadSpotItem(uint16 id, int16 condition, bool fade);
	SpotItemFace *loadMenuSpotItem(int16 condition, const Common::Rect &rect);

	void loadSubtitles(uint32 id);
	bool hasSubtitlesToDraw();

protected:
	virtual bool isFaceVisible(uint faceId) = 0;

	Myst3Engine *_vm;
	uint16 _id;
	Face *_faces[6];
	Common::Array<SpotItem *> _spotItems;
	Subtitles *_subtitles;
	Common::Array<Effect *> _effects;
};

} // end of namespace Myst3

#endif
