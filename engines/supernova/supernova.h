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

#ifndef SUPERNOVA_H
#define SUPERNOVA_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/array.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "engines/engine.h"

#include "supernova/console.h"
#include "supernova/graphics.h"
#include "supernova/msn_def.h"

namespace Supernova {

const int kScreenWidth  = 320;
const int kScreenHeight = 200;
const int kTextSpeed[] = {19, 14, 10, 7, 4};

class SupernovaEngine : public Engine {
public:
	SupernovaEngine(OSystem *syst);
	~SupernovaEngine();

	virtual Common::Error run();

private:
	Common::RandomSource *_rnd;
	Console *_console;
	Audio::SoundHandle _soundHandle;
	bool _gameRunning;
	MSNImageDecoder _image;
	byte _colorIndex;
	
	void initData();
	void initPalette();
	void updateEvents();
	void playSound(int filenumber, int offset = 0);
	void playSoundMod(int filenumber);
	void stopSound();
	void renderImage(int filenumber, int section);
	void renderMessage(char *text, MessagePosition position);
	void renderText(const char *text, int x, int y, byte color);
	void renderBox(int x, int y, int width, int height, byte color);
};

}

#endif
