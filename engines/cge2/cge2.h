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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_H
#define CGE2_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "cge2/fileio.h"
#include "cge2/general.h"

namespace CGE2 {

class Vga;
class Sprite;
class MusicPlayer;
class Fx;
class Sound;
class Text;
struct HeroTab;

#define kScrWidth      320
#define kScrHeight     240
#define kMaxFile       128
#define kPathMax       128
#define kDimMax          8
#define kWayMax         10
#define kPocketMax       4
#define kCaveMax       100

class CGE2Engine : public Engine {
public:
	CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription);
	virtual bool hasFeature(EngineFeature f) const;
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual Common::Error run();

	bool showTitle(const char *name);
	void cge2_main();
	char *mergeExt(char *buf, const char *name, const char *ext);
	void inf(const char *text, bool wideSpace = false);
	void movie(const char *ext);
	void runGame();
	void loadScript(const char *fname);
	void loadSprite(const char *fname, int ref, int scene, V3D &pos);

	const ADGameDescription *_gameDescription;

	bool _quitFlag;
	Dac *_bitmapPalette;
	int _mode;
	bool _music;
	int _startupMode;
	int _now;
	bool _sex;

	ResourceManager *_resman;
	Vga *_vga;
	Sprite *_sprite;
	MusicPlayer *_midiPlayer;
	Fx *_fx;
	Sound *_sound;
	Text *_text;
	HeroTab *_heroTab[2];
private:
	void init();
	void deinit();
};

} // End of namespace CGE2

#endif // CGE2_H
