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
 */

#ifndef SCUMM_INSANE_REBEL2_PSX_UI_H
#define SCUMM_INSANE_REBEL2_PSX_UI_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/types.h"

#include "graphics/surface.h"

namespace Scumm {

class RA2PSXArchive;

class RA2PSXLevel1UI {
public:
	bool load(const RA2PSXArchive &archive);

	void drawCockpit(Graphics::Surface &surface) const;
	void drawExplosion(Graphics::Surface &surface, int x, int y, int frame) const;
	void drawHUD(Graphics::Surface &surface, int score, int lives, int shield, int frame) const;

private:
	enum BlendMode {
		kBlendOpaque,
		kBlendAdditive
	};

	struct Texture {
		Common::String name;
		uint16 width;
		uint16 height;
		Common::Array<uint32> pixels;
	};

	bool loadTextures(const Common::Array<byte> &data);
	const Texture *findTexture(const char *name) const;
	void drawTexture(Graphics::Surface &surface, const char *name,
			int x, int y, const Common::Rect &source, int brightness = 0x80,
			BlendMode blend = kBlendOpaque) const;
	void drawShield(Graphics::Surface &surface, int shield, int xOffset, int yOffset) const;

	Common::Array<Texture> _textures;
};

} // End of namespace Scumm

#endif
