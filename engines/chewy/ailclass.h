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

#ifndef CHEWY_AILCLASS_H
#define CHEWY_AILCLASS_H

#include "audio/mixer.h"
#include "chewy/ngstypes.h"

namespace Chewy {

void mod_irq();
void checkSampleEnd();
void DecodePatternLine();
void DecodeChannel(int16 ch);

class ailclass {
public:
	ailclass();
	~ailclass();

	void setMusicMasterVol(int16 vol);
	void setSoundMasterVol(int16 vol);
	void disableSound();

	void getMusicInfo(musik_info *mi);
	int16 musicPlaying();
	int16 getSampleStatus(int16 channel);

	void initNoteTable(uint16 sfreq);
	void initMixMode();
	void exitMixMode();
	void playMod(tmf_header *th);
	void stopMod();
	void continueMusic();
	void playSequence(int16 startPos, int16 endPos);
	void playPattern(int16 pattern);
	void setLoopMode(int16 mode);
	void fadeOut(uint16 delay);

	void endSound();
};

} // namespace Chewy

#endif
