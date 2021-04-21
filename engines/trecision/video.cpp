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

#include "sound.h"

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
		_smkAnims[i] = nullptr;
		_playingAnims[i] = 0;
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

	if (!smkDecoder->loadFile(filename)) {
		warning("playMovie: File %s not found", filename.c_str());
		doEvent(MC_DIALOG, ME_ENDCHOICE, MP_HIGH, smkDecoder->getCurFrame(), 0, 0, 0);
		return;
	}

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

	g_vm->FreeKey();
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
		frame16->free();
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

void AnimManager::openSmkAnim(int slot, Common::String name) {
	//debug("Opening anim %s", name.c_str());
	for (int i = 0; i < 3; i++) {
		// Open the animation, or swap the 3 CDs to find it
		if (_animFile[slot].hasFile(name)) {
			openSmk(slot, _animFile[slot].createReadStreamForMember(name));
			return;
		}

		_curCD = _curCD < 3 ? _curCD + 1 : 1;
		swapCD(_curCD);
	}

	error("openSmkAnim(): File %s not found", name.c_str());
}

void AnimManager::openSmk(int slot, Common::SeekableReadStream *stream) {
	_smkAnims[slot] = new NightlongSmackerDecoder();

	if (!_smkAnims[slot]->loadStream(stream)) {
		warning("Invalid SMK file");
		closeSmk(slot);
	} else {
		_smkAnims[slot]->start();
	}
}

void AnimManager::closeSmk(int slot) {
	delete _smkAnims[slot];
	_smkAnims[slot] = nullptr;
}

void AnimManager::smkGoto(int slot, int frame) {
	if (_smkAnims[slot] == nullptr)
		return;

	_smkAnims[slot]->forceSeekToFrame(frame);
}

void AnimManager::smkVolumePan(int slot, int track, int vol) {
	if (_smkAnims[slot] == nullptr)
		return;

	_smkAnims[slot]->muteTrack(track, vol == 0);
}

void AnimManager::smkSoundOnOff(int slot, bool on) {
	if (_smkAnims[slot] == nullptr)
		return;

	_smkAnims[slot]->setMute(!on);
}

int16 AnimManager::smkCurFrame(int slot) {
	if (_smkAnims[slot] == nullptr)
		return -1;

	return _smkAnims[slot]->getCurFrame();
}

void AnimManager::startSmkAnim(uint16 animation) {
	int slot;

	// choose the buffer to use
	if (_animTab[animation]._flag & SMKANIM_BKG)
		slot = kSmackerBackground;
	else if (_animTab[animation]._flag & SMKANIM_ICON)
		slot = kSmackerIcon;
	else
		slot = kSmackerAction;

	if (_playingAnims[slot] != 0) {
		smkStop(slot);
	}

	_playingAnims[slot] = animation;

	// choose how to open
	if (_animTab[animation]._flag & SMKANIM_BKG) {
		openSmkAnim(kSmackerBackground, _animTab[animation]._name);

		// Turns off when not needed
		if ((animation == aBKG11) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG14) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG1C) && (_vm->_obj[oFAX17]._flag & kObjFlagExtra)) {
			_animTab[animation]._flag |= SMKANIM_OFF1;
			smkVolumePan(0, 1, 0);
		} else if ((animation == aBKG1D) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG22) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG48) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG4P) && (_animTab[animation]._flag & SMKANIM_OFF1))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG28) && (_animTab[animation]._flag & SMKANIM_OFF4))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG37) && (!(_vm->_room[_vm->_curRoom]._flag & kObjFlagExtra)))
			smkVolumePan(0, 1, 0);
		else if ((animation == aBKG2E) && (_animTab[animation]._flag & SMKANIM_OFF2))
			smkVolumePan(0, 2, 0);
		else if ((animation == aBKG2G) && (g_vm->_choice[556]._flag & kObjFlagDone))
			smkVolumePan(0, 2, 0);
		else if ((animation == aBKG34) &&                                     // If it's BKG 34 and
		         ((g_vm->_choice[616]._flag & kObjFlagDone) ||          // if the FMV is already done or
		          (_vm->_obj[oTUBOT34]._mode & OBJMODE_OBJSTATUS) ||    // if the whole tube is available or
		          (_vm->_obj[oTUBOFT34]._mode & OBJMODE_OBJSTATUS) ||   // if the outside of the tube is available or
		          (_vm->_obj[oVALVOLAC34]._mode & OBJMODE_OBJSTATUS)))  // if the valve is closed
			smkVolumePan(0, 2, 0);
	} else if (_animTab[animation]._flag & SMKANIM_ICON) {
		openSmkAnim(kSmackerIcon, _animTab[animation]._name);
	} else {
		uint32 st = _vm->ReadTime();

		openSmkAnim(kSmackerAction, _animTab[animation]._name);
		_vm->_nextRefresh += _vm->ReadTime() - st; // fixup opening time
	}
}

void AnimManager::smkStop(uint16 slot) {
	_playingAnims[slot] = 0;

	closeSmk(slot);

	_vm->_lightIcon = 0xFF;
}

void AnimManager::stopAllSmkAnims() {
	for (int slot = 0; slot < MAXSMACK; slot++) {
		if (_playingAnims[slot])
			smkStop(slot);
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
		    (_curDialog == dF491) || ((_curDialog == dC581) && !(g_vm->_choice[886]._flag & kObjFlagDone) && (g_vm->_choice[258]._flag & kObjFlagDone)) ||
		    ((_curDialog == dC5A1) && (_vm->_room[kRoom5A]._flag & kObjFlagExtra)))
			g_vm->_flagShowCharacter = false;
		else
			RedrawRoom();

		if (_curDialog == dF582)
			_vm->_soundMgr->fadeOut();
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
	_vm->_soundMgr->soundTimer();
}

void AnimManager::refreshSmkAnim(int animation) {
	if (animation == 0)
		return;

	if (_animTab[animation]._flag & SMKANIM_ICON) {
		drawSmkIconFrame(_vm->_inventoryRefreshStartIcon, animation);
	} else if (_animTab[animation]._flag & SMKANIM_BKG) {
		drawSmkBackgroundFrame(animation);
		handleEndOfVideo(animation, kSmackerBackground);
	} else {
		drawSmkActionFrame();
		handleEndOfVideo(animation, kSmackerAction);
	}
	
	for (int32 a = 0; a < MAXCHILD; a++) {
		if (!(_animTab[animation]._flag & (SMKANIM_OFF1 << a)) && (_animTab[animation]._lim[a].bottom != 0)) {
			Common::Rect l = _animTab[animation]._lim[a];
			l.translate(0, TOP);
			_vm->_limits[_vm->_limitsNum++] = l;
		}
	}
}

void AnimManager::handleEndOfVideo(int animation, int slot) {
	if (_smkAnims[slot]->endOfVideo()) {
		if (!(_animTab[animation]._flag & SMKANIM_LOOP) && !(_animTab[animation]._flag & SMKANIM_BKG)) {
			smkStop(slot);
			g_vm->_flagPaintCharacter = true;
		} else {
			_smkAnims[slot]->rewind();
			InitAtFrameHandler(animation, 0);
		}
	}
}

void AnimManager::drawSmkBackgroundFrame(int animation) {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerBackground];
	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	if (!frame)
		return;

	Graphics::Surface *frame16 = frame->convertTo(g_system->getScreenFormat(), smkDecoder->getPalette());
	const Common::Rect *lastRect = smkDecoder->getNextDirtyRect();
	const byte *pal = smkDecoder->getPalette();
	uint16 mask = g_vm->_graphicsMgr->palTo16bit(pal[0], pal[1], pal[2]);

	if (smkDecoder->getCurFrame() == 0)
		_vm->_graphicsMgr->setSmkBackground();

	while (lastRect) {
		bool intersects = false;
		for (int32 a = 0; a < MAXCHILD; a++) {
			if (_animTab[animation]._flag & (SMKANIM_OFF1 << a)) {
				if (_animTab[animation]._lim[a].intersects(*lastRect)) {
					intersects = true;
					break;
				}
			}
		}

		if (smkCurFrame(kSmackerBackground) > 0 && !intersects) {
			Graphics::Surface anim = frame16->getSubArea(*lastRect);
			g_vm->_graphicsMgr->blitToScreenBuffer(&anim, lastRect->left, lastRect->top + TOP, mask, true);
		}

		lastRect = _smkAnims[kSmackerBackground]->getNextDirtyRect();
	}

	frame16->free();
	delete frame16;
}

void AnimManager::drawSmkIconFrame(int startIcon, int iconNum) {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerIcon];
	if (smkDecoder == nullptr)
		return;

	int stx = ICONMARGSX;
	int32 a;
	for (a = 0; a < ICONSHOWN; a++) {
		if (_vm->_inventory[a + startIcon] == (iconNum - FIRST_INV_ITEM + 1)) {
			stx = a * ICONDX + ICONMARGSX;
			break;
		}
	}
	if (a == ICONSHOWN)
		return;

	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	Graphics::Surface *frame16 = frame->convertTo(g_system->getScreenFormat(), smkDecoder->getPalette());
	g_vm->_graphicsMgr->copyToScreenBuffer(frame16, stx, FIRSTLINE);
	frame16->free();
	delete frame16;

	if (smkDecoder->endOfVideo()) {
		smkDecoder->rewind();
	}
}

void AnimManager::drawSmkActionFrame() {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerAction];
	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	if (!frame)
		return;

	const byte *pal = smkDecoder->getPalette();
	uint16 mask = g_vm->_graphicsMgr->palTo16bit(pal[0], pal[1], pal[2]);

	if (smkCurFrame(kSmackerAction) == 0) {
		for (uint16 curY = 0; curY < AREA; curY++) {
			for (uint16 curX = 0; curX < MAXX; curX++) {
				if (frame->getPixel(curX, curY)) {
					_animMinX = MIN(curX, _animMinX);
					_animMinY = MIN(curY, _animMinY);
					_animMaxX = MAX(curX, _animMaxX);
					_animMaxY = MAX(curY, _animMaxY);
				}
			}
		}
	}

	if (_animMaxX > _animMinX && _animMaxY > _animMinY) {
		Graphics::Surface *frame16 = frame->convertTo(g_system->getScreenFormat(), smkDecoder->getPalette());
		const Common::Rect animRect(_animMinX, _animMinY, _animMaxX, _animMaxY);
		Graphics::Surface anim = frame16->getSubArea(animRect);
		g_vm->_graphicsMgr->blitToScreenBuffer(&anim, _animMinX, _animMinY + TOP, mask, false);
		frame16->free();
		delete frame16;

		_vm->_actorLimit = _vm->_limitsNum;
		_vm->_limits[_vm->_limitsNum++] = Common::Rect(_animMinX, _animMinY + TOP, _animMaxX, _animMaxY + TOP);
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
