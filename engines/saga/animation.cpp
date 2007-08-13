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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Background animation management module

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/console.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/render.h"
#include "saga/rscfile.h"
#include "saga/scene.h"

#include "saga/animation.h"

namespace Saga {

Anim::Anim(SagaEngine *vm) : _vm(vm) {
	uint16 i;

	_cutawayList = NULL;
	_cutawayListLength = 0;
	_cutawayActive = false;

	for (i = 0; i < MAX_ANIMATIONS; i++)
		_animations[i] = NULL;

	for (i = 0; i < ARRAYSIZE(_cutawayAnimations); i++)
		_cutawayAnimations[i] = NULL;
}

Anim::~Anim(void) {
	reset();
}

void Anim::loadCutawayList(const byte *resourcePointer, size_t resourceLength) {
	free(_cutawayList);
	_cutawayListLength = resourceLength / 8;
	_cutawayList = (Cutaway *)malloc(_cutawayListLength * sizeof(Cutaway));

	MemoryReadStream cutawayS(resourcePointer, resourceLength);

	for (int i = 0; i < _cutawayListLength; i++) {
		_cutawayList[i].backgroundResourceId = cutawayS.readUint16LE();
		_cutawayList[i].animResourceId = cutawayS.readUint16LE();
		_cutawayList[i].cycles = cutawayS.readSint16LE();
		_cutawayList[i].frameRate = cutawayS.readSint16LE();
	}
}

void Anim::freeCutawayList(void) {
	free(_cutawayList);
	_cutawayList = NULL;
	_cutawayListLength = 0;
}

void Anim::playCutaway(int cut, bool fade) {
	debug(0, "playCutaway(%d, %d)", cut, fade);

	bool startImmediately = false;

	_cutAwayFade = fade;

	// Chained cutaway, clean up the previous cutaway
	if (_cutawayActive) {
		clearCutaway();

		// This is used because when AM is zapping the child's mother in Benny's chapter, 
		// there is a cutaway followed by a video. The video needs to start immediately after
		// the cutaway so that it looks like the original
		startImmediately = true;
	}

	// WORKAROUND: The IHNM demo deals with chained cutaways in a different manner. Don't save
	// the palette of cutaway 11 (the woman looking at the marble)
	if (!(_vm->getGameId() == GID_IHNM_DEMO && cut == 11))
		_vm->_gfx->savePalette();

	if (fade) {
		_vm->_gfx->getCurrentPal(saved_pal);
		// TODO
		/*
		Event event;
		static PalEntry cur_pal[PAL_ENTRIES];

		_vm->_gfx->getCurrentPal(cur_pal);

		event.type = kEvTImmediate;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = cur_pal;

		_vm->_events->queue(&event);
		*/
	}

	// Prepare cutaway
	_vm->_gfx->showCursor(false);
	_vm->_interface->setStatusText("");
	_vm->_interface->setSaveReminderState(0);
	_vm->_interface->rememberMode();
	if (_cutAwayMode == kPanelVideo)
		_vm->_interface->setMode(kPanelVideo);
	else
		_vm->_interface->setMode(kPanelCutaway);
	_cutawayActive = true;

	// Set the initial background and palette for the cutaway
	ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

	byte *resourceData;
	size_t resourceDataLength;

	_vm->_resource->loadResource(context, _cutawayList[cut].backgroundResourceId, resourceData, resourceDataLength);

	byte *buf;
	size_t buflen;
	int width;
	int height;

	_vm->decodeBGImage(resourceData, resourceDataLength, &buf, &buflen, &width, &height);

	const PalEntry *palette = (const PalEntry *)_vm->getImagePal(resourceData, resourceDataLength);

	Surface *bgSurface = _vm->_render->getBackGroundSurface();
	const Rect rect(width, height);

	bgSurface->blit(rect, buf);
	_vm->_frameCount++;

	_vm->_gfx->setPalette(palette);

	// WORKAROUND for a bug found in the original IHNM demo. The palette of cutaway 12 is incorrect (the incorrect
	// palette can be seen in the original demo too, for a split second). Therefore, use the saved palette for this
	// cutaway
	if (_vm->getGameId() == GID_IHNM_DEMO && cut == 12)
		_vm->_gfx->restorePalette();

	free(buf);
	free(resourceData);

	// Play the animation

	int cutawaySlot = -1;

	for (int i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
		if (!_cutawayAnimations[i]) {
			cutawaySlot = i;
		} else if (_cutawayAnimations[i]->state == ANIM_PAUSE) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
			cutawaySlot = i;
		} else if (_cutawayAnimations[i]->state == ANIM_PLAYING) {
			_cutawayAnimations[i]->state = ANIM_PAUSE;
		}
	}

	if (cutawaySlot == -1) {
		warning("Could not allocate cutaway animation slot");
		return;
	}
	
	// Some cutaways in IHNM have animResourceId equal to 0, which means that they only have
	// a background frame and no animation. Those animations are actually game scripts.
	// An example is the "nightfall" animation in Ben's chapter (fadein-fadeout), the animation
	// for the second from the left monitor in Ellen's chapter etc
	// Therefore, skip the animation bit if animResourceId is 0 and only show the background
	if (_cutawayList[cut].animResourceId == 0)
		return;

	_vm->_resource->loadResource(context, _cutawayList[cut].animResourceId, resourceData, resourceDataLength);

	load(MAX_ANIMATIONS + cutawaySlot, resourceData, resourceDataLength);

	free(resourceData);

	setCycles(MAX_ANIMATIONS + cutawaySlot, _cutawayList[cut].cycles);
	setFrameTime(MAX_ANIMATIONS + cutawaySlot, 1000 / _cutawayList[cut].frameRate);

	if (_cutAwayMode != kPanelVideo || startImmediately)
		play(MAX_ANIMATIONS + cutawaySlot, 0);
	else {
		Event event;
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = MAX_ANIMATIONS + cutawaySlot;
		event.time = (40 / 3) * 1000 / _cutawayList[cut].frameRate;

		_vm->_events->queue(&event);
	}
}

void Anim::endCutaway(void) {
	// I believe this is called by scripts after running one cutaway. At
	// this time, nothing needs to be done here.

	debug(0, "endCutaway()");
}

void Anim::returnFromCutaway(void) {
	// I believe this is called by scripts after running a cutaway to
	// ensure that we return to the scene as if nothing had happened. It's
	// not called by the IHNM intro, presumably because there is no old
	// scene to return to.

	debug(0, "returnFromCutaway()");


	if (_cutawayActive) {
		Event event;
		Event *q_event = NULL;

		if (_cutAwayFade) {
			static PalEntry cur_pal[PAL_ENTRIES];

			_vm->_gfx->getCurrentPal(cur_pal);

			event.type = kEvTImmediate;
			event.code = kPalEvent;
			event.op = kEventPalToBlack;
			event.time = 0;
			event.duration = kNormalFadeDuration;
			event.data = cur_pal;

			q_event = _vm->_events->queue(&event);
		}

		// Clear the cutaway. Note that this sets _cutawayActive to false
		event.type = kEvTImmediate;
		event.code = kCutawayEvent;
		event.op = kEventClear;
		event.time = 0;
		event.duration = 0;

		if (_cutAwayFade)
			q_event = _vm->_events->chain(q_event, &event);		// chain with the other events
		else
			q_event = _vm->_events->queue(&event);

		_vm->_scene->restoreScene();

		// Restore the animations
		event.type = kEvTImmediate;
		event.code = kAnimEvent;
		event.op = kEventResumeAll;
		event.time = 0;
		event.duration = 0;

		q_event = _vm->_events->chain(q_event, &event);		// chain with the other events

		// Draw the scene
		event.type = kEvTImmediate;
		event.code = kSceneEvent;
		event.op = kEventDraw;
		event.time = 0;
		event.duration = 0;

		q_event = _vm->_events->chain(q_event, &event);		// chain with the other events

		// Handle fade up, if we previously faded down
		if (_cutAwayFade) {
			event.type = kEvTImmediate;
			event.code = kPalEvent;
			event.op = kEventBlackToPal;
			event.time = 0;
			event.duration = kNormalFadeDuration;
			event.data = saved_pal;

			q_event = _vm->_events->chain(q_event, &event);

		}

		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventThreadWake;
		event.param = kWaitTypeWakeUp;

		q_event = _vm->_events->chain(q_event, &event);
	}
}

void Anim::clearCutaway(void) {
	debug(1, "clearCutaway()");
	if (_cutawayActive) {
		_cutawayActive = false;

		for (int i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
		}

		_vm->_interface->restoreMode();
		
		if (_vm->getGameId() != GID_IHNM_DEMO) {
			if (_vm->_scene->currentSceneNumber() >= 144 && _vm->_scene->currentSceneNumber() <= 149) {
				// Don't show the mouse cursor in the non-interactive part of the IHNM demo
			} else {
				_vm->_gfx->showCursor(true);
			}
		} else {
			// Enable the save reminder state after each cutaway for the IHNM demo
			_vm->_interface->setSaveReminderState(true);
		}
	}
}

void Anim::startVideo(int vid, bool fade) {
	debug(0, "startVideo(%d, %d)", vid, fade);

	_vm->_interface->setStatusText("");
	_vm->_frameCount = 0;

	playCutaway(vid, fade);
}

void Anim::endVideo(void) {
	debug(0, "endVideo()");

	clearCutaway();
}

void Anim::returnFromVideo(void) {
	debug(0, "returnFromVideo()");

	returnFromCutaway();
}

void Anim::load(uint16 animId, const byte *animResourceData, size_t animResourceLength) {
	AnimationData *anim;
	uint16 temp;

	if (animId >= MAX_ANIMATIONS) {
		if (animId >= MAX_ANIMATIONS + ARRAYSIZE(_cutawayAnimations))
			error("Anim::load could not find unused animation slot");
		anim = _cutawayAnimations[animId - MAX_ANIMATIONS] = new AnimationData(animResourceData, animResourceLength);
	} else
		anim = _animations[animId] = new AnimationData(animResourceData, animResourceLength);

	MemoryReadStreamEndian headerReadS(anim->resourceData, anim->resourceLength, _vm->isBigEndian());
	anim->magic = headerReadS.readUint16LE(); // cause ALWAYS LE
	anim->screenWidth = headerReadS.readUint16();
	anim->screenHeight = headerReadS.readUint16();

	anim->unknown06 = headerReadS.readByte();
	anim->unknown07 = headerReadS.readByte();
	anim->maxFrame = headerReadS.readByte() - 1;
	anim->loopFrame = headerReadS.readByte() - 1;
	temp = headerReadS.readUint16BE();
	anim->start = headerReadS.pos();
	if (temp == (uint16)(-1)) {
		temp = 0;
	}
	anim->start += temp;

	// Cache frame offsets
	anim->frameOffsets = (size_t *)malloc((anim->maxFrame + 1) * sizeof(*anim->frameOffsets));
	if (anim->frameOffsets == NULL) {
		memoryError("Anim::load");
	}

	fillFrameOffsets(anim);

	// Set animation data
	anim->currentFrame = 0;
	anim->completed = 0;
	anim->cycles = anim->maxFrame;

	anim->frameTime = DEFAULT_FRAME_TIME;
	anim->flags = ANIM_FLAG_NONE;
	anim->linkId = -1;
	anim->state = ANIM_PAUSE;
}

void Anim::link(int16 animId1, int16 animId2) {
	AnimationData *anim1;
	AnimationData *anim2;

	anim1 = getAnimation(animId1);

	anim1->linkId = animId2;

	if (animId2 == -1) {
		return;
	}

	anim2 = getAnimation(animId2);
	anim2->frameTime = anim1->frameTime;
}

void Anim::setCycles(uint16 animId, int cycles) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->cycles = cycles;
}

void Anim::play(uint16 animId, int vectorTime, bool playing) {
	Event event;
	Surface *backGroundSurface;

	byte *displayBuffer;

	uint16 frame;
	int frameTime;

	AnimationData *anim;
	AnimationData *linkAnim;

	if (animId > MAX_ANIMATIONS && !_cutawayActive)
		return;

	if (animId < MAX_ANIMATIONS && _cutawayActive)
		return;

	anim = getAnimation(animId);

	backGroundSurface = _vm->_render->getBackGroundSurface();
	displayBuffer = (byte*)backGroundSurface->pixels;

	if (playing) {
		anim->state = ANIM_PLAYING;
	}

	if (anim->state == ANIM_PAUSE) {
		return;
	}

	if (anim->completed < anim->cycles) {
		if (anim->currentFrame < 0)
			anim->currentFrame = 0;

		frame = anim->currentFrame;

		// WORKAROUND for a buggy animation in IHNM. Animation 0 in scene 67 (the mob of angry prisoners) should
		// start from frame 0, not frame 1. Frame 0 is the background of the animation (the mob of prisoners), whereas
		// the rest of the frames are their animated arms. Therefore, in order for the prisoners to appear correctly,
		// frame 0 should be displayed as the first frame, but anim->currentframe is set to 1, which means that the
		// prisoners will never be shown. In the original, the prisoners (first frame in the animation) are shown a
		// bit after the animation is started (which is wrong again, but not that apparent), whereas in ScummVM the
		// first frame is never shown. Therefore, make sure that for this animation, frame 0 is shown first
		if (_vm->getGameType() == GType_IHNM && _vm->_scene->currentChapterNumber() == 4 && 
			_vm->_scene->currentSceneNumber() == 67 && animId == 0 && anim->completed == 1)
			frame = 0;

		// FIXME: if start > 0, then this works incorrectly
		decodeFrame(anim, anim->frameOffsets[frame], displayBuffer, _vm->getDisplayWidth() * _vm->getDisplayHeight());
		_vm->_frameCount++;	
		anim->currentFrame++;
		if (anim->completed != 65535) {
			anim->completed++;
		}

		if (anim->currentFrame > anim->maxFrame) {

			anim->currentFrame = anim->loopFrame;
			_vm->_frameCount++;	

			if (anim->state == ANIM_STOPPING || anim->currentFrame == -1) {
				anim->state = ANIM_PAUSE;
			}
		}
	} else {
		_vm->_frameCount += 100;	// make sure the waiting thread stops waiting
		// Animation done playing
		anim->state = ANIM_PAUSE;
		if (anim->linkId == -1) {
			if (anim->flags & ANIM_FLAG_ENDSCENE) {
				// This animation ends the scene
				event.type = kEvTOneshot;
				event.code = kSceneEvent;
				event.op = kEventEnd;
				event.time = anim->frameTime + vectorTime;
				_vm->_events->queue(&event);
			}
			return;
		} else {
			anim->currentFrame = 0;
			anim->completed = 0;
		}
	}

	if (anim->state == ANIM_PAUSE && anim->linkId != -1) {
		// If this animation has a link, follow it
		linkAnim = getAnimation(anim->linkId);

		debug(5, "Animation ended going to %d", anim->linkId);
		linkAnim->state = ANIM_PLAYING;
		animId = anim->linkId;
		frameTime = 0;
	} else {
		frameTime = anim->frameTime + vectorTime;
	}

	event.type = kEvTOneshot;
	event.code = kAnimEvent;
	event.op = kEventFrame;
	event.param = animId;
	event.time = frameTime;

	_vm->_events->queue(&event);
}

void Anim::stop(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->state = ANIM_PAUSE;
}

void Anim::finish(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->state = ANIM_STOPPING;
}

void Anim::resume(uint16 animId, int cycles) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->cycles += cycles;
	play(animId, 0, true);
}

void Anim::reset() {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] != NULL) {
			delete _animations[i];
			_animations[i] = NULL;
		}
	}

	for (i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
		if (_cutawayAnimations[i] != NULL) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
		}
	}
}

void Anim::setFlag(uint16 animId, uint16 flag) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->flags |= flag;
}

void Anim::clearFlag(uint16 animId, uint16 flag) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->flags &= ~flag;
}

void Anim::setFrameTime(uint16 animId, int time) {
	AnimationData *anim;

	anim = getAnimation(animId);

	anim->frameTime = time;
}

int16 Anim::getCurrentFrame(uint16 animId) {
	AnimationData *anim;

	anim = getAnimation(animId);

	return anim->currentFrame;
}

void Anim::decodeFrame(AnimationData *anim, size_t frameOffset, byte *buf, size_t bufLength) {
	byte *writePointer = NULL;

	uint16 xStart = 0;
	uint16 yStart = 0;
	uint32 screenWidth;
	uint32 screenHeight;

	int markByte;
	byte dataByte;
	int newRow;

	uint16 controlChar;
	uint16 paramChar;

	uint16 runcount;
	int xVector;

	uint16 i;
	bool longData = isLongData();

	screenWidth = anim->screenWidth;
	screenHeight = anim->screenHeight;

	if ((screenWidth * screenHeight) > bufLength) {
		// Buffer argument is too small to hold decoded frame, abort.
		error("decodeFrame() Buffer size inadequate");
	}

	MemoryReadStream readS(anim->resourceData + frameOffset, anim->resourceLength - frameOffset);


#if 1
#define VALIDATE_WRITE_POINTER \
	if ((writePointer < buf) || (writePointer >= (buf + screenWidth * screenHeight))) { \
		error("VALIDATE_WRITE_POINTER: writePointer=%p buf=%p", (void *)writePointer, (void *)buf); \
	}
#else
#define VALIDATE_WRITE_POINTER
#endif


	// Begin RLE decompression to output buffer
	do {		
		markByte = readS.readByte();
		switch (markByte) {
		case SAGA_FRAME_START:
			xStart = readS.readUint16BE();
			if (longData)
				yStart = readS.readUint16BE();
			else
				yStart = readS.readByte();
			readS.readByte();		// Skip pad byte
			/*xPos = */readS.readUint16BE();
			/*yPos = */readS.readUint16BE();
			/*width = */readS.readUint16BE();
			/*height = */readS.readUint16BE();

			// Setup write pointer to the draw origin
			writePointer = (buf + (yStart * screenWidth) + xStart);
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_NOOP: // Does nothing
			readS.readByte();
			readS.readByte();
			readS.readByte();
			continue;
			break;
		case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // Long Unencoded Run
			runcount = readS.readSint16BE();
			for (i = 0; i < runcount; i++) {
				dataByte = readS.readByte();
				if (dataByte != 0) {
					*writePointer = dataByte;
				}
				writePointer++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long encoded run
			runcount = readS.readSint16BE();
			dataByte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*writePointer++ = dataByte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_ROW_END: // End of row
			xVector = readS.readSint16BE();

			if (longData)
				newRow = readS.readSint16BE();
			else
				newRow = readS.readByte();

			// Set write pointer to the new draw origin
			writePointer = buf + ((yStart + newRow) * screenWidth) + xStart + xVector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_REPOSITION: // Reposition command
			xVector = readS.readSint16BE();
			writePointer += xVector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_END: // End of frame marker
			return;
			break;
		default:
			break;
		}

		// Mask all but two high order control bits
		controlChar = markByte & 0xC0U;
		paramChar = markByte & 0x3FU;
		switch (controlChar) {
		case SAGA_FRAME_EMPTY_RUN: // 1100 0000
			// Run of empty pixels
			runcount = paramChar + 1;
			writePointer += runcount;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_COMPRESSED_RUN: // 1000 0000
			// Run of compressed data
			runcount = paramChar + 1;
			dataByte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*writePointer++ = dataByte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_UNCOMPRESSED_RUN: // 0100 0000
			// Uncompressed run
			runcount = paramChar + 1;
			for (i = 0; i < runcount; i++) {
				dataByte = readS.readByte();
				if (dataByte != 0) {
					*writePointer = dataByte;
				}
				writePointer++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		default:
			// Unknown marker found - abort
			error("decodeFrame() Invalid RLE marker encountered");
			break;
		}
	} while (1);
}

void Anim::fillFrameOffsets(AnimationData *anim) {
	uint16 currentFrame;
	byte markByte;
	uint16 control;
	uint16 runcount;
	int i;
	bool longData = isLongData();

	MemoryReadStreamEndian readS(anim->resourceData, anim->resourceLength, _vm->isBigEndian());

	readS.seek(12);

	readS._bigEndian = !_vm->isBigEndian(); // RLE has inversion BE<>LE

	for (currentFrame = 0; currentFrame <= anim->maxFrame; currentFrame++) {
		anim->frameOffsets[currentFrame] = readS.pos();

		// For some strange reason, the animation header is in little
		// endian format, but the actual RLE encoded frame data,
		// including the frame header, is in big endian format. */
		do {
			markByte = readS.readByte();
//			debug(7, "_pos=%x currentFrame=%i markByte=%x", readS.pos(), currentFrame, markByte);

			switch (markByte) {
			case SAGA_FRAME_START: // Start of frame
				// skip header
				if (longData) {
					readS.seek(13, SEEK_CUR);
				} else {
					readS.seek(12, SEEK_CUR);
				}
				continue;
				break;

			case SAGA_FRAME_END: // End of frame marker
				continue;
				break;
			case SAGA_FRAME_REPOSITION: // Reposition command
				readS.readSint16BE();
				continue;
				break;
			case SAGA_FRAME_ROW_END: // End of row marker
				readS.readSint16BE();
				if (longData)
					readS.readSint16BE();
				else
					readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long compressed run marker
				readS.readSint16BE();
				readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // (16) 0001 0000
				// Long Uncompressed Run
				runcount = readS.readSint16BE();
				for (i = 0; i < runcount; i++)
					readS.readByte();
				continue;
				break;
			case SAGA_FRAME_NOOP: // Does nothing
				readS.readByte();
				readS.readByte();
				readS.readByte();
				continue;
				break;
			default:
				break;
			}

			// Mask all but two high order (control) bits
			control = markByte & 0xC0;
			switch (control) {
			case SAGA_FRAME_EMPTY_RUN:
				// Run of empty pixels
				continue;
				break;
			case SAGA_FRAME_COMPRESSED_RUN:
				// Run of compressed data
				readS.readByte(); // Skip data byte
				continue;
				break;
			case SAGA_FRAME_UNCOMPRESSED_RUN:
				// Uncompressed run
				runcount = (markByte & 0x3f) + 1;
				for (i = 0; i < runcount; i++)
					readS.readByte();
				continue;
				break;
			default:
				error("Encountered unknown RLE marker %i", markByte);
				break;
			}
		} while (markByte != SAGA_FRAME_END);
	}
}

void Anim::animInfo() {
	uint16 animCount;
	uint16 i;

	animCount = getAnimationCount();

	_vm->_console->DebugPrintf("There are %d animations loaded:\n", animCount);

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] == NULL) {
			continue;
		}

		_vm->_console->DebugPrintf("%02d: Frames: %u Flags: %u\n", i, _animations[i]->maxFrame, _animations[i]->flags);
	}
}

void Anim::cutawayInfo() {
	uint16 i;

	_vm->_console->DebugPrintf("There are %d cutaways loaded:\n", _cutawayListLength);

	for (i = 0; i < _cutawayListLength; i++) {
		_vm->_console->DebugPrintf("%02d: Bg res: %u Anim res: %u Cycles: %u Framerate: %u\n", i,
			_cutawayList[i].backgroundResourceId, _cutawayList[i].animResourceId,
			_cutawayList[i].cycles, _cutawayList[i].frameRate);
	}
}

void Anim::resumeAll() {
	// Restore the animations
	for (int i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] && _animations[i]->state == ANIM_PLAYING) {
			resume(i, 0);
		}
	}
}

} // End of namespace Saga
