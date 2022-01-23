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

#ifndef TRECISION_ANIMMANAGER_H
#define TRECISION_ANIMMANAGER_H

#include "trecision/fastfile.h"
#include "trecision/struct.h"

namespace Trecision {

#define MAXANIM 750
#define MAXACTIVEANIM 3

// SMACKER ANIMATION FLAGS
#define SMKANIM_BKG 1
#define SMKANIM_ICON 2
#define SMKANIM_LOOP 4
#define SMKANIM_OLD 8
#define SMKANIM_ON 16

enum SmackerType {
	kSmackerBackground = 0, // Scene background animations
	kSmackerAction = 1,     // Main character action animations
	kSmackerIcon = 2        // Smacker inventory animations
};

class TrecisionEngine;
class NightlongVideoDecoder;

class AnimManager {
public:
	AnimManager(TrecisionEngine *vm);
	~AnimManager();

private:
	TrecisionEngine *_vm;

	NightlongVideoDecoder *_animations[MAXACTIVEANIM];
	uint16 _playingAnims[MAXACTIVEANIM];

	FastFile _animFile[MAXACTIVEANIM]; // nlanim.cd1 / nlanim.cd2 / nlanim.cd3
	int _curCD;
	bool _bgAnimRestarted;

	void openSmk(int slot, Common::SeekableReadStream *stream);
	void openSmkAnim(int slot, const Common::String &name);
	void toggleMuteBgAnim(uint16 animation);
	void closeSmk(int slot);
	void drawFrame(NightlongVideoDecoder *smkDecoder, uint16 x, uint16 y, bool updateScreen);
	void drawFrameSubtitles(Graphics::Surface *surface, int frameNum);
	void setVideoRange(NightlongVideoDecoder *smkDecoder, int &startFrame, int &endFrame);
	void refreshSmkAnim(uint16 animation);
	void handleEndOfVideo(int animation, int slot);
	bool shouldShowAnim(int animation, Common::Rect curRect);

	void drawSmkBackgroundFrame(int animation);
	void drawSmkIconFrame(uint16 startIcon, uint16 iconNum);
	void drawSmkActionFrame();
	void swapCD(int cd);
	void patchAnimTab();

public:
	Common::Rect _animRect;
	SAnim _animTab[MAXANIM];

	void smkGoto(int slot, int frame);
	void smkToggleAudio(int slot, bool on);
	void smkToggleTrackAudio(int slot, int track, bool on);
	int16 smkCurFrame(int slot);
	void smkStop(uint16 slot);
	void refreshActionAnimation() { refreshSmkAnim(_playingAnims[kSmackerAction]); }
	bool isActionActive() const { return _playingAnims[kSmackerAction] != 0; }
	void playMovie(const Common::String &filename, int startFrame = 0, int endFrame = -1, bool singleChoice = false);
	void startFullMotion();
	void stopFullMotion();

	void refreshAnim(int box);
	void startSmkAnim(uint16 animation);
	void stopAllSmkAnims();

	void syncGameStream(Common::Serializer &ser);
	void loadAnimTab(Common::SeekableReadStreamEndian *stream);
};

} // End of namespace Trecision
#endif

