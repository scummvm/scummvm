
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

#ifndef BAGEL_BAGLIB_BAG_CHARACTER_OBJECT_H
#define BAGEL_BAGLIB_BAG_CHARACTER_OBJECT_H

#include "video/smk_decoder.h"
#include "bagel/spacebar/baglib/object.h"

namespace Bagel {
namespace SpaceBar {

class CBagCharacterObject : public CBagObject {
protected:
	Video::SmackerDecoder *_smacker = nullptr;
	CBofBitmap *_bmpBuf = nullptr;
	int _charTransColor = 0;

	char *_binBuf = nullptr;
	int32 _binBufLen = 0;

	int _playbackSpeed = 0;
	int _numOfLoops = 0;
	int _startFrame = 0;
	int _endFrame = 0;

	bool _exitAtEnd : 1;
	bool _firstFrame : 1;

	bool _saveState : 1; // Flag to save the state/frame of the character
	bool _pAnim : 1;     // If affected by Panimations On/Off setting

	int _prevFrame = 0;

	void setFrame(int n);

	// Keep track of the PDA wand and the number of frames it has
	static CBagCharacterObject *_pdaWand;
	static bool _pdaAnimating;

public:
	CBagCharacterObject();
	virtual ~CBagCharacterObject();
	static void initialize();

	// Return ERR_NONE if the Object had members that are properly initialized/de-initialized
	ErrorCode attach() override;
	ErrorCode detach() override;

	CBofRect getRect() override;

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;

	bool doAdvance();
	void updatePosition();
	bool refreshCurrentFrame();

	bool runObject() override;
	bool isInside(const CBofPoint &point) override;

	void arrangeFrames();

	int getNumberOfLoops() const {
		return _numOfLoops;
	}
	int getPlaybackSpeed() const {
		return _playbackSpeed;
	}
	int getStartFrame() const {
		return _startFrame;
	}
	int getEndFrame() const {
		return _endFrame;
	}
	int getCurrentFrame() const {
		return (_smacker != nullptr) ? _smacker->getCurFrame() : -1;
	}

	bool isModalDone() override {
		return !_numOfLoops;
	}

	bool isPanim() const {
		return _pAnim;
	}

	void setNumOfLoops(int n);
	void setPlaybackSpeed(int n);
	void setStartFrame(int n);
	void setEndFrame(int n);
	void setCurrentFrame(int n);

	ParseCodes setInfo(CBagIfstream &istr) override;

	void setProperty(const CBofString &prop, int val) override;
	int getProperty(const CBofString &prop) override;

	// Remember the pda wand, we'll need to know about it and
	// it's total number of frames.
	static void setPdaWand(CBagCharacterObject *pdaWand);
	static bool pdaWandAnimating();

	bool isStationary() const {
		return _binBuf != nullptr;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
