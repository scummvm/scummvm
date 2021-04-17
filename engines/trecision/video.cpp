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

#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

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

	FreeKey();
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
				smkDecoder->forceSeekToFrame(_dialog[_curDialog]._newPal[a - 1] - 1);
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

void AnimManager::openSmkAnim(int buf, Common::String name) {
	//debug("Opening anim %s", name.c_str());
	if (_animFile[buf].hasFile(name)) {
		openSmk(buf, _animFile[buf].createReadStreamForMember(name));
		return;
	}

	_curCD = _curCD == 1 ? 2 : 1;
	swapCD(_curCD);

	if (_animFile[buf].hasFile(name)) {
		openSmk(buf, _animFile[buf].createReadStreamForMember(name));
		return;
	}

	// Invalid file
	error("openSmkAnim(): File %s not found", name.c_str());
}

void AnimManager::openSmk(int buf, Common::SeekableReadStream *stream) {
	_smkAnims[buf] = new NightlongSmackerDecoder();

	if (!_smkAnims[buf]->loadStream(stream)) {
		warning("Invalid SMK file");
		closeSmk(buf);
	} else {
		_smkAnims[buf]->start();
		smkNextFrame(buf);
	}
}

void AnimManager::closeSmk(int buf) {
	delete _smkAnims[buf];
	_smkAnims[buf] = nullptr;
}

void AnimManager::smkNextFrame(int buf) {
	if (_smkAnims[buf] == nullptr)
		return;

	// Loop
	if (_smkAnims[buf]->getCurFrame() >= (int)_smkAnims[buf]->getFrameCount() - 1) {
		_smkAnims[buf]->rewind();
		_smkAnims[buf]->decodeNextFrame(); // Skip frame 0
	}

	const Graphics::Surface *surface = _smkAnims[buf]->decodeNextFrame();
	_smkBuffer[buf] = (uint8 *)surface->getPixels();
}

void AnimManager::smkGoto(int buf, int num) {
	if (_smkAnims[buf] == nullptr)
		return;

	_smkAnims[buf]->forceSeekToFrame(num);
}

void AnimManager::smkVolumePan(int buf, int track, int vol) {
	if (_smkAnims[buf] == nullptr)
		return;

	_smkAnims[buf]->muteTrack(track, vol == 0);
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

	if (_playingAnims[pos] != 0) {
		stopSmkAnim(_playingAnims[pos]);
	}

	_playingAnims[pos] = num;
	_curAnimFrame[pos] = 0;

	// choose how to open
	if (_animTab[num]._flag & SMKANIM_BKG) {
		openSmkAnim(kSmackerBackground, _animTab[num]._name);

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
		openSmkAnim(kSmackerIcon, _animTab[num]._name);
	} else {
		uint32 st = ReadTime();

		openSmkAnim(kSmackerAction, _animTab[num]._name);
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

	closeSmk(pos);

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
	g_vm->hideCursor();
}

void AnimManager::stopFullMotion() {
	g_vm->_flagDialogActive = false;
	g_vm->_flagDialogMenuActive = false;
	g_vm->showCursor();
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
			if (a != 1) {
				refreshSmkAnim(_playingAnims[a]);
			}
		}
	}
}

void AnimManager::refreshAllAnimations() {
	soundtimefunct();
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
	if (num == 0)
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
		else
			pos = kSmackerAction;
	}

	if (_smkAnims[pos] == nullptr)
		return;

	_curAnimFrame[pos]++;

	refreshPalette(pos);

	while (const Common::Rect *lastRect = _smkAnims[pos]->getNextDirtyRect()) {
		bool intersects = false;
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (_animTab[num]._flag & (SMKANIM_OFF1 << a)) {
				Common::Rect lr = *lastRect;
				lr.bottom++;
				lr.right++;

				if (_animTab[num]._lim[a].intersects(lr)) {
					intersects = true;
					break;
				}
			}
		}

		if (_curAnimFrame[pos] > 0 && !intersects) {
			if (pos == kSmackerBackground) {
				for (int32 a = 0; a < lastRect->height(); a++) {
					byte2wordn(
						_vm->_screenBuffer + lastRect->left + (lastRect->top + a + TOP) * MAXX,
						_smkBuffer[pos] + lastRect->left + (lastRect->top + a) * _smkAnims[pos]->getWidth(),
						_vm->_smackImageBuffer + lastRect->left + (lastRect->top + a) * MAXX,
						_smkPal[pos],
						lastRect->width()
					);

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
		if (_curAnimFrame[kSmackerAction] == 1) {
			for (uint16 b = 0; b < AREA; b++) {
				for (uint16 a = 0; a < MAXX; a++) {
					if (_smkBuffer[kSmackerAction][b * MAXX + a]) {
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
				_smkBuffer[kSmackerAction] + _animMinX + (_animMinY + a) * _smkAnims[kSmackerAction]->getWidth(),
				_smkPal[kSmackerAction],
				_animMaxX - _animMinX
			);
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
			smkNextFrame(pos);

	} else
		smkNextFrame(pos);

	if ((_smkAnims[pos] != nullptr) && (_curAnimFrame[pos] >= _smkAnims[pos]->getFrameCount())) {
		if ((_animTab[num]._flag & SMKANIM_LOOP) || (_animTab[num]._flag & SMKANIM_BKG))
			InitAtFrameHandler(num, 0);

		_curAnimFrame[pos] = 0;
	}
}

void AnimManager::refreshSmkIcon(int StartIcon, int num) {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerIcon];

	//_curAnimFrame[kSmackerIcon]++;	// not needed

	if (smkDecoder == nullptr)
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

	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	Graphics::Surface *frame16 = frame->convertTo(g_system->getScreenFormat(), smkDecoder->getPalette());
	g_vm->_graphicsMgr->copyToScreenBuffer(frame16, stx, FIRSTLINE);
	delete frame16;

	if (smkDecoder->endOfVideo()) {
		smkDecoder->rewind();
	}
}

void AnimManager::swapCD(int cd) {
	Common::String animFileName = Common::String::format("nlanim.cd%d", cd);
	for (int i = 0; i < MAXSMACK; i++) {
		_animFile[i].close();
		_animFile[i].open(animFileName);
	}
}

} // namespace Trecision
