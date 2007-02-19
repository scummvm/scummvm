/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef AGOS_ANIMATION_H
#define AGOS_ANIMATION_H

#include "common/file.h"
#include "common/stream.h"

#include "graphics/dxa_player.h"
#include "sound/mixer.h"

namespace AGOS {

class AGOSEngine;

class MoviePlayer : public Graphics::DXAPlayer {
	AGOSEngine *_vm;

	Audio::Mixer *_mixer;

	Audio::SoundHandle _bgSound;
	Audio::AudioStream *_bgSoundStream;

	bool _omniTV;
	bool _leftButtonDown;
	bool _rightButtonDown;
	uint32 _ticks;
	
	char baseName[40];
	static const char *_sequenceList[90];
	uint8 _sequenceNum;
public:
	MoviePlayer(AGOSEngine *vm, Audio::Mixer *mixer);

	bool load(const char *filename);
	void play();
	void nextFrame();
protected:
	virtual void setPalette(byte *pal);
private:
	void playOmniTV();

	void handleNextFrame();
	void processFrame();
	void startSound();
};

} // End of namespace AGOS

#endif
