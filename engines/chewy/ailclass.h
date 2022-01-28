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

class ailScummVM {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandles[2];
public:
	ailScummVM();

	/**
	 * Plays a VOC file from the passed stream, and takes
	 * responsibility for freeing it when done
	 */
	void playSpeech(int channel, Common::SeekableReadStream *src);

	/**
	 * Returns true if one of the two speech channels are
	 * currently playing
	 */
	bool isSpeechActive(int channel = 0) const;

	/**
	 * Helper method to wait until any playing speech is finished
	 */
	void waitForSpeechToFinish();

	/**
	 * Returns true if subtitles are enabled
	 */
	bool hasSubtitles();

	/**
	 * Returns true if speech is muted
	 */
	bool isSpeechMuted();
};

class ailclass : public ailScummVM {
public:
	ailclass();
	~ailclass();

	int16 init(uint16 freq);
	int16 init(char *midiDrvName);
	void exit1();

	void setMusicMasterVol(int16 vol);
	void setSoundMasterVol(int16 vol);
	void setStereoPos(int16 channel, int16 pos);
	void setChannelVol(uint8 channel, uint8 vol);
	void disableSound();

	void getMusicInfo(musik_info *mi);
	void getChannelInfo(channel_info *mi, int16 channel);
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
	void fadeIn(uint16 delay);
	void fadeOut(uint16 delay);

	void playVoc(byte *anf_adr, int16 channel, int16 vol, int16 rep);
	void playRaw(int16 channel, char *voc, uint32 len, uint16 frequency, int16 volume, int16 rep);
	void endSound();
	void stopSound();
	void continueSound();
	void stopSample(int16 channel);
	void continueSample(int16 channel);
	void endSample(int16 channel);
	void initDoubleBuffer(byte *b1, byte *b2, uint32 len, int16 channel);

	void startDbVoc(Stream *voc, int16 channel, int16 vol);
	void serveDbSamples();
	void switchMusic(bool onOff);
	void switchSound(bool onOff);
};

extern void serve_speech();

} // namespace Chewy

#endif
