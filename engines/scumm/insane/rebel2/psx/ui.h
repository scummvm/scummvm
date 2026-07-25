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
		kBlendAdditive,
		// The GPU's semi-transparency mode 0: half background, half source.
		kBlendAverage
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
			int x, int y, int brightness = 0x80) const;
	int measureText(const char *text) const;
	// The 16 pixel tall menu headline font, drawn over a black drop shadow.
	void drawHeadline(Graphics::Surface &surface, const char *text, int x, int y) const;
	int measureHeadline(const char *text) const;

private:
	const RA2PSXTexture *find(const char *name) const;
	void drawShape(Graphics::Surface &surface, const char *name, int x, int y,
			const Common::Rect &source, byte r, byte g, byte b) const;
	Common::Array<RA2PSXTexture> _textures;
};

// The GPU's semi-transparency mode 2: background minus source.
void subtractRA2PSXRect(Graphics::Surface &surface, const Common::Rect &rect,
		int r, int g, int b);
void drawRA2PSXGouraudLine(Graphics::Surface &surface, int x0, int y0, int x1, int y1,
		const byte *from, const byte *to);
// The "back" and "select" button captions along the bottom of every menu page.
void drawRA2PSXMenuHints(Graphics::Surface &surface, const RA2PSXTextureSet &textures);

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
	// The spinning crest is drawn between these two passes.
	void drawBackground(Graphics::Surface &surface) const;
	void drawForeground(Graphics::Surface &surface, int selection) const;
	static Common::Rect itemRect(int item);
	const RA2PSXTextureSet &textures() const { return _textures; }

private:
	RA2PSXTextureSet _textures;
};

// Options menu; passcode, memory card, high scores and controls are dimmed for now.
class RA2PSXOptionsUI {
public:
	enum Item {
		kItemDifficulty,
		kItemAdjustSound,
		kItemPasscode,
		kItemMemoryCard,
		kItemHighScores,
		kItemControls,
		kItemResetSettings,
		kItemExit,
		kItemCount
	};

	enum SoundItem {
		kSoundItemMode,
		kSoundItemEffects,
		kSoundItemMusic,
		kSoundItemMovies,
		kSoundItemExit,
		kSoundItemCount
	};

	explicit RA2PSXOptionsUI(const RA2PSXTextureSet &textures) : _textures(textures) {}

	static bool isItemAvailable(int item);
	static Common::Rect mainItemRect(int item);
	static Common::Rect soundItemRect(int item);

	void drawMain(Graphics::Surface &surface, int selection,
			const RA2PSXSettings &settings) const;
	void drawSound(Graphics::Surface &surface, int selection,
			const RA2PSXSettings &settings) const;
	// A two or three line box; pass a negative selection for a plain message.
	void drawDialog(Graphics::Surface &surface, const char *headline,
			const char *question, const char *detail, int selection) const;

private:
	void drawItem(Graphics::Surface &surface, const char *text, int x, int y,
			bool centered, int state) const;
	void drawVolume(Graphics::Surface &surface, int y, int level) const;

	const RA2PSXTextureSet &_textures;
};

// The scrolling chapter grid, whose tiles are frames of LEVELSEL.STR diced into
// a four by four atlas of 80x60 previews.
class RA2PSXChapterSelectUI {
public:
	enum {
		kChapterCount = 16,
		kTileWidth = 80,
		kTileHeight = 60,
		kRowPitch = 62
	};

	explicit RA2PSXChapterSelectUI(const RA2PSXTextureSet &textures) : _textures(textures) {}

	void draw(Graphics::Surface &surface, const Graphics::Surface *previews,
			int scroll, int selection, int unlocked) const;
	static int rowY(int chapter, int scroll);
	static Common::Rect tileRect(int chapter, int scroll);

private:
	void drawTile(Graphics::Surface &surface, int chapter, int y,
			const Graphics::Surface *previews, bool selected, bool unlocked) const;
	void drawLabel(Graphics::Surface &surface, int chapter, int y,
			bool selected, bool unlocked) const;

	const RA2PSXTextureSet &_textures;
};

// A hit washes the screen for five frames.
enum { kRA2PSXHitFlashFrames = 5 };

void drawRA2PSXHitFlash(Graphics::Surface &surface, int frame);

// The shield gauge runs 0 to 0x1000, and warns below 0x501.
enum {
	kRA2PSXShieldFull = 0x1000,
	kRA2PSXLowShield = 0x501
};

class RA2PSXLevel1UI {
public:
	bool load(const RA2PSXArchive &archive);

	// The shell zooms away while the camera swaps views (0x1000 is its resting size)
	// and drifts a few pixels with the crosshair.
	void drawCockpit(Graphics::Surface &surface, int scale = 0x1000,
			int driftX = 0, int driftY = 0) const;
	void drawExplosion(Graphics::Surface &surface, int x, int y, int frame) const;
	void drawHUD(Graphics::Surface &surface, int score, int lives, int shield, int frame) const;

private:
	void drawShield(Graphics::Surface &surface, int shield, int xOffset, int yOffset) const;

	RA2PSXTextureSet _textures;
};

} // End of namespace Scumm

#endif
