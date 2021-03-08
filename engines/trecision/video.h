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

#ifndef TRECISION_VIDEO_H
#define TRECISION_VIDEO_H

#include "video/smk_decoder.h"

#define SMACKNULL 0
#define SMACKOPEN 1
#define SMACKCLOSE 2

#define FULLMOTIONANIM 620
#define MAXSMACK 3

// SMACKER ANIMATION FLAGS
#define SMKANIM_BKG 1
#define SMKANIM_ICON 2
#define SMKANIM_LOOP 4
#define SMKANIM_OLD 8
#define SMKANIM_ON 16
#define SMKANIM_OFF1 32
#define SMKANIM_OFF2 64
#define SMKANIM_OFF3 128
#define SMKANIM_OFF4 256

namespace Trecision {
class TrecisionEngine;

class NightlongSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
};

class AnimManager {
public:
	AnimManager(TrecisionEngine *vm);
	~AnimManager();

private:
	TrecisionEngine *_vm;

	uint16 _smackPal[MAXSMACK][256];
	uint8 _curSmackAction;
	NightlongSmackerDecoder *SmkAnims[MAXSMACK];

	void CallSmackOpen(Common::SeekableReadStream *stream);
	void CallSmackClose();
	void CallSmackNextFrame();

	void PaintSmackBuffer(int px, int py, int dx, int dy);
	void RegenFullMotion();
	void RegenSmackIcon(int StartIcon, int num);

public:
	uint8 *_smackBuffer[MAXSMACK];
	uint8 *SmackTempBuffer[MAXSMACK];

	uint8 _curSmackBuffer;
	uint16 _playingAnims[MAXSMACK];
	uint16 _curAnimFrame[MAXSMACK];
	int _fullMotionStart, _fullMotionEnd;

	uint16 _animMaxX, _animMinX, _animMaxY, _animMinY;

	SAnim AnimTab[MAXANIM];

	void CallSmackGoto(int buf, int num);
	void CallSmackSoundOnOff(int pos, bool on);
	void CallSmackVolumePan(int buf, int track, int vol);

	void PlayFullMotion(int start, int end);
	void RefreshAllAnimations();
	void RegenAnim(int box);
	void RegenSmackAnim(int num);
	void StartFullMotion(const char *name);
	void StartSmackAnim(uint16 num);
	void StopAllSmackAnims();
	void StopFullMotion();
	void StopSmackAnim(uint16 num);
};
} // end of namespace
#endif
