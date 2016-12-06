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

#ifndef CRYO_VIDEO_H
#define CRYO_VIDEO_H

namespace Cryo {

class CryoEngine;

class HnmPlayer {
private:
	CryoEngine *_vm;

	void resetInternalTimer();
	void wantsSound(bool sound);
	void decompADPCM(byte *buffer, int16 *output, int size);
	void loadDecompTable(int16 *buffer);
	bool loadFrame(hnm_t *hnm);
	void tryRead(hnm_t *hnm, int size);
	void changePalette(hnm_t *hnm);
	void selectBuffers(hnm_t *hnm);
	void decompLempelZiv(byte *buffer, byte *output);
	void desentrelace320(byte *frame_buffer, byte *final_buffer, uint16 height);
	void desentrelace(hnm_t *hnm);
	void decompUBA(byte *output, byte *curr_buffer, byte *prev_buffer, byte *input, int width, char flags);

	// Unused
	void done();
	void dispose(hnm_t *hnm);
	void canLoop(hnm_t *hnm, bool canLoop);
	void soundInADPCM(bool is_adpcm);
	void soundMono(bool is_mono);
	//

	bool _soundStarted;
	int16 _pendingSounds;
	float _timeDrift;
	float _nextFrameTime;
	float _expectedFrameTime;
	float _rate;
	bool _useSoundSync;
	bool _useSound;
	int16 _prevRight;
	int16 _prevLeft;
	bool _useAdpcm;
	bool _useMono;
	bool _preserveColor0;
	int16 decompTable[256];
	bool _safePalette;

	void (*_customChunkHandler)(byte *buffer, int size, int16 id, char h6, char h7);

	soundchannel_t *_soundChannel;
	SoundGroup *_soundGroup;
	soundchannel_t *_soundChannelAdpcm;
	SoundGroup *_soundGroupAdpcm;

public:
	uint16 _curVideoNum;

	HnmPlayer(CryoEngine *vm);

	void setupTimer(float rate);
	void reset(hnm_t *hnm);
	void closeSound();
	void waitLoop(hnm_t *hnm);
	void flushPreloadBuffer(hnm_t *hnm);
	void setupSound(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode);
	void setupSoundADPCM(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode);
	bool nextElement(hnm_t *hnm);
	void init();
	void setForceZero2Black(bool forceblack);
	void readHeader(hnm_t *hnm);
	int16 getVersion(hnm_t *hnm);
	void allocMemory(hnm_t *hnm);
	void deallocMemory(hnm_t *hnm);
	void setFinalBuffer(hnm_t *hnm, byte *buffer);
	int getFrameNum(hnm_t *hnm);
	hnm_t *resetInternals();
	void setFile(hnm_t *hnm, Common::File *file);

	soundchannel_t *getSoundChannel();
};

} // End of namespace Cryo

#endif
