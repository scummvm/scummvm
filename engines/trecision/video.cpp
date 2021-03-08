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

#include "trecision/trecision.h"
#include "trecision/video.h"

#include <common/config-manager.h>

#include "trecision/graphics.h"

#include "common/scummsys.h"
#include "nl/define.h"
#include "nl/extern.h"

namespace Trecision {

bool NightlongSmackerDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (Video::SmackerDecoder::loadStream(stream)) {
		// Map audio tracks to sound types
		for (uint32 i = 0; i < 8; i++) {
			Track *t = getTrack(i);
			if (t && t->getTrackType() == Track::kTrackTypeAudio) {
				AudioTrack *audio = (AudioTrack *)t;
				audio->setMute(false);
				audio->setSoundType(i == 7 ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType);
			}
		}
		return true;
	}
	return false;
}

void NightlongSmackerDecoder::muteTrack(uint track, bool mute) {
	Track *t = getTrack(track);
	if (t && t->getTrackType() == Track::kTrackTypeAudio) {
		((AudioTrack *)t)->setMute(mute);
	}
}

void NightlongSmackerDecoder::setMute(bool mute) {
	for (TrackList::iterator it = getTrackListBegin(); it != getTrackListEnd(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setMute(mute);
	}
}

AnimManager::AnimManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < MAXSMACK; ++i) {
		_smackBuffer[i] = nullptr;
		SmackTempBuffer[i] = nullptr;
		SmkAnims[i] = nullptr;
		_playingAnims[i] = 0;
		_curAnimFrame[i] = 0;

		for (int j = 0; j < 256; ++j)
			_smackPal[i][j] = 0;
	}

	for (int i = 0; i < MAXANIM; ++i) {
		AnimTab[i]._flag = 0;
		AnimTab[i]._name[0] = '\0';
	}
	

	_animMaxX = _animMinX = 0;
	_animMaxY = _animMinY = 0;

	_curSmackAction = 0;
	_curSmackBuffer = 0;

	_fullMotionStart = _fullMotionEnd = 0;
}

AnimManager::~AnimManager() {}

/*------------------------------------------------
					CallSmackOpen
--------------------------------------------------*/
void AnimManager::CallSmackOpen(Common::SeekableReadStream *stream) {
	if (stream == nullptr)
		return;

	SmkAnims[_curSmackBuffer] = new NightlongSmackerDecoder();

	if (!SmkAnims[_curSmackBuffer]->loadStream(stream)) {
		warning("Invalid SMK file");
		CallSmackClose();
	} else {
		SmkAnims[_curSmackBuffer]->start();
		CallSmackNextFrame();
	}
}

/*------------------------------------------------
					CallSmackClose
--------------------------------------------------*/
void AnimManager::CallSmackClose() {
	delete SmkAnims[_curSmackBuffer];
	SmkAnims[_curSmackBuffer] = nullptr;
}

/*------------------------------------------------
				CallSmackNextFrame
--------------------------------------------------*/
void AnimManager::CallSmackNextFrame() {
	if (SmkAnims[_curSmackBuffer] == nullptr)
		return;

	if (SmkAnims[_curSmackBuffer]->getCurFrame() == (int)SmkAnims[_curSmackBuffer]->getFrameCount() - 1) {
		SmkAnims[_curSmackBuffer]->rewind();
	}

	if (SmkAnims[_curSmackBuffer]->needsUpdate()) {
		const Graphics::Surface *surface = SmkAnims[_curSmackBuffer]->decodeNextFrame();
		if (surface != nullptr)
			_smackBuffer[_curSmackBuffer] = (uint8 *)surface->getPixels();
	}
}

/*---------------------------------------------------
 * 					CallSmackGoto
 *---------------------------------------------------*/
void AnimManager::CallSmackGoto(int buf, int num) {
	_curSmackBuffer = buf;

	if (SmkAnims[_curSmackBuffer] == NULL)
		return;

	SmkAnims[_curSmackBuffer]->seekToFrame(num);
}

/*------------------------------------------------
				CallSmackVolumePan
--------------------------------------------------*/
void AnimManager::CallSmackVolumePan(int buf, int track, int vol) {
	_curSmackBuffer = buf;

	if (SmkAnims[_curSmackBuffer] == nullptr)
		return;

	SmkAnims[_curSmackBuffer]->muteTrack(track, vol == 0);
}

/*--------------------------------------------------
 * 				CallSmackSoundOnOff
 *--------------------------------------------------*/
void AnimManager::CallSmackSoundOnOff(int pos, bool on) {
	if (SmkAnims[pos] == nullptr)
		return;

	SmkAnims[pos]->setMute(!on);
}

/*------------------------------------------------
					StartSmackAnim
--------------------------------------------------*/
void AnimManager::StartSmackAnim(uint16 num) {
	int pos;

	_curSmackAction = SMACKOPEN;

	// choose the buffer to use
	if (AnimTab[num]._flag & SMKANIM_BKG)
		pos = 0;
	else if (AnimTab[num]._flag & SMKANIM_ICON)
		pos = 2;
	else {
		pos = 1;

		_animMaxX = 0;
		_animMinX = MAXX;
		_animMaxY = 0;
		_animMinY = MAXY;
	}

	_curSmackBuffer = pos;

	if (_playingAnims[pos] != 0) {
		_curSmackAction = SMACKNULL;
		StopSmackAnim(_playingAnims[pos]);
		_curSmackAction = SMACKOPEN;
	}

	_playingAnims[pos] = num;
	_curAnimFrame[pos] = 0;

	// choose how to open
	if (AnimTab[num]._flag & SMKANIM_BKG) {
		sprintf(_vm->UStr, "Bkg\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));

		// Turns off when not needed
		if ((num == aBKG11) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG14) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG1C) && (_vm->_obj[oFAX17]._flag & OBJFLAG_EXTRA)) {
			AnimTab[num]._flag |= SMKANIM_OFF1;
			CallSmackVolumePan(0, 1, 0);
		} else if ((num == aBKG1D) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG22) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG48) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG4P) && (AnimTab[num]._flag & SMKANIM_OFF1))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG28) && (AnimTab[num]._flag & SMKANIM_OFF4))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG37) && (!(_vm->_room[_vm->_curRoom]._flag & OBJFLAG_EXTRA)))
			CallSmackVolumePan(0, 1, 0);
		else if ((num == aBKG2E) && (AnimTab[num]._flag & SMKANIM_OFF2))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG2G) && (_choice[556]._flag & OBJFLAG_DONE))
			CallSmackVolumePan(0, 2, 0);
		else if ((num == aBKG34) &&                                     // If it's BKG 34 and
		         ((_choice[616]._flag & OBJFLAG_DONE) ||                // if the FMV is already done or
		          (_vm->_obj[oTUBOT34]._mode & OBJMODE_OBJSTATUS) ||    // if the whole tube is available or
		          (_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS) ||   // if the outside of the tube is available or
		          (_vm->_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)))  // if the valve is closed
			CallSmackVolumePan(0, 2, 0);
	} else if (AnimTab[num]._flag & SMKANIM_ICON) {
		sprintf(_vm->UStr, "Icon\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));
	} else {
		uint32 st = ReadTime();

		sprintf(_vm->UStr, "Anim\\%s", AnimTab[num]._name);
		CallSmackOpen(AnimFileOpen(AnimTab[num]._name));
		_vm->NextRefresh += (ReadTime() - st); // fixup opening time
	}

	_curSmackAction = SMACKNULL;
}

/*------------------------------------------------
				StopSmackAnim
--------------------------------------------------*/
void AnimManager::StopSmackAnim(uint16 num) {
	if (num == 0)
		return;

	_curSmackAction = SMACKCLOSE;

	int pos = 0;

	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos++;

	if (pos >= MAXSMACK) {
		if (AnimTab[num]._flag & SMKANIM_BKG)
			pos = 0;
		else if (AnimTab[num]._flag & SMKANIM_ICON)
			pos = 2;
		else
			pos = 1;
	}

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	_curSmackBuffer = pos;
	CallSmackClose();

	_curSmackAction = SMACKNULL;

	_vm->_lightIcon = 0xFF;
}

/*-------------------------------------------------
				StopAllSmackAnims
 --------------------------------------------------*/
void AnimManager::StopAllSmackAnims() {
	for (int a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a])
			StopSmackAnim(_playingAnims[a]);
	}
}

/*------------------------------------------------
				StartFullMotion
--------------------------------------------------*/
void AnimManager::StartFullMotion(const char *name) {
	// Stops all the other animations
	for (int pos = 0; pos < MAXSMACK; pos++) {
		if (_playingAnims[pos] != 0) {
			_curSmackAction = SMACKNULL;
			StopSmackAnim(_playingAnims[pos]);
		}
	}
	_curSmackAction = SMACKOPEN;
	_curSmackBuffer = 1;

	_playingAnims[_curSmackBuffer] = FULLMOTIONANIM;
	_curAnimFrame[_curSmackBuffer] = 0;

	FlagShowCharacter = false;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
	TextStatus = TEXT_OFF;
	wordset(_vm->_video2, 0, TOP * MAXX);
	_vm->_graphicsMgr->showScreen(0, 0, MAXX, TOP);
	wordset(_vm->_video2 + (TOP + AREA) * MAXX, 0, TOP * MAXX);
	wordset(_vm->_video2, 0, MAXX * MAXY);
	_vm->_graphicsMgr->showScreen(0, AREA + TOP, MAXX, TOP);
	_vm->_graphicsMgr->unlock();

	_vm->_gameQueue.initQueue();
	_vm->_animQueue.initQueue();
	_vm->_characterQueue.initQueue();
	actorStop();
	FlagMouseEnabled = false;

	CallSmackOpen(FmvFileOpen(name));

	_curSmackAction = SMACKNULL;
}

/*------------------------------------------------
				StopFullMotion
--------------------------------------------------*/
void AnimManager::StopFullMotion() {
	_curSmackAction = SMACKCLOSE;
	_curSmackBuffer = 1;

	if (_playingAnims[_curSmackBuffer] == 0)
		return;

	_playingAnims[_curSmackBuffer] = 0;
	_curAnimFrame[_curSmackBuffer] = 0;

	CallSmackClose();

	_curSmackAction = SMACKNULL;

	FlagDialogActive = false;
	FlagDialogMenuActive = false;
	FlagMouseEnabled = true;
	FlagSomeOneSpeak = false;

	_vm->_lightIcon = 0xFF;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
	if (_curDialog == dFCRED) {
		CloseSys(nullptr);
		return;
	}

	if (!((_curDialog == dNEGOZIANTE1A) && (_curChoice == 185))) {
		if ((_curDialog == dF582) || (_curDialog == dFLOG) || (_curDialog == dINTRO) || (_curDialog == dF362) || (_curDialog == dC381) || (_curDialog == dF381) ||
		    (_curDialog == dF491) || ((_curDialog == dC581) && !(_choice[886]._flag & OBJFLAG_DONE) && (_choice[258]._flag & OBJFLAG_DONE)) ||
		    ((_curDialog == dC5A1) && (_vm->_room[r5A]._flag & OBJFLAG_EXTRA)))
			FlagShowCharacter = false;
		else
			RedrawRoom();

		if (_curDialog == dF582)
			SoundFadOut();
	}
}

/*------------------------------------------------
					RegenAnim
--------------------------------------------------*/
void AnimManager::RegenAnim(int box) {
	for (int a = 0; a < MAXSMACK; a++) {
		if ((_playingAnims[a] != 0) && (box == BACKGROUND)) {
			if ((a == 1) && (_playingAnims[a] == FULLMOTIONANIM))
				RegenFullMotion();
			else if (a != 1)
				RegenSmackAnim(_playingAnims[a]);
		}
	}
}

/*-------------------------------------------------
				RefreshAllAnimations
 --------------------------------------------------*/
void AnimManager::RefreshAllAnimations() {
	soundtimefunct();

	for (int a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a]) {
			_curSmackBuffer = a;
			CallSmackNextFrame();
		}
	}
}

/*------------------------------------------------
					RegenSmackAnim
--------------------------------------------------*/
void AnimManager::RegenSmackAnim(int num) {
	if ((num == 0) || (num == FULLMOTIONANIM))
		return;

	if (AnimTab[num]._flag & SMKANIM_ICON) {
		RegenSmackIcon(_vm->_regenInvStartIcon, num);
		return;
	}

	int pos = 0;
	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos++;

	if (pos >= MAXSMACK) {
		if (AnimTab[num]._flag & SMKANIM_BKG)
			pos = 0;
		else if (AnimTab[num]._flag & SMKANIM_ICON)
			pos = 2;
		else
			pos = 1;
	}

	_curSmackBuffer = pos;

	if (SmkAnims[_curSmackBuffer] == nullptr)
		return;

	_curAnimFrame[pos]++;

	if (SmkAnims[pos]->hasDirtyPalette()) {
		for (int32 a = 0; a < 256; a++) {
			_smackPal[pos][a] = _vm->_graphicsMgr->palTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0],
			                                                   SmkAnims[pos]->getPalette()[a * 3 + 1],
			                                                   SmkAnims[pos]->getPalette()[a * 3 + 2]);
		}
	}

	while (const Common::Rect *lastRect = SmkAnims[pos]->getNextDirtyRect()) {
		int inters = 0;
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (AnimTab[num]._flag & (SMKANIM_OFF1 << a)) {
				// if the rectangle is completely included in the limit, raise it
				if ((AnimTab[num]._lim[a][0] <= lastRect->right) &&
				    (AnimTab[num]._lim[a][1] <= lastRect->bottom) &&
				    (AnimTab[num]._lim[a][2] >= lastRect->left) &&
				    (AnimTab[num]._lim[a][3] >= lastRect->top)) {
					inters++;
				}
			}
		}

		if ((_curAnimFrame[pos] > 0) && (inters == 0)) {
			if (pos == 0) {
				for (int32 a = 0; a < lastRect->height(); a++) {
					byte2wordn(_vm->_video2 + lastRect->left + (lastRect->top + a + TOP) * MAXX,
					           _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
					           _smackPal[pos], lastRect->width());

					AddLine(lastRect->left, lastRect->right, lastRect->top + a + TOP);

					wordcopy(ImagePointer + lastRect->left + (lastRect->top + a) * MAXX,
					         _vm->_video2 + lastRect->left + (lastRect->top + a + TOP) * MAXX,
					         lastRect->width());
				}
			} else if (_curAnimFrame[pos] > 1) {
				_animMinX = MIN((uint16)lastRect->left, _animMinX);
				_animMinY = MIN((uint16)lastRect->top, _animMinY);

				_animMaxX = MAX((uint16)lastRect->right, _animMaxX);
				_animMaxY = MAX((uint16)lastRect->bottom, _animMaxY);
			}
		}
	}

	if (pos == 0) {
		// If it's a background
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (!(AnimTab[num]._flag & (SMKANIM_OFF1 << a)) && (AnimTab[num]._lim[a][3] != 0)) {
				_vm->_limits[_vm->_limitsNum][0] = AnimTab[num]._lim[a][0];
				_vm->_limits[_vm->_limitsNum][1] = AnimTab[num]._lim[a][1] + TOP;
				_vm->_limits[_vm->_limitsNum][2] = AnimTab[num]._lim[a][2];
				_vm->_limits[_vm->_limitsNum][3] = AnimTab[num]._lim[a][3] + TOP;
				_vm->_limitsNum++;
			}
		}
	} else if (pos == 1) {
		// Only for the character
		if (_curAnimFrame[pos] == 1) {
			for (uint16 b = 0; b < AREA; b++) {
				for (uint16 a = 0; a < MAXX; a++) {
					if (_smackBuffer[pos][b * MAXX + a]) {
						_animMinX = MIN(a, _animMinX);
						_animMinY = MIN(b, _animMinY);

						_animMaxX = MAX(a, _animMaxX);
						_animMaxY = MAX(b, _animMaxY);
					}
				}

				_animMaxX = MIN(MAXX, _animMaxX + 1);
				_animMaxY = MIN(AREA, _animMaxY + 1);
			}
		}

		for (int32 a = 0; a < _animMaxY - _animMinY; a++) {
			byte2wordm(_vm->_video2 + _animMinX + (_animMinY + a + TOP) * MAXX,
			           _smackBuffer[pos] + _animMinX + (_animMinY + a) * SmkAnims[pos]->getWidth(),
			           _smackPal[pos], _animMaxX - _animMinX);

			AddLine(_animMinX, _animMaxX, _animMinY + a + TOP);
		}

		_vm->_limits[_vm->_limitsNum][0] = _animMinX;
		_vm->_limits[_vm->_limitsNum][1] = _animMinY + TOP;
		_vm->_limits[_vm->_limitsNum][2] = _animMaxX;
		_vm->_limits[_vm->_limitsNum][3] = _animMaxY + TOP;

		_vm->_actorLimit = _vm->_limitsNum;
		_vm->_limitsNum++;
	}

	if (!(AnimTab[num]._flag & SMKANIM_LOOP) && !(AnimTab[num]._flag & SMKANIM_BKG)) {
		if (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount()) {
			StopSmackAnim(num);
			FlagPaintCharacter = true;

			_animMaxX = 0;
			_animMinX = MAXX;
			_animMaxY = 0;
			_animMinY = MAXY;
		} else
			CallSmackNextFrame();

	} else
		CallSmackNextFrame();

	if ((SmkAnims[pos] != NULL) && (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount())) {
		if ((AnimTab[num]._flag & SMKANIM_LOOP) || (AnimTab[num]._flag & SMKANIM_BKG))
			InitAtFrameHandler(num, 0);

		_curAnimFrame[pos] = 0;
	}
}

/*------------------------------------------------
			Refresh FullMotion
--------------------------------------------------*/
void AnimManager::RegenFullMotion() {
	int32 yfact;

	int pos = 1;
	_curSmackBuffer = pos;

	if (((_curAnimFrame[pos] + 1) >= _fullMotionStart) && ((_curAnimFrame[pos] + 1) <= _fullMotionEnd)) {
		_curAnimFrame[pos]++;

		if (SmkAnims[pos]->hasDirtyPalette()) {
			for (int32 a = 0; a < 256; a++) {
				_smackPal[pos][a] = _vm->_graphicsMgr->palTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0],
				                                                  SmkAnims[pos]->getPalette()[a * 3 + 1],
				                                                  SmkAnims[pos]->getPalette()[a * 3 + 2]);

				_vm->_newData[a] = _smackPal[pos][a];
				_vm->_newData2[a] = (uint32)((uint32)_smackPal[pos][a] + (((uint32)_smackPal[pos][a]) << 16));
			}
		}

		DialogHandler(_curAnimFrame[pos]);

		_vm->sdt.dx = TextLength(_vm->sdt.sign, 0);

		_vm->sdt.x = 20;
		_vm->sdt.y = 380;
		_vm->sdt.dx = MAXX - 40;
		_vm->sdt.dy = _vm->sdt.checkDText();
		_vm->sdt.l[0] = 0;
		_vm->sdt.l[1] = 0;
		_vm->sdt.l[2] = MAXX;
		_vm->sdt.l[3] = MAXY;
		_vm->sdt.scol = MASKCOL;

		// If text was displayed, remove it
		if (_vm->osdt.sign != nullptr) {
			if ((_vm->osdt.y < _vm->sdt.y) || (_vm->osdt.y + _vm->osdt.dy > _vm->sdt.y + _vm->sdt.dy) || (_vm->sdt.sign == nullptr)) {
				PaintSmackBuffer(0, _vm->osdt.y - TOP, MAXX, _vm->osdt.dy);
				_vm->_graphicsMgr->showScreen(0, _vm->osdt.y, MAXX, _vm->osdt.dy);
			}
			_vm->osdt.sign = nullptr;
		}
		// If there's text
		if (_vm->sdt.sign != nullptr) {
			PaintSmackBuffer(0, _vm->sdt.y - TOP, MAXX, _vm->sdt.dy);
			// Write string
			if (ConfMan.getBool("subtitles"))
				_vm->sdt.DText();
			// and show it
			_vm->osdt.sign = nullptr;
		}

		if (SmkAnims[pos]->getHeight() > MAXY / 2)
			yfact = 1;
		else
			yfact = 2;

		while (const Common::Rect *lastRect = SmkAnims[pos]->getNextDirtyRect()) {
			for (int32 a = 0; a < lastRect->height(); a++) {
				// if it's already copied
				if ((_vm->sdt.sign == nullptr) ||
				    ((lastRect->top + a) * yfact < (_vm->sdt.y - TOP)) ||
				    ((lastRect->top + a) * yfact >= (_vm->sdt.y + _vm->sdt.dy - TOP))) {
					// Decide to double or not...
					// in height
					if (SmkAnims[pos]->getHeight() > MAXY / 2) {
						// in width
						if (SmkAnims[pos]->getWidth() > MAXX / 2) {
							_vm->_graphicsMgr->BCopy(lastRect->left + (lastRect->top + a) * MAXX + ((MAXY - SmkAnims[pos]->getHeight()) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
						} else {
							_vm->_graphicsMgr->DCopy(lastRect->left * 2 + (lastRect->top + a) * MAXX + ((MAXY - SmkAnims[pos]->getHeight()) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
						}
					} else {
						if (SmkAnims[pos]->getWidth() > MAXX / 2) {
							_vm->_graphicsMgr->BCopy(lastRect->left + ((lastRect->top + a) * 2) * MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
							_vm->_graphicsMgr->BCopy(lastRect->left + ((lastRect->top + a) * 2 + 1) * MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
						} else {
							_vm->_graphicsMgr->DCopy(lastRect->left * 2 + ((lastRect->top + a) * 2) * MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
							_vm->_graphicsMgr->DCopy(lastRect->left * 2 + ((lastRect->top + a) * 2 + 1) * MAXX + ((MAXY - SmkAnims[pos]->getHeight() * 2) / 2) * MAXX,
							                          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
							                          lastRect->width());
						}
					}
				}
			}
		}
		if (_vm->sdt.sign != nullptr)
			_vm->_graphicsMgr->showScreen(0, _vm->sdt.y, MAXX, _vm->sdt.dy);
		_vm->_graphicsMgr->unlock();

		if (_curAnimFrame[pos] == _fullMotionEnd) {
			PaintSmackBuffer(0, 0, MAXX, AREA);
			doEvent(MC_DIALOG, ME_ENDCHOICE, MP_HIGH, _curAnimFrame[pos], 0, 0, 0);
			CallSmackSoundOnOff(pos, false);
		} else {
			CallSmackNextFrame();

			if (_curAnimFrame[pos] >= SmkAnims[pos]->getFrameCount())
				StopFullMotion();
		}
	}
}

/*------------------------------------------------
			Refresh Smacker Icons
--------------------------------------------------*/
void AnimManager::RegenSmackIcon(int StartIcon, int num) {
	int pos = MAXSMACK - 1;
	_curAnimFrame[pos]++;

	_curSmackBuffer = pos;

	if (SmkAnims[_curSmackBuffer] == nullptr)
		return;

	int stx = ICONMARGSX;
	int32 a;
	for (a = 0; a < ICONSHOWN; a++) {
		if (_vm->_inventory[a + StartIcon] == (num - aiBANCONOTE + 1)) {
			stx = a * ICONDX + ICONMARGSX + CurScrollPageDx;
			break;
		}
	}
	if (a == ICONSHOWN)
		return;

	if (SmkAnims[pos]->hasDirtyPalette()) {
		for (a = 0; a < 256; a++) {
			_smackPal[pos][a] = _vm->_graphicsMgr->palTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0],
			                                                  SmkAnims[pos]->getPalette()[a * 3 + 1],
			                                                  SmkAnims[pos]->getPalette()[a * 3 + 2]);
		}
	}

	while (const Common::Rect *lastRect = SmkAnims[pos]->getNextDirtyRect()) {
		for (a = 0; a < ICONDY - lastRect->top; a++) {
			byte2word(_vm->_video2 + lastRect->left + stx + (lastRect->top + a + FIRSTLINE) * SCREENLEN,
			          _smackBuffer[pos] + lastRect->left + (lastRect->top + a) * SmkAnims[pos]->getWidth(),
			          _smackPal[pos], lastRect->width());

			AddLine(lastRect->left + stx, lastRect->right + stx, lastRect->top + a + FIRSTLINE);
		}
	}

	CallSmackNextFrame();
}

/*------------------------------------------------
					PlayFullMotion
--------------------------------------------------*/
void AnimManager::PlayFullMotion(int start, int end) {
	extern unsigned short _curDialog;

	int pos = 1;
	_curSmackBuffer = pos;

	if (start < 1)
		start = 1;
	if (end < 1)
		end = 1;

	if (start > SmkAnims[pos]->getFrameCount()) {
		start = SmkAnims[pos]->getFrameCount() - 1;
	}
	if (end > SmkAnims[pos]->getFrameCount()) {
		end = SmkAnims[pos]->getFrameCount();
	}

	//	If choices are attached
	if (_curAnimFrame[pos] != (start - 1)) {
		for (int a = 0; a < MAXNEWSMKPAL; a++) {
			if (((_dialog[_curDialog]._newPal[a] > start) || !(_dialog[_curDialog]._newPal[a])) && (a)) {
				SmkAnims[pos]->seekToFrame(_dialog[_curDialog]._newPal[a - 1]);
				for (a = 0; a < 256; a++) {
					_smackPal[pos][a] = _vm->_graphicsMgr->palTo16bit(SmkAnims[pos]->getPalette()[a * 3 + 0],
					                                                  SmkAnims[pos]->getPalette()[a * 3 + 1],
					                                                  SmkAnims[pos]->getPalette()[a * 3 + 2]);

					_vm->_newData[a] = _smackPal[pos][a];
					_vm->_newData2[a] = (uint32)((uint32)_smackPal[pos][a] + (((uint32)_smackPal[pos][a]) << 16));
				}
				break;
			}

			if ((_dialog[_curDialog]._newPal[a] == 0) || (_dialog[_curDialog]._newPal[a] == start))
				break;
		}

		if ((end - start) > 2) {
			if (start > 10)
				SmkAnims[pos]->seekToFrame(start - 10);
			else
				SmkAnims[pos]->seekToFrame(1);

			CallSmackSoundOnOff(pos, true);
		} else
			SmkAnims[pos]->seekToFrame(start);

		_curAnimFrame[pos] = start - 1;
	} else if ((end - start) > 2)
		CallSmackSoundOnOff(pos, true);

	_fullMotionStart = start;
	_fullMotionEnd = end;

	_vm->sdt.clear();
	_vm->osdt.clear();
}

/*-------------------------------------------------
					PaintSmackBuffer
 --------------------------------------------------*/
void AnimManager::PaintSmackBuffer(int px, int py, int dx, int dy) {
	int pos = 1;
	for (int a = 0; a < dy; a++) {
		if (SmkAnims[pos]->getHeight() > MAXY / 2) {
			if (SmkAnims[pos]->getWidth() > MAXX / 2)
				byte2word(_vm->_video2 + (a + py + TOP) * MAXX + px,
				          _smackBuffer[pos] + (a + py) * SmkAnims[pos]->getWidth() + px, _vm->_newData, dx);
			else
				byte2long(_vm->_video2 + (a + py + TOP) * MAXX + px,
				          _smackBuffer[pos] + (a + py) * SmkAnims[pos]->getWidth() + px / 2, _vm->_newData2, dx / 2);
		} else {
			if (SmkAnims[pos]->getWidth() > MAXX / 2)
				byte2word(_vm->_video2 + (a + py + TOP) * MAXX + px,
				          _smackBuffer[pos] + ((a + py) / 2) * SmkAnims[pos]->getWidth() + px, _vm->_newData, dx);
			else
				byte2long(_vm->_video2 + (a + py + TOP) * MAXX + px,
				          _smackBuffer[pos] + ((a + py) / 2) * SmkAnims[pos]->getWidth() + px / 2, _vm->_newData2, dx / 2);
		}
	}
}

} // namespace Trecision
