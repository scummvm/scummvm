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

#include "common/random.h"
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
	// Sprites streamed into a texture's VRAM slot are drawn through its CLUT.
	const Common::Array<uint32> *palette(const char *name) const;
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

// One part of level 2: the backdrop halves and parallax walls scroll behind the rookie,
// who stands centred and ducks in and out of cover while scripted troopers trade fire.
class RA2PSXLevel2Scene {
public:
	RA2PSXLevel2Scene();

	bool load(const RA2PSXArchive &archive, int part, int difficulty,
			Common::RandomSource &random);
	// Toggles between cover and the open; ignored while a move is already running.
	void toggleCover();
	void update(int aimX, int aimY);
	// One 60Hz tick of the trooper slots. Returns the shield damage it cost the player.
	int updateEnemies(Common::RandomSource &random);
	// Fires a bolt at the crosshair and kills whatever it covers; returns the score.
	int shoot(int aimX, int aimY);
	bool outOfCover() const { return _out && !_moving; }
	// Anything but fully ducked; the original only shields a rookie who has finished.
	bool exposed() const { return _out || _moving; }
	bool busy() const { return _moving; }
	bool cleared() const { return _clearTicks >= kRA2PSXLevel2ClearTicks; }
	int kills() const { return _kills; }
	int misses() const { return _misses; }
	void draw(Graphics::Surface &surface, int aimX, int aimY) const;
	const RA2PSXLevel2PartInfo &info() const;

private:
	void drawLayer(Graphics::Surface &surface, const RA2PSXLevel2Layer &layer,
			int left, int top) const;
	void drawFrame(Graphics::Surface &surface, const RA2PSXTexture &frame,
			int x, int y) const;
	void drawPlayFrame(Graphics::Surface &surface, const RA2PSXPlayFrame &frame,
			const Common::Array<uint32> &palette, int left, int top) const;
	void setScrollTarget(const int16 target[2][2]);
	int aimFrame(int aimX, int aimY) const;
	// How many poses the rookie walks leaning out; the aim grid starts right after.
	int coverFrames() const;
	// Restarts an actor on the given animation, the way the original's loader does.
	void startActor(RA2PSXLevel2Actor &actor, int state, int animation);
	const RA2PSXPlayFrame *actorFrame(const RA2PSXLevel2Actor &actor) const;
	void advanceActor(RA2PSXLevel2Actor &actor);
	int slotDelay(int base, int range, Common::RandomSource &random) const;
	// Projects a bolt's 3D line onto the screen at the given point along it.
	void projectShot(const RA2PSXLevel2Shot &shot, int step, int &x, int &y) const;

	RA2PSXTextureSet _textures;
	RA2PSXTextureSet _hud;
	Common::Array<RA2PSXTexture> _rookie;
	Common::Array<RA2PSXPlayAnimation> _play;
	int _part;
	int _difficulty;
	int _frame;
	int _delay;
	bool _out;
	bool _moving;
	// Background and parallax scroll, held in 16.16 with a target to ease toward.
	int _scroll[2][2];
	int _scrollTarget[2][2];
	int _scrollStep[2][2];
	int _scrollHold[2][2];
	// The trooper slots, the bolt each one has in the air, and the wave they draw from.
	RA2PSXLevel2Actor _troopers[kRA2PSXLevel2TrooperCount];
	RA2PSXLevel2Actor _bolts[kRA2PSXLevel2TrooperCount];
	RA2PSXLevel2Shot _shots[kRA2PSXLevel2ShotCount];
	int _tick;
	int _remaining;
	int _active;
	int _clearTicks;
	int _kills;
	int _misses;
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
