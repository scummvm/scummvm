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

#include "nl/struct.h"
#include "nl/sysdef.h"
#include "video/smk_decoder.h"

namespace Trecision {

enum SmackerType {
	kSmackerBackground = 0,		// Smacker animations embedded in nlanim.cd? files
	kSmackerFullMotion = 1,		// Standalone full screen Smacker videos
	kSmackerAction = 1,			// Main character action animations
	kSmackerIcon = 2			// Smacker inventory animations embedded in nlanim.cd? files
};

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

class TrecisionEngine;

class NightlongSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
	bool forceSeekToFrame(uint frame);
};

class AnimManager {
public:
	AnimManager(TrecisionEngine *vm);
	~AnimManager();

private:
	TrecisionEngine *_vm;

	uint16 _smkPal[MAXSMACK][256];
	NightlongSmackerDecoder *_smkAnims[MAXSMACK];

	FastFile _animFile[MAXSMACK]; // nlanim.cd1 / nlanim.cd2
	int _curCD;

	void openSmk(Common::SeekableReadStream *stream);
	void openSmkAnim(Common::String name);
	void openSmkVideo(Common::String name);
	void closeSmk();
	void smkNextFrame();
	void drawFrame(NightlongSmackerDecoder *smkDecoder, uint16 x, uint16 y, bool updateScreen);
	void drawFrameSubtitles(Graphics::Surface *surface, int frameNum);

	void drawSmkBuffer(int px, int py, int dx, int dy);
	void refreshFullMotion();

	void refreshSmkIcon(int StartIcon, int num);
	void swapCD(int cd);

public:
	uint8 _curSmackBuffer;
	uint8 *_smkBuffer[MAXSMACK];
	uint16 _playingAnims[MAXSMACK];
	uint16 _curAnimFrame[MAXSMACK];

	int _fullMotionStart, _fullMotionEnd;
	uint16 _animMaxX, _animMinX, _animMaxY, _animMinY;

	SAnim _animTab[MAXANIM];

	void smkGoto(int buf, int num);
	void smkSoundOnOff(int pos, bool on);
	void smkVolumePan(int buf, int track, int vol);

	void playMovie(Common::String filename, int startFrame = 1, int endFrame = -1);
	void playFullMotion(int start, int end);
	void startFullMotion(const char *name);
	void stopFullMotion();

	void refreshAllAnimations();
	void refreshAnim(int box);
	void refreshSmkAnim(int num);
	void refreshPalette(int num);
	void startSmkAnim(uint16 num);
	void stopAllSmkAnims();
	void stopSmkAnim(uint16 num);
};
} // end of namespace
#endif
