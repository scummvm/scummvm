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

#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "common/events.h"

#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/video.h"
#include "trecision/nl/message.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"

namespace Trecision {

bool NightlongSmackerDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (SmackerDecoder::loadStream(stream)) {
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

bool NightlongSmackerDecoder::forceSeekToFrame(uint frame) {
	if (!isVideoLoaded())
		return true;

	if (frame >= getFrameCount())
		return false;

	if (!rewind())
		return false;

	SmackerVideoTrack *videoTrack = (SmackerVideoTrack *)getTrack(0);
	uint32 start = _fileStream->pos();
	uint32 offset = 0;
	for (uint32 i = 0; i < frame; i++) {
		videoTrack->increaseCurFrame();
		// Frames with palette data contain palette entries which use
		// the previous palette as their base. Therefore, we need to
		// parse all palette entries up to the requested frame
		if (_frameTypes[videoTrack->getCurFrame()] & 1) {
			_fileStream->seek(start + offset, SEEK_SET);
			videoTrack->unpackPalette(_fileStream);
		}
		offset += _frameSizes[i] & ~3;
	}

	_startTime = g_system->getMillis() - (videoTrack->getFrameTime(frame).msecs() / getRate()).toInt();
	
	return _fileStream->seek(start + offset, SEEK_SET);
}

AnimManager::AnimManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < MAXSMACK; ++i) {
		_smkBuffer[i] = nullptr;
		_smkAnims[i] = nullptr;
		_playingAnims[i] = 0;
		_curAnimFrame[i] = 0;

		for (int j = 0; j < 256; ++j)
			_smkPal[i][j] = 0;
	}

	for (int i = 0; i < MAXANIM; ++i) {
		_animTab[i]._flag = 0;
		_animTab[i]._name[0] = '\0';
	}
	
	_animMaxX = _animMinX = 0;
	_animMaxY = _animMinY = 0;

	_curSmackBuffer = kSmackerBackground;

#if (!USE_NEW_VIDEO_CODE)
	_fullMotionStart = _fullMotionEnd = 0;
#endif

	_curCD = 1;
	swapCD(_curCD);
}

AnimManager::~AnimManager() {
	for (int i = 0; i < MAXSMACK; ++i) {
		delete _smkAnims[i];
		_smkAnims[i] = nullptr;
		_animFile[i].close();
	}
}

#if USE_NEW_VIDEO_CODE

void AnimManager::playMovie(Common::String filename, int startFrame, int endFrame) {
	NightlongSmackerDecoder *smkDecoder = new NightlongSmackerDecoder;

	if (!smkDecoder->loadFile(filename))
		return;

	Common::Event event;
	bool skipVideo = false;
	uint16 x = (g_system->getWidth() - smkDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - smkDecoder->getHeight()) / 2;
	g_vm->_sdText.text = nullptr;

	smkDecoder->start();
	//debug("playMovie %s, %d - %d", filename.c_str(), startFrame, endFrame);
	
	setVideoRange(smkDecoder, startFrame, endFrame);
	
	while (!g_vm->shouldQuit() && !smkDecoder->endOfVideo() && smkDecoder->getCurFrame() < endFrame && !skipVideo) {
		if (smkDecoder->needsUpdate()) {
			drawFrame(smkDecoder, x, y, true);
		}

		while (g_engine->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	doEvent(MC_DIALOG, ME_ENDCHOICE, MP_HIGH, smkDecoder->getCurFrame(), 0, 0, 0);
	delete smkDecoder;
}

void AnimManager::setVideoRange(NightlongSmackerDecoder *smkDecoder, int &startFrame, int &endFrame) {
	uint16 x = (g_system->getWidth() - smkDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - smkDecoder->getHeight()) / 2;
	startFrame = CLIP<int32>(startFrame, 1, smkDecoder->getFrameCount() - 1) - 1;
	endFrame = CLIP<int32>(endFrame, 1, smkDecoder->getFrameCount()) - 1;

	//	If choices are attached
	if (smkDecoder->getCurFrame() != startFrame) {
		for (int a = 0; a < MAXNEWSMKPAL; a++) {
			if ((_dialog[_curDialog]._newPal[a] > startFrame || !_dialog[_curDialog]._newPal[a]) && a) {
				smkDecoder->forceSeekToFrame(_dialog[_curDialog]._newPal[a - 1]);
				break;
			}

			if (!_dialog[_curDialog]._newPal[a] || _dialog[_curDialog]._newPal[a] == startFrame)
				break;
		}

		if (endFrame - startFrame > 2) {
			if (startFrame > 10)
				smkDecoder->forceSeekToFrame(startFrame - 10);
			else
				smkDecoder->forceSeekToFrame(0);

			while (smkDecoder->getCurFrame() < startFrame) {
				drawFrame(smkDecoder, x, y, false);
			}
		} else
			smkDecoder->forceSeekToFrame(startFrame);
	}
	if (endFrame - startFrame > 2)
		smkDecoder->setMute(false);
}

void AnimManager::drawFrame(NightlongSmackerDecoder *smkDecoder, uint16 x, uint16 y, bool updateScreen) {
	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	if (frame) {
		Graphics::Surface *frame16 = frame->convertTo(g_system->getScreenFormat(), smkDecoder->getPalette());
		drawFrameSubtitles(frame16, smkDecoder->getCurFrame());
		g_system->copyRectToScreen(frame16->getPixels(), frame16->pitch, x, y, frame16->w, frame16->h);
		delete frame16;

		if (updateScreen)
			g_vm->_system->updateScreen();
	}
}

void AnimManager::drawFrameSubtitles(Graphics::Surface *surface, int frameNum) {
	if (!ConfMan.getBool("subtitles"))
		return;

	DialogHandler(frameNum);
	if (_vm->_sdText.text == nullptr)
		return;
	
	// Subtitles can be placed in different coordinates in the video,
	// which are set inside DialogHandler(), but are then reset to
	// fixed coordinates
	_vm->_sdText.x = 20;
	_vm->_sdText.y = 380 - TOP;
	_vm->_sdText.dx = MAXX - 40;
	_vm->_sdText.dy = _vm->_sdText.checkDText();
	_vm->_sdText._subtitleRect.left = 0;
	_vm->_sdText._subtitleRect.top = 0;
	_vm->_sdText._subtitleRect.right = MAXX;
	_vm->_sdText._subtitleRect.bottom = MAXY;
	_vm->_sdText.scol = MASKCOL;
	_vm->_sdText.DText((uint16 *)surface->getPixels());
}

#else

void AnimManager::openSmkVideo(Common::String name) {
	//debug("Opening video %s", name.c_str());

	Common::File *smkFile = new Common::File();
	smkFile->open(name);

	if (!smkFile->isOpen()) {
		warning("openSmkVideo: File %s not found", name.c_str());
		return;
	}

	openSmk(smkFile);
}
#endif

void AnimManager::openSmkAnim(Common::String name) {
	//debug("Opening anim %s", name.c_str());
	if (_animFile[_curSmackBuffer].hasFile(name)) {
		openSmk(_animFile[_curSmackBuffer].createReadStreamForMember(name));
		return;
	}

	_curCD = _curCD == 1 ? 2 : 1;
	swapCD(_curCD);

	if (_animFile[_curSmackBuffer].hasFile(name)) {
		openSmk(_animFile[_curSmackBuffer].createReadStreamForMember(name));
		return;
	}

	// Invalid file
	error("openSmkAnim(): File %s not found", name.c_str());
}

void AnimManager::openSmk(Common::SeekableReadStream *stream) {
	_smkAnims[_curSmackBuffer] = new NightlongSmackerDecoder();

	if (!_smkAnims[_curSmackBuffer]->loadStream(stream)) {
		warning("Invalid SMK file");
		closeSmk();
	} else {
		_smkAnims[_curSmackBuffer]->start();
		smkNextFrame();
	}
}

void AnimManager::closeSmk() {
	delete _smkAnims[_curSmackBuffer];
	_smkAnims[_curSmackBuffer] = nullptr;
}

void AnimManager::smkNextFrame() {
	if (_smkAnims[_curSmackBuffer] == nullptr)
		return;

	// Loop
	if (_smkAnims[_curSmackBuffer]->getCurFrame() >= (int)_smkAnims[_curSmackBuffer]->getFrameCount() - 1) {
		_smkAnims[_curSmackBuffer]->rewind();
		_smkAnims[_curSmackBuffer]->decodeNextFrame(); // Skip frame 0
	}

	const Graphics::Surface *surface = _smkAnims[_curSmackBuffer]->decodeNextFrame();
	_smkBuffer[_curSmackBuffer] = (uint8 *)surface->getPixels();
}

void AnimManager::smkGoto(int buf, int num) {
	_curSmackBuffer = buf;

	if (_smkAnims[_curSmackBuffer] == nullptr)
		return;

	_smkAnims[_curSmackBuffer]->forceSeekToFrame(num);
}

void AnimManager::smkVolumePan(int buf, int track, int vol) {
	_curSmackBuffer = buf;

	if (_smkAnims[_curSmackBuffer] == nullptr)
		return;

	_smkAnims[_curSmackBuffer]->muteTrack(track, vol == 0);
}

void AnimManager::smkSoundOnOff(int pos, bool on) {
	if (_smkAnims[pos] == nullptr)
		return;

	_smkAnims[pos]->setMute(!on);
}

void AnimManager::startSmkAnim(uint16 num) {
	int pos;

	// choose the buffer to use
	if (_animTab[num]._flag & SMKANIM_BKG)
		pos = kSmackerBackground;
	else if (_animTab[num]._flag & SMKANIM_ICON)
		pos = kSmackerIcon;
	else {
		pos = kSmackerAction;

		_animMaxX = 0;
		_animMinX = MAXX;
		_animMaxY = 0;
		_animMinY = MAXY;
	}

	_curSmackBuffer = pos;

	if (_playingAnims[pos] != 0) {
		stopSmkAnim(_playingAnims[pos]);
	}

	_playingAnims[pos] = num;
	_curAnimFrame[pos] = 0;

	// choose how to open
	if (_animTab[num]._flag & SMKANIM_BKG) {
		openSmkAnim(_animTab[num]._name);

		// Turns off when not needed
		if ((num == aBKG11) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG14) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG1C) && (_vm->_obj[oFAX17]._flag & OBJFLAG_EXTRA)) {
			_animTab[num]._flag |= SMKANIM_OFF1;
			smkVolumePan(0, 1, 0);
		} else if ((num == aBKG1D) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG22) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG48) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG4P) && (_animTab[num]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG28) && (_animTab[num]._flag & SMKANIM_OFF4))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG37) && (!(_vm->_room[_vm->_curRoom]._flag & OBJFLAG_EXTRA)))
			smkVolumePan(0, 1, 0);
		else if ((num == aBKG2E) && (_animTab[num]._flag & SMKANIM_OFF2))
			smkVolumePan(0, 2, 0);
		else if ((num == aBKG2G) && (g_vm->_choice[556]._flag & OBJFLAG_DONE))
			smkVolumePan(0, 2, 0);
		else if ((num == aBKG34) &&                                     // If it's BKG 34 and
		         ((g_vm->_choice[616]._flag & OBJFLAG_DONE) ||          // if the FMV is already done or
		          (_vm->_obj[oTUBOT34]._mode & OBJMODE_OBJSTATUS) ||    // if the whole tube is available or
		          (_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS) ||   // if the outside of the tube is available or
		          (_vm->_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)))  // if the valve is closed
			smkVolumePan(0, 2, 0);
	} else if (_animTab[num]._flag & SMKANIM_ICON) {
		openSmkAnim(_animTab[num]._name);
	} else {
		uint32 st = ReadTime();

		openSmkAnim(_animTab[num]._name);
		_vm->_nextRefresh += (ReadTime() - st); // fixup opening time
	}
}

void AnimManager::stopSmkAnim(uint16 num) {
	if (num == 0)
		return;

	int pos = kSmackerBackground;

	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos++;

	if (pos >= MAXSMACK) {
		if (_animTab[num]._flag & SMKANIM_BKG)
			pos = kSmackerBackground;
		else if (_animTab[num]._flag & SMKANIM_ICON)
			pos = kSmackerIcon;
		else
			pos = kSmackerAction;
	}

	_playingAnims[pos] = 0;
	_curAnimFrame[pos] = 0;

	_curSmackBuffer = pos;
	closeSmk();

	_vm->_lightIcon = 0xFF;
}

void AnimManager::stopAllSmkAnims() {
	for (int a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a])
			stopSmkAnim(_playingAnims[a]);
	}
}

void AnimManager::startFullMotion(const char *name) {
	stopAllSmkAnims();

#if (!USE_NEW_VIDEO_CODE)
	_curSmackBuffer = kSmackerFullMotion;
	_playingAnims[_curSmackBuffer] = FULLMOTIONANIM;
	_curAnimFrame[_curSmackBuffer] = 0;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
#endif

	g_vm->_flagShowCharacter = false;
	TextStatus = TEXT_OFF;
	memset(_vm->_screenBuffer, 0, TOP * MAXX * 2);
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);
	memset(_vm->_screenBuffer + (TOP + AREA) * MAXX, 0, TOP * MAXX * 2);
	memset(_vm->_screenBuffer, 0, MAXX * MAXY * 2);
	_vm->_graphicsMgr->copyToScreen(0, AREA + TOP, MAXX, TOP);

	_vm->_gameQueue.initQueue();
	_vm->_animQueue.initQueue();
	_vm->_characterQueue.initQueue();
	actorStop();
	g_vm->_flagMouseEnabled = false;

#if (!USE_NEW_VIDEO_CODE)
	openSmkVideo(name);
#endif
}

void AnimManager::stopFullMotion() {
#if (!USE_NEW_VIDEO_CODE)
	_curSmackBuffer = kSmackerFullMotion;

	if (_playingAnims[_curSmackBuffer] == 0)
		return;

	_playingAnims[_curSmackBuffer] = 0;
	_curAnimFrame[_curSmackBuffer] = 0;
	_fullMotionStart = 0;
	_fullMotionEnd = 0;
#endif

	closeSmk();

	g_vm->_flagDialogActive = false;
	g_vm->_flagDialogMenuActive = false;
	g_vm->_flagMouseEnabled = true;
	g_vm->_flagSomeoneSpeaks = false;

	_vm->_lightIcon = 0xFF;
	if (_curDialog == dFCRED) {
		g_vm->quitGame();
		return;
	}

	if (!((_curDialog == dSHOPKEEPER1A) && (_curChoice == 185))) {
		if ((_curDialog == dF582) || (_curDialog == dFLOG) || (_curDialog == dINTRO) || (_curDialog == dF362) || (_curDialog == dC381) || (_curDialog == dF381) ||
		    (_curDialog == dF491) || ((_curDialog == dC581) && !(g_vm->_choice[886]._flag & OBJFLAG_DONE) && (g_vm->_choice[258]._flag & OBJFLAG_DONE)) ||
		    ((_curDialog == dC5A1) && (_vm->_room[r5A]._flag & OBJFLAG_EXTRA)))
			g_vm->_flagShowCharacter = false;
		else
			RedrawRoom();

		if (_curDialog == dF582)
			SoundFadOut();
	}
}

void AnimManager::refreshAnim(int box) {
	for (int a = 0; a < MAXSMACK; a++) {
		if ((_playingAnims[a] != 0) && (box == BACKGROUND)) {
#if (!USE_NEW_VIDEO_CODE)
			if ((a == 1) && (_playingAnims[a] == FULLMOTIONANIM)) {
				refreshFullMotion();
			} else
#endif
			if (a != 1) {
				refreshSmkAnim(_playingAnims[a]);
			}
		}
	}
}

void AnimManager::refreshAllAnimations() {
	soundtimefunct();

	#if 0
	for (int a = 0; a < MAXSMACK; a++) {
		if (_playingAnims[a]) {
			_curSmackBuffer = a;
			smkNextFrame();
		}
	}
	#endif
}

void AnimManager::refreshPalette(int num) {
	if (_smkAnims[num]->hasDirtyPalette()) {
		const byte *pal = _smkAnims[num]->getPalette();
		for (int32 a = 0; a < 256; a++) {
			_smkPal[num][a] = _vm->_graphicsMgr->palTo16bit(
				pal[a * 3 + 0],
				pal[a * 3 + 1],
				pal[a * 3 + 2]);
		}
	}
}

void AnimManager::refreshSmkAnim(int num) {
	if (num == 0 || num == FULLMOTIONANIM)
		return;

	if (_animTab[num]._flag & SMKANIM_ICON) {
		refreshSmkIcon(_vm->_inventoryRefreshStartIcon, num);
		return;
	}

	int pos = 0;
	while ((pos < MAXSMACK) && (_playingAnims[pos] != num))
		pos++;

	if (pos >= MAXSMACK) {
		if (_animTab[num]._flag & SMKANIM_BKG)
			pos = kSmackerBackground;
		else if (_animTab[num]._flag & SMKANIM_ICON)
			pos = kSmackerIcon;
		else
			pos = kSmackerAction;
	}

	_curSmackBuffer = pos;

	if (_smkAnims[_curSmackBuffer] == nullptr)
		return;

	_curAnimFrame[pos]++;

	refreshPalette(pos);

	while (const Common::Rect *lastRect = _smkAnims[pos]->getNextDirtyRect()) {
		int inters = 0;
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (_animTab[num]._flag & (SMKANIM_OFF1 << a)) {
				// if the rectangle is completely included in the limit, raise it
				if ((_animTab[num]._lim[a].left <= lastRect->right) &&
				    (_animTab[num]._lim[a].top <= lastRect->bottom) &&
				    (_animTab[num]._lim[a].right >= lastRect->left) &&
				    (_animTab[num]._lim[a].bottom >= lastRect->top)) {
					inters++;
				}
			}
		}

		if ((_curAnimFrame[pos] > 0) && (inters == 0)) {
			if (pos == 0) {
				for (int32 a = 0; a < lastRect->height(); a++) {
					byte2wordn(
						_vm->_screenBuffer + lastRect->left + (lastRect->top + a + TOP) * MAXX,
						_smkBuffer[pos] + lastRect->left + (lastRect->top + a) * _smkAnims[pos]->getWidth(),
						_vm->_smackImageBuffer + lastRect->left + (lastRect->top + a) * MAXX,
						_smkPal[pos],
						lastRect->width()
					);

#if USE_DIRTY_RECTS
					AddLine(lastRect->left, lastRect->right, lastRect->top + a + TOP);
#endif

					memcpy(ImagePointer + lastRect->left + (lastRect->top + a) * MAXX,
					         _vm->_screenBuffer + lastRect->left + (lastRect->top + a + TOP) * MAXX,
					         lastRect->width() * 2);
				}
			} else if (_curAnimFrame[pos] > 1) {
				_animMinX = MIN((uint16)lastRect->left, _animMinX);
				_animMinY = MIN((uint16)lastRect->top, _animMinY);

				_animMaxX = MAX((uint16)lastRect->right, _animMaxX);
				_animMaxY = MAX((uint16)lastRect->bottom, _animMaxY);
			}
		}
	}

	if (pos == kSmackerBackground) {
		// If it's a background
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (!(_animTab[num]._flag & (SMKANIM_OFF1 << a)) && (_animTab[num]._lim[a].bottom != 0)) {
				Common::Rect l = _animTab[num]._lim[a];
				l.translate(0, TOP);
				_vm->_limits[_vm->_limitsNum++] = l;
			}
		}
	} else if (pos == kSmackerAction) {
		// Only for the character
		if (_curAnimFrame[pos] == 1) {
			for (uint16 b = 0; b < AREA; b++) {
				for (uint16 a = 0; a < MAXX; a++) {
					if (_smkBuffer[pos][b * MAXX + a]) {
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
			byte2wordm(
				_vm->_screenBuffer + _animMinX + (_animMinY + a + TOP) * MAXX,
				_smkBuffer[pos] + _animMinX + (_animMinY + a) * _smkAnims[pos]->getWidth(),
				_smkPal[pos],
				_animMaxX - _animMinX
			);

#if USE_DIRTY_RECTS
			AddLine(_animMinX, _animMaxX, _animMinY + a + TOP);
#endif
		}

		if (_animMaxX > _animMinX && _animMaxY > _animMinY) {
			_vm->_actorLimit = _vm->_limitsNum;
			_vm->_limits[_vm->_limitsNum++] = Common::Rect(_animMinX, _animMinY + TOP, _animMaxX, _animMaxY + TOP);			
		}
	}

	if (!(_animTab[num]._flag & SMKANIM_LOOP) && !(_animTab[num]._flag & SMKANIM_BKG)) {
		if (_curAnimFrame[pos] >= _smkAnims[pos]->getFrameCount()) {
			stopSmkAnim(num);
			g_vm->_flagPaintCharacter = true;

			_animMaxX = 0;
			_animMinX = MAXX;
			_animMaxY = 0;
			_animMinY = MAXY;
		} else
			smkNextFrame();

	} else
		smkNextFrame();

	if ((_smkAnims[pos] != nullptr) && (_curAnimFrame[pos] >= _smkAnims[pos]->getFrameCount())) {
		if ((_animTab[num]._flag & SMKANIM_LOOP) || (_animTab[num]._flag & SMKANIM_BKG))
			InitAtFrameHandler(num, 0);

		_curAnimFrame[pos] = 0;
	}
}

void AnimManager::refreshSmkIcon(int StartIcon, int num) {
	_curAnimFrame[kSmackerIcon]++;

	_curSmackBuffer = kSmackerIcon;

	if (_smkAnims[_curSmackBuffer] == nullptr)
		return;

	int stx = ICONMARGSX;
	int32 a;
	for (a = 0; a < ICONSHOWN; a++) {
		if (_vm->_inventory[a + StartIcon] == (num - aiBANCONOTE + 1)) {
			stx = a * ICONDX + ICONMARGSX;
			break;
		}
	}
	if (a == ICONSHOWN)
		return;

	refreshPalette(kSmackerIcon);

	Common::Rect dirtyRect = Common::Rect(0, 0, _smkAnims[kSmackerIcon]->getWidth(), _smkAnims[kSmackerIcon]->getHeight());
	for (a = 0; a < ICONDY - dirtyRect.top; a++) {
		byte2word(
			_vm->_screenBuffer + dirtyRect.left + stx + (dirtyRect.top + a + FIRSTLINE) * SCREENLEN,
			_smkBuffer[kSmackerIcon] + dirtyRect.left + (dirtyRect.top + a) * _smkAnims[kSmackerIcon]->getWidth(),
			_smkPal[kSmackerIcon], dirtyRect.width());

#if USE_DIRTY_RECTS
		AddLine(lastRect->left + stx, lastRect->right + stx, lastRect->top + a + FIRSTLINE);
#endif
	}

	smkNextFrame();
}

#if (!USE_NEW_VIDEO_CODE)
void AnimManager::refreshFullMotion() {
	int32 yfact;

	_curSmackBuffer = kSmackerFullMotion;

	if (((_curAnimFrame[kSmackerFullMotion] + 1) >= _fullMotionStart) && (_curAnimFrame[kSmackerFullMotion] + 1 <= _fullMotionEnd)) {
		_curAnimFrame[kSmackerFullMotion]++;

		refreshPalette(kSmackerFullMotion);

		DialogHandler(_curAnimFrame[kSmackerFullMotion]);

		_vm->_sdText.dx = TextLength(_vm->_sdText.text, 0);

		_vm->_sdText.x = 20;
		_vm->_sdText.y = 380;
		_vm->_sdText.dx = MAXX - 40;
		_vm->_sdText.dy = _vm->_sdText.checkDText();
		_vm->_sdText._subtitleRect.left = 0;
		_vm->_sdText._subtitleRect.top = 0;
		_vm->_sdText._subtitleRect.right = MAXX;
		_vm->_sdText._subtitleRect.bottom = MAXY;
		_vm->_sdText.scol = MASKCOL;

		// If text was displayed, remove it
		if (_vm->_oldSdText.text != nullptr) {
			if ((_vm->_oldSdText.y < _vm->_sdText.y) || (_vm->_oldSdText.y + _vm->_oldSdText.dy > _vm->_sdText.y + _vm->_sdText.dy) || (_vm->_sdText.text == nullptr)) {
				drawSmkBuffer(0, _vm->_oldSdText.y - TOP, MAXX, _vm->_oldSdText.dy);
				_vm->_graphicsMgr->copyToScreen(0, _vm->_oldSdText.y, MAXX, _vm->_oldSdText.dy);
			}
			_vm->_oldSdText.text = nullptr;
		}
		// If there's text
		if (_vm->_sdText.text != nullptr) {
			drawSmkBuffer(0, _vm->_sdText.y - TOP, MAXX, _vm->_sdText.dy);
			// Write string
			if (ConfMan.getBool("subtitles"))
				_vm->_sdText.DText();
			// and show it
			_vm->_oldSdText.text = nullptr;
		}

		if (_smkAnims[kSmackerFullMotion]->getHeight() > MAXY / 2)
			yfact = 1;
		else
			yfact = 2;

		Common::Rect dirtyRect = Common::Rect(0, 0, _smkAnims[kSmackerFullMotion]->getWidth(), _smkAnims[kSmackerFullMotion]->getHeight());
		_vm->_graphicsMgr->lock();

		for (int32 a = 0; a < dirtyRect.height(); a++) {
			// if it's already copied
			if ((_vm->_sdText.text == nullptr) ||
				((dirtyRect.top + a) * yfact < (_vm->_sdText.y - TOP)) ||
				((dirtyRect.top + a) * yfact >= (_vm->_sdText.y + _vm->_sdText.dy - TOP))) {

				byte2word(
					_vm->_graphicsMgr->_screenPtr + dirtyRect.left + (dirtyRect.top + a) * MAXX + (MAXY - _smkAnims[kSmackerFullMotion]->getHeight()) / 2 * MAXX,
					_smkBuffer[kSmackerFullMotion] + dirtyRect.left + (dirtyRect.top + a) * _smkAnims[kSmackerFullMotion]->getWidth(),
					_smkPal[kSmackerFullMotion],
					dirtyRect.width()
				);
			}
		}

		_vm->_graphicsMgr->unlock();

		if (_vm->_sdText.text != nullptr)
			_vm->_graphicsMgr->copyToScreen(0, _vm->_sdText.y, MAXX, _vm->_sdText.dy);

		if (_curAnimFrame[kSmackerFullMotion] == _fullMotionEnd) {
			drawSmkBuffer(0, 0, MAXX, AREA);
			doEvent(MC_DIALOG, ME_ENDCHOICE, MP_HIGH, _curAnimFrame[kSmackerFullMotion], 0, 0, 0);
			smkSoundOnOff(kSmackerFullMotion, false);
		} else {
			smkNextFrame();

			if (_curAnimFrame[kSmackerFullMotion] >= _smkAnims[kSmackerFullMotion]->getFrameCount())
				stopFullMotion();
		}
	}
}

void AnimManager::playFullMotion(int start, int end) {
	_curSmackBuffer = kSmackerFullMotion;

	start = CLIP<int32>(start, 1, _smkAnims[kSmackerFullMotion]->getFrameCount() - 1);
	end = CLIP<int32>(end, 1, _smkAnims[kSmackerFullMotion]->getFrameCount());
	
	//	If choices are attached
	if (_curAnimFrame[kSmackerFullMotion] != (start - 1)) {
		for (int a = 0; a < MAXNEWSMKPAL; a++) {
			if ((_dialog[_curDialog]._newPal[a] > start || !_dialog[_curDialog]._newPal[a]) && a) {
				smkGoto(kSmackerFullMotion, _dialog[_curDialog]._newPal[a - 1]);
				refreshPalette(kSmackerFullMotion);
				break;
			}

			if (_dialog[_curDialog]._newPal[a] == 0 || _dialog[_curDialog]._newPal[a] == start)
				break;
		}

		if (end - start > 2) {
			if (start > 10)
				smkGoto(kSmackerFullMotion, start - 10);
			else
				smkGoto(kSmackerFullMotion, 1);

			while (_smkAnims[kSmackerFullMotion]->getCurFrame() < start - 1) {
				smkNextFrame();
			}
		} else
			smkGoto(kSmackerFullMotion, start);

		_curAnimFrame[kSmackerFullMotion] = start - 1;
	}
	if ((end - start) > 2)
		smkSoundOnOff(kSmackerFullMotion, true);

	_fullMotionStart = start;
	_fullMotionEnd = end;

	_vm->_sdText.clear();
	_vm->_oldSdText.clear();
}

void AnimManager::drawSmkBuffer(int px, int py, int dx, int dy) {
	for (int a = 0; a < dy; a++) {
		byte2word(
			_vm->_screenBuffer + (a + py + TOP) * MAXX + px,
			_smkBuffer[kSmackerFullMotion] + (a + py) * _smkAnims[kSmackerFullMotion]->getWidth() + px,
			_smkPal[kSmackerFullMotion],
			dx
		);
	}
}
#endif

void AnimManager::swapCD(int cd) {
	Common::String animFileName = Common::String::format("nlanim.cd%d", cd);
	for (int i = 0; i < MAXSMACK; i++) {
		_animFile[i].close();
		_animFile[i].open(animFileName);
	}
}

} // namespace Trecision
