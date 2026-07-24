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

#include "common/rect.h"

#include "graphics/surface.h"

#include "scumm/insane/rebel2/psx/psx.h"

namespace Scumm {

class RA2PSXTextureSet {
public:
	enum BlendMode {
		kBlendOpaque,
		kBlendAdditive
	};

	void clear() { _textures.clear(); }
	bool append(const Common::Array<byte> &data);
	bool appendRaw24(const char *name, const Common::Array<byte> &data,
			uint16 width, uint16 height);
	bool has(const char *name) const { return find(name) != nullptr; }
	void draw(Graphics::Surface &surface, const char *name, int x, int y,
			const Common::Rect &source, int brightness = 0x80,
			BlendMode blend = kBlendOpaque) const;
	void drawText(Graphics::Surface &surface, const char *font, const char *text,
			int x, int y) const;

private:
	const RA2PSXTexture *find(const char *name) const;
	Common::Array<RA2PSXTexture> _textures;
};

class RA2PSXMovieText {
public:
	bool load(Common::SeekableReadStream &executable);
	void draw(Graphics::Surface &surface, RA2PSXMovieTextSequence sequence,
			int frame, int xOffset, int yOffset) const;

private:
	RA2PSXTextureSet _textures;
};

class RA2PSXMainMenuUI {
public:
	bool load(const RA2PSXArchive &archive);
	void draw(Graphics::Surface &surface, int selection) const;
	Common::Rect itemRect(int item) const;

private:
	RA2PSXTextureSet _textures;
};

class RA2PSXLevel1UI {
public:
	bool load(const RA2PSXArchive &archive);

	void drawCockpit(Graphics::Surface &surface) const;
	void drawExplosion(Graphics::Surface &surface, int x, int y, int frame) const;
	void drawHUD(Graphics::Surface &surface, int score, int lives, int shield, int frame) const;

private:
	void drawShield(Graphics::Surface &surface, int shield, int xOffset, int yOffset) const;

	RA2PSXTextureSet _textures;
};

} // End of namespace Scumm

#endif
