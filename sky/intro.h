/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef INTRO_H
#define INTRO_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "sound/mixer.h"

namespace Sky {

class Disk;
class Screen;
class MusicBase;
class Sound;
class Text;

class Intro {
public:
	Intro(Disk *disk, Screen *screen, MusicBase *music, Sound *sound, Text *text, SoundMixer *mixer, OSystem *system);
	~Intro(void);
	bool doIntro(bool floppyIntro);
	bool _quitProg;
private:
	static uint16 _mainIntroSeq[];
	static uint16 _floppyIntroSeq[];
	static uint16 _cdIntroSeq[];

	Disk *_skyDisk;
	Screen *_skyScreen;
	MusicBase *_skyMusic;
	Sound *_skySound;
	Text *_skyText;
	OSystem *_system;
	SoundMixer *_mixer;

	uint8 *_textBuf, *_saveBuf;
	uint8 *_bgBuf;
	uint32 _bgSize;
	PlayingSoundHandle _voice, _bgSfx;

	bool escDelay(uint32 msecs);
	bool nextPart(uint16 *&data);
	bool floppyScrollFlirt(void);
	bool commandFlirt(uint16 *&data);
	void showTextBuf(void);
	void restoreScreen(void);
};

} // End of namespace Sky

#endif // INTRO_H
