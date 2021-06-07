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
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "trecision/actor.h"
#include "trecision/anim.h"
#include "trecision/defines.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

bool NightlongSmackerDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!SmackerDecoder::loadStream(stream))
		return false;

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

void NightlongSmackerDecoder::muteTrack(uint track, bool mute) {
	Track *t = getTrack(track);
	if (t && t->getTrackType() == Track::kTrackTypeAudio) {
		((AudioTrack *)t)->setMute(mute);
	}
}

void NightlongSmackerDecoder::setMute(bool mute) {
	for (TrackList::iterator it = getTrackListBegin(); it != getTrackListEnd(); ++it) {
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setMute(mute);
	}
}

bool NightlongSmackerDecoder::forceSeekToFrame(uint frame) {
	const uint seekFrame = MAX<uint>(frame - 10, 0);

	if (!isVideoLoaded())
		return true;

	if (seekFrame >= getFrameCount())
		return false;

	if (!rewind())
		return false;

	stopAudio();
	SmackerVideoTrack *videoTrack = (SmackerVideoTrack *)getTrack(0);
	uint32 start = _fileStream->pos();
	uint32 offset = 0;
	for (uint32 i = 0; i < seekFrame; i++) {
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

	if (!_fileStream->seek(start + offset, SEEK_SET))
		return false;

	while (getCurFrame() < (int)frame) {
		decodeNextFrame();
	}

	_lastTimeChange = videoTrack->getFrameTime(frame);
	_startTime = g_system->getMillis() - (_lastTimeChange.msecs() / getRate()).toInt();
	startAudio();

	return true;
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

	_curCD = 1;
	swapCD(_curCD);

	_bgAnimRestarted = false;
}

AnimManager::~AnimManager() {
	for (int i = 0; i < MAXSMACK; ++i) {
		delete _smkAnims[i];
		_smkAnims[i] = nullptr;
		_animFile[i].close();
	}
}

void AnimManager::playMovie(const Common::String &filename, int startFrame, int endFrame) {
	NightlongSmackerDecoder *smkDecoder = new NightlongSmackerDecoder;

	if (!smkDecoder->loadFile(filename)) {
		warning("playMovie: File %s not found", filename.c_str());
		delete smkDecoder;
		_vm->_dialogMgr->afterChoice();
		return;
	}

	Common::Event event;
	bool skipVideo = false;
	uint16 x = (g_system->getWidth() - smkDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - smkDecoder->getHeight()) / 2;
	_vm->_drawText._text.clear();

	smkDecoder->start();

	setVideoRange(smkDecoder, startFrame, endFrame);

	while (!_vm->shouldQuit() && startFrame != endFrame && !smkDecoder->endOfVideo() && !skipVideo) {
		if (smkDecoder->needsUpdate()) {
			drawFrame(smkDecoder, x, y, true);
		}

		while (_vm->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYUP && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	delete smkDecoder;

	_vm->_mouseLeftBtn = _vm->_mouseRightBtn = false;
	_vm->freeKey();
	_vm->_dialogMgr->afterChoice();
}

void AnimManager::setVideoRange(NightlongSmackerDecoder *smkDecoder, int &startFrame, int &endFrame) {
	// Trecision starts at 1 but ScummVM starts at 0
	startFrame = CLIP<int32>(startFrame - 1, 0, smkDecoder->getFrameCount() - 1);
	endFrame = CLIP<int32>(endFrame - 1, 0, smkDecoder->getFrameCount() - 1);

	// If choices are attached
	if (startFrame > 0 && startFrame > smkDecoder->getCurFrame()) {
		smkDecoder->forceSeekToFrame(startFrame - 1);
	}
	smkDecoder->setEndFrame(endFrame);
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
			_vm->_system->updateScreen();
	}
}

void AnimManager::drawFrameSubtitles(Graphics::Surface *surface, int frameNum) {
	if (!ConfMan.getBool("subtitles"))
		return;

	_vm->_dialogMgr->dialogHandler(frameNum);
	if (_vm->_drawText._text.empty())
		return;

	// Subtitles can be placed in different coordinates in the video,
	// which are set inside dialogHandler(), but are then reset to
	// fixed coordinates
	_vm->_drawText._rect = Common::Rect(20, 380 - TOP, MAXX - 40 + 20, _vm->_drawText.calcHeight(_vm) + (380 - TOP));
	_vm->_drawText._subtitleRect = Common::Rect(MAXX, MAXY);
	_vm->_drawText._shadowCol = MASKCOL;
	_vm->_drawText.draw(_vm, surface);
}

void AnimManager::openSmkAnim(int slot, const Common::String &name) {
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

void AnimManager::smkToggleTrackAudio(int slot, int track, bool on) {
	if (_smkAnims[slot] == nullptr)
		return;

	_smkAnims[slot]->muteTrack(track, !on);
}

void AnimManager::smkToggleAudio(int slot, bool on) {
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
	uint16 animFlag = _animTab[animation]._flag;

	// choose the buffer to use
	if (animFlag & SMKANIM_BKG)
		slot = kSmackerBackground;
	else if (animFlag & SMKANIM_ICON)
		slot = kSmackerIcon;
	else
		slot = kSmackerAction;

	smkStop(slot);

	_playingAnims[slot] = animation;

	// choose how to open
	if (slot == kSmackerBackground) {
		openSmkAnim(kSmackerBackground, _animTab[animation]._name);
		_bgAnimRestarted = false;

		toggleMuteBgAnim(animation);
	} else if (slot == kSmackerIcon) {
		openSmkAnim(kSmackerIcon, _animTab[animation]._name);
	} else {
		uint32 st = _vm->readTime();
		openSmkAnim(kSmackerAction, _animTab[animation]._name);
		_vm->_nextRefresh += _vm->readTime() - st; // fixup opening time
	}
}

void AnimManager::toggleMuteBgAnim(uint16 animation) {
	uint16 animFlag = _animTab[animation]._flag;
	NightlongSmackerDecoder *decoder = _smkAnims[kSmackerBackground];
	if (decoder == nullptr)
		return;

	// Turns off when not needed
	if (animation == aBKG11 && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG14 && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG1C && _vm->_obj[oFAX17].isFlagExtra()) {
		_animTab[animation]._flag |= SMKANIM_OFF1;
		decoder->muteTrack(1, true);
	} else if (animation == aBKG1D && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG22 && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG48 && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG4P && (animFlag & SMKANIM_OFF1))
		decoder->muteTrack(1, true);
	else if (animation == aBKG28 && (animFlag & SMKANIM_OFF4))
		decoder->muteTrack(1, true);
	else if (animation == aBKG37 && !_vm->_room[_vm->_curRoom].hasExtra())
		decoder->muteTrack(1, true);
	else if (animation == aBKG2E && (animFlag & SMKANIM_OFF2))
		decoder->muteTrack(2, true);
	else if (animation == aBKG2G && _vm->_dialogMgr->isDialogFinished(556))
		decoder->muteTrack(2, true);
	else if (animation == aBKG34 &&                                   // If it's BKG 34 and
			 (_vm->_dialogMgr->isDialogFinished(616) || // if the FMV is already done or
			  _vm->isObjectVisible(oTUBOT34) ||                       // if the whole tube is available or
			  _vm->isObjectVisible(oTUBOFT34) ||                      // if the outside of the tube is available or
			  _vm->isObjectVisible(oVALVOLAC34)))                     // if the valve is closed
		decoder->muteTrack(2, true);
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

void AnimManager::startFullMotion() {
	stopAllSmkAnims();

	_vm->_flagDialogActive = true;
	_vm->_flagShowCharacter = false;

	_vm->_textStatus = TEXT_OFF;
	_vm->_inventoryStatus = INV_OFF;
	_vm->_inventoryCounter = INVENTORY_HIDE;

	_vm->_textMgr->clearTextStack();
	_vm->_graphicsMgr->clearScreen();

	_vm->_scheduler->resetQueues();
	_vm->_actor->actorStop();
	_vm->_graphicsMgr->hideCursor();
}

void AnimManager::stopFullMotion() {
	const uint16 curDialog = _vm->_dialogMgr->getCurDialog();

	_vm->_flagDialogActive = false;
	_vm->_flagDialogMenuActive = false;
	_vm->_flagSomeoneSpeaks = false;
	_vm->_lightIcon = 0xFF;
	_vm->_graphicsMgr->showCursor();

	if (curDialog == dFCRED) {
		_vm->quitGame();
		return;
	}

	if (!((curDialog == dSHOPKEEPER1A) && _vm->_dialogMgr->getCurChoice() == 185)) {
		if ((curDialog == dF582) || (curDialog == dFLOG) || (curDialog == dINTRO) || (curDialog == dF362) || (curDialog == dC381) || (curDialog == dF381) ||
		    (curDialog == dF491) || ((curDialog == dC581) && !_vm->_dialogMgr->isDialogFinished(886) && _vm->_dialogMgr->isDialogFinished(258)) ||
		    ((curDialog == dC5A1) && _vm->_room[kRoom5A].hasExtra()))
			_vm->_flagShowCharacter = false;
		else
			_vm->redrawRoom();

		if (curDialog == dF582)
			_vm->_soundMgr->stopAllExceptMusic();
	}
}

void AnimManager::refreshAnim(int box) {
	for (int i = 0; i < MAXSMACK; i++) {
		if (_playingAnims[i] != 0 && box == BOX_BACKGROUND && i != kSmackerAction) {
			refreshSmkAnim(_playingAnims[i]);
		}
	}
}

void AnimManager::refreshSmkAnim(uint16 animation) {
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
			_vm->_graphicsMgr->addDirtyRect(_animTab[animation]._lim[a], true);
		}
	}
}

void AnimManager::handleEndOfVideo(int animation, int slot) {
	if (_smkAnims[slot] == nullptr) {
		smkStop(slot);
		return;
	}
	if (!_smkAnims[slot]->endOfVideo())
		return;
	
	if (!(_animTab[animation]._flag & SMKANIM_LOOP) && !(_animTab[animation]._flag & SMKANIM_BKG)) {
		smkStop(slot);
		_vm->_flagPaintCharacter = true;
	} else {
		_smkAnims[slot]->rewind();
		_vm->_animTypeMgr->init(animation, 0);
		_bgAnimRestarted = true;
	}
}

void AnimManager::drawSmkBackgroundFrame(int animation) {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerBackground];
	if (smkDecoder == nullptr)
		return;
	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	if (!frame)
		return;

	const Common::Rect *lastRect = smkDecoder->getNextDirtyRect();
	const byte *palette = smkDecoder->getPalette();

	if (smkDecoder->getCurFrame() == 0 && !_bgAnimRestarted) {
		_vm->_graphicsMgr->blitToScreenBuffer(frame, 0, TOP, palette, true);
	} else {
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

			if (smkDecoder->getCurFrame() > 0 && !intersects) {
				Graphics::Surface anim = frame->getSubArea(*lastRect);
				_vm->_graphicsMgr->blitToScreenBuffer(&anim, lastRect->left, lastRect->top + TOP, palette, true);
			}

			lastRect = smkDecoder->getNextDirtyRect();
		}
	}
}

void AnimManager::drawSmkIconFrame(uint16 startIcon, uint16 iconNum) {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerIcon];
	if (smkDecoder == nullptr)
		return;

	int stx = ICONMARGSX;
	uint a;
	for (a = 0; a < ICONSHOWN; ++a) {
		if (a + startIcon >= _vm->_inventory.size())
			break;

		if (_vm->_inventory[a + startIcon] == iconNum - FIRST_INV_ITEM + 1) {
			stx = a * ICONDX + ICONMARGSX;
			break;
		}
	}

	if (a == ICONSHOWN)
		return;

	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	_vm->_graphicsMgr->copyToScreenBuffer(frame, stx, FIRSTLINE, smkDecoder->getPalette());

	if (smkDecoder->endOfVideo())
		smkDecoder->rewind();
}

void AnimManager::drawSmkActionFrame() {
	NightlongSmackerDecoder *smkDecoder = _smkAnims[kSmackerAction];
	if (smkDecoder == nullptr)
		return;
	const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
	if (!frame)
		return;

	const byte *palette = smkDecoder->getPalette();

	if (smkDecoder->getCurFrame() == 0) {
		for (uint16 curY = 0; curY < AREA; curY++) {
			for (uint16 curX = 0; curX < MAXX; curX++) {
				if (frame->getPixel(curX, curY)) {
					_animRect.left = MIN<uint16>(curX, _animRect.left);
					_animRect.top = MIN<uint16>(curY, _animRect.top);
					_animRect.right = MAX<uint16>(curX, _animRect.right);
					_animRect.bottom = MAX<uint16>(curY, _animRect.bottom);
				}
			}
		}
	}

	if (_animRect.width() > 0 && _animRect.height() > 0) {
		Graphics::Surface anim = frame->getSubArea(_animRect);
		_vm->_graphicsMgr->blitToScreenBuffer(&anim, _animRect.left, _animRect.top + TOP, palette, false);
		_vm->_graphicsMgr->addDirtyRect(_animRect, true, true);
	}
}

void AnimManager::swapCD(int cd) {
	Common::String animFileName = Common::String::format("nlanim.cd%d", cd);
	for (uint8 i = 0; i < MAXSMACK; ++i) {
		_animFile[i].close();
		_animFile[i].open(_vm, animFileName);
	}
}

void AnimManager::syncGameStream(Common::Serializer &ser) {
	for (int a = 0; a < MAXANIM; a++) {
		SAnim *cur = &_animTab[a];
		ser.syncBytes((byte *)cur->_name, 14);
		ser.syncAsUint16LE(cur->_flag);
		for (uint8 i = 0; i < MAXCHILD; ++i) {
			ser.syncAsUint16LE(cur->_lim[i].left);
			ser.syncAsUint16LE(cur->_lim[i].top);
			ser.syncAsUint16LE(cur->_lim[i].right);
			ser.syncAsUint16LE(cur->_lim[i].bottom);
		}
		ser.syncAsByte(cur->_nbox);
		ser.skip(1, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX);
		for (uint8 i = 0; i < MAXATFRAME; ++i) {
			ser.syncAsByte(cur->_atFrame[i]._type);
			ser.syncAsByte(cur->_atFrame[i]._child);
			ser.syncAsUint16LE(cur->_atFrame[i]._numFrame);
			ser.syncAsUint16LE(cur->_atFrame[i]._index);
		}
	}
}

void AnimManager::loadAnimTab(Common::SeekableReadStreamEndian *stream) {
	for (uint16 i = 0; i < MAXANIM; ++i) {
		stream->read(&_animTab[i]._name, 14);

		_animTab[i]._flag = stream->readUint16();

		for (uint8 j = 0; j < MAXCHILD; ++j) {
			_animTab[i]._lim[j].left = stream->readUint16();
			_animTab[i]._lim[j].top = stream->readUint16();
			_animTab[i]._lim[j].right = stream->readUint16();
			_animTab[i]._lim[j].bottom = stream->readUint16();
		}

		_animTab[i]._nbox = stream->readByte();
		stream->readByte(); // Padding

		for (uint8 j = 0; j < MAXATFRAME; ++j) {
			_animTab[i]._atFrame[j]._type = stream->readByte();
			_animTab[i]._atFrame[j]._child = stream->readByte();
			_animTab[i]._atFrame[j]._numFrame = stream->readUint16();
			_animTab[i]._atFrame[j]._index = stream->readUint16();
		}
	}
}

} // namespace Trecision
