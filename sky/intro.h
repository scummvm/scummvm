/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "sky/screen.h"
#include "sky/disk.h"

class SkyIntro {
public:
	SkyIntro(SkyDisk *disk, SkyScreen *screen, SkyMusicBase *music, SkySound *sound, SkyText *text, SoundMixer *mixer, OSystem *system);
	~SkyIntro(void);
	bool doIntro(bool floppyIntro);
	bool _quitProg;
private:
	static uint16 _mainIntroSeq[];
	static uint16 _floppyIntroSeq[];
	static uint16 _cdIntroSeq[];

	SkyDisk *_skyDisk;
	SkyScreen *_skyScreen;
	SkyMusicBase *_skyMusic;
	SkySound *_skySound;
	SkyText *_skyText;
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

#endif // INTRO_H

