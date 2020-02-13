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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef LILLIPUT_SOUND_H
#define LILLIPUT_SOUND_H

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"

namespace Lilliput {

class LilliputEngine;

class LilliputSound: public Audio::MidiPlayer {
public:
	LilliputSound();
	~LilliputSound() override;

	void init();
	void refresh();
	void playSound(int var1, Common::Point var2, Common::Point var3, Common::Point var4);
	void stopSound(Common::Point pos);
	void toggleOnOff();
	void update();
	void remove();

private:
	int _fileNumb;
	byte **_unpackedFiles;
	uint16 *_unpackedSizes;
	bool _isGM;

	uint32 decode(const byte *src, byte *dst, uint32 len, uint32 start);
	byte readByte(const byte *data, uint32 offset);

	void loadMusic(Common::String filename);
	void playMusic(int var1);

	void send(uint32 b) override;
	void sendToChannel(byte channel, uint32 b) override;
};

} // End of namespace Lilliput

#endif

