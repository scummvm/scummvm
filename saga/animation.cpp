/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
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

	if (fade) {
		// TODO: Fade down. Is this blocking or non-blocking?
	}

	if (!_cutawayActive) {
		_vm->_gfx->showCursor(false);
		_vm->_interface->setStatusText("");
		_vm->_interface->setSaveReminderState(0);

		// TODO: Hide the inventory. Perhaps a new panel mode?

		_cutawayActive = true;
	}

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
	_vm->_gfx->setPalette(palette);

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

	_vm->_resource->loadResource(context, _cutawayList[cut].animResourceId, resourceData, resourceDataLength);

	load(MAX_ANIMATIONS + cutawaySlot, resourceData, resourceDataLength);

	free(resourceData);

	setCycles(MAX_ANIMATIONS + cutawaySlot, _cutawayList[cut].cycles);
	setFrameTime(MAX_ANIMATIONS + cutawaySlot, 1000 / _cutawayList[cut].frameRate);
	play(MAX_ANIMATIONS + cutawaySlot, 0);
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
		// Note that clearCutaway() sets _cutawayActive to false.
		clearCutaway();

		// TODO: Handle fade up, if we previously faded down

		// TODO: Restore the scene

		// TODO: Restore the animations

		for (int i = 0; i < MAX_ANIMATIONS; i++) {
			if (_animations[i] && _animations[i]->state == ANIM_PLAYING) {
				resume(i, 0);
			}
		}
	}
}

void Anim::clearCutaway(void) {
	debug(0, "clearCutaway()\n");

	if (_cutawayActive) {
		_cutawayActive = false;

		for (int i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
		}

		_vm->_gfx->showCursor(true);
	}
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

	/*	char s[200];
		sprintf(s, "d:\\anim%i",animId);
		long flen=anim->resourceLength;
		char *buf=(char*)anim->resourceData;
		FILE*f;
		f=fopen(s,"wb");
		for(long i=0;i<flen;i++)
			fputc(buf[i],f);
		fclose(f);*/

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
		frame = anim->currentFrame;
		// FIXME: if start > 0, then this works incorrectly
		decodeFrame(anim, anim->frameOffsets[frame], displayBuffer, _vm->getDisplayWidth() * _vm->getDisplayHeight());

		anim->currentFrame++;
		if (anim->completed != 65535) {
			anim->completed++;
		}

		if (anim->currentFrame > anim->maxFrame) {
			anim->currentFrame = anim->loopFrame;

			if (anim->state == ANIM_STOPPING || anim->currentFrame == -1) {
				anim->state = ANIM_PAUSE;
			}
		}
	} else {
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
		}
	}

	if (anim->state == ANIM_PAUSE && anim->linkId != -1) {
		// If this animation has a link, follow it
		linkAnim = getAnimation(anim->linkId);

		debug(5, "Animation ended going to %d", anim->linkId);
//		linkAnim->cycles = anim->cycles;
//		linkAnim->currentFrame = 0;
//		linkAnim->completed = 0;
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
		error("VALIDATE_WRITE_POINTER: writePointer=%x buf=%x", writePointer, buf); \
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
			readS.readByte();		/* Skip pad byte */
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

} // End of namespace Saga
