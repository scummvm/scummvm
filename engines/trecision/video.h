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

#include "trecision/nl/struct.h"

#include "common/file.h"
#include "video/smk_decoder.h"
#include "common/serializer.h"

namespace Trecision {

#define MAXANIM  750
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

enum SmackerType {
	kSmackerBackground = 0,		// Scene background animations
	kSmackerAction = 1,			// Main character action animations
	kSmackerIcon = 2			// Smacker inventory animations
};

class TrecisionEngine;

class NightlongSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
	bool forceSeekToFrame(uint frame);
	void forceSkip(uint frame);
	bool endOfVideo() const;
};

class AnimManager {
public:
	AnimManager(TrecisionEngine *vm);
	~AnimManager();

private:
	TrecisionEngine *_vm;

	NightlongSmackerDecoder *_smkAnims[MAXSMACK];

	FastFile _animFile[MAXSMACK]; // nlanim.cd1 / nlanim.cd2 / nlanim.cd3
	int _curCD;

	void openSmk(int slot, Common::SeekableReadStream *stream);
	void openSmkAnim(int slot, Common::String name);
	void closeSmk(int slot);
	void drawFrame(NightlongSmackerDecoder *smkDecoder, uint16 x, uint16 y, bool updateScreen);
	void drawFrameSubtitles(Graphics::Surface *surface, int frameNum);
	void setVideoRange(NightlongSmackerDecoder *smkDecoder, int &startFrame, int &endFrame);
	void handleEndOfVideo(int animation, int slot);

	void drawSmkBackgroundFrame(int animation);
	void drawSmkIconFrame(uint16 startIcon, uint16 iconNum);
	void drawSmkActionFrame();
	void swapCD(int cd);

public:
	uint16 _playingAnims[MAXSMACK];
	Common::Rect _animRect;
	SAnim _animTab[MAXANIM];

	void smkGoto(int slot, int frame);
	void smkSoundOnOff(int slot, bool on);
	void smkVolumePan(int slot, int track, int vol);
	int16 smkCurFrame(int slot);
	void smkStop(uint16 slot);

	void playMovie(Common::String filename, int startFrame = 0, int endFrame = -1);
	void startFullMotion(const char *name);
	void stopFullMotion();

	void refreshAllAnimations();
	void refreshAnim(int box);
	void refreshSmkAnim(uint16 animation);
	void startSmkAnim(uint16 animation);
	void stopAllSmkAnims();

	void syncGameStream(Common::Serializer &ser);
	void loadAnimTab(Common::File *file);
};
} // end of namespace
#endif
