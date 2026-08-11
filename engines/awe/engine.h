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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef AWE_ENGINE_H
#define AWE_ENGINE_H

#include "audio/mixer.h"
#include "awe/intern.h"
#include "awe/resource.h"
#include "awe/script.h"
#include "awe/sfx_player.h"
#include "awe/sound.h"
#include "awe/video.h"

namespace Awe {

struct Gfx;
struct SystemStub;

struct Engine {
	enum {
		kStateLogo3DO,
		kStateTitle3DO,
		kStateEnd3DO,
		kStateGame
	};

	int _state = 0;
	Gfx *_graphics = nullptr;
	SystemStub *_stub = nullptr;
	Script _script;
	Sound *_sound = nullptr;
	Resource _res;
	SfxPlayer _ply;
	Video _vid;
	int _partNum = 0;

	Engine(Sound *sound, DataType dataType, int partNum);

	void setSystemStub(SystemStub *, Gfx *);

	const char *getGameTitle(Language lang) const {
		return _res.getGameTitle(lang);
	}

	void run();
	void setup(Language lang, int graphicsType);
	void finish();
	void processInput();

	// 3DO
	void doThreeScreens();
	void doEndCredits();
	void playCinepak(const char *name);
	void scrollText(int a, int b, const char *text);
	void titlePage();

	void saveGameState(uint8 slot, const char *desc);
	void loadGameState(uint8 slot);
};

} // namespace Awe

#endif
